#include "audio_player.h"

#include "Logger.h"

AudioPlayer::AudioPlayer(std::string id, flutter::MethodChannel<flutter::EncodableValue> *channel) : _id(id), _channel(channel) {
    gst_init(NULL, NULL);
    playbin = gst_element_factory_make("playbin", "playbin");
    if (!playbin) {
        Logger::Error(std::string("Not all elements could be created."));
        return;
    }

    // Setup stereo balance controller
    panorama = gst_element_factory_make("audiopanorama", "audiopanorama");
    if (panorama) {
        GstElement *audiosink = gst_element_factory_make("autoaudiosink", "audio_sink");

        GstElement *audiobin = gst_bin_new("audiobin");
        gst_bin_add_many(GST_BIN(audiobin), panorama, audiosink, NULL);
        gst_element_link(panorama, audiosink);

        GstPad *sinkpad = gst_element_get_static_pad(panorama, "sink");
        gst_element_add_pad(audiobin, gst_ghost_pad_new("sink", sinkpad));
        gst_object_unref(GST_OBJECT(sinkpad));

        g_object_set(G_OBJECT(playbin), "audio-sink", audiobin, NULL);
        gst_object_unref(GST_OBJECT(audiobin));

        g_object_set(G_OBJECT(panorama), "method", 1, NULL);
    }

    // Setup source options
    g_signal_connect(playbin, "source-setup", G_CALLBACK(AudioPlayer::SourceSetup), &source);

    bus = gst_element_get_bus(playbin);

    // Watch bus messages for one time events
    gst_bus_add_watch(bus, OnBusMessage, this);

    // Refresh continuously to emit reoccurring events
    g_timeout_add(1000, (GSourceFunc)AudioPlayer::OnRefresh, this);
    // Dispatch enqued events every time out loop is running
    g_timeout_add(0, AudioPlayer::DispatchEventQueue, this);
}

AudioPlayer::~AudioPlayer() {}

/**
 * @return int64_t the position in milliseconds
 */
int64_t AudioPlayer::GetPosition() {
    gint64 current = 0;
    if (!gst_element_query_position(playbin, GST_FORMAT_TIME, &current)) {
        Logger::Error(std::string("Could not query current position."));
        return -1;
    }
    return current / 1000000;
}

/**
 * @return int64_t the duration in milliseconds
 */
int64_t AudioPlayer::GetDuration() {
    gint64 duration = 0;
    if (!gst_element_query_duration(playbin, GST_FORMAT_TIME, &duration)) {
        Logger::Error(std::string("Could not query current duration."));
        return -1;
    }
    return duration / 1000000;
}

bool AudioPlayer::GetLooping() { return _isLooping; }

void AudioPlayer::Play() {
    SetPosition(0);
    Resume();
}

void AudioPlayer::Pause() {
    _isPlaying = false;
    GstStateChangeReturn ret = gst_element_set_state(playbin, GST_STATE_PAUSED);
    if (ret == GST_STATE_CHANGE_FAILURE) {
        Logger::Error(std::string("Unable to set the pipeline to the paused state."));
        return;
    }
    OnPositionUpdate();  // Update to exact position when pausing
}

void AudioPlayer::Resume() {
    _isPlaying = true;
    if (!_isInitialized) {
        return;
    }
    GstStateChangeReturn ret = gst_element_set_state(playbin, GST_STATE_PLAYING);
    if (ret == GST_STATE_CHANGE_FAILURE) {
        Logger::Error(std::string("Unable to set the pipeline to the playing state."));
        return;
    }
    // Update position and duration when start playing, as no event is emitted
    // elsewhere
    OnPositionUpdate();
    OnDurationUpdate();
}

void AudioPlayer::Dispose() {
    if (_isInitialized) {
        Pause();
    }
    gst_object_unref(bus);
    gst_object_unref(source);
    gst_object_unref(panorama);

    gst_element_set_state(playbin, GST_STATE_NULL);
    gst_object_unref(playbin);

    _channel = nullptr;
    _isInitialized = false;
}

void AudioPlayer::SetBalance(float balance) {
    if (!panorama) {
        Logger::Error(std::string("Audiopanorama was not initialized"));
        return;
    }

    if (balance > 1.0f) {
        balance = 1.0f;
    } else if (balance < -1.0f) {
        balance = -1.0f;
    }
    g_object_set(G_OBJECT(panorama), "panorama", balance, NULL);
}

void AudioPlayer::SetLooping(bool isLooping) { _isLooping = isLooping; }

void AudioPlayer::SetVolume(double volume) {
    if (volume > 1) {
        volume = 1;
    } else if (volume < 0) {
        volume = 0;
    }
    g_object_set(G_OBJECT(playbin), "volume", volume, NULL);
}

void AudioPlayer::SetPlaybackRate(double rate) {
    SetPlayback(GetPosition(), rate);
}

void AudioPlayer::SetPosition(int64_t position) {
    if (!_isInitialized) {
        return;
    }
    SetPlayback(position, _playbackRate);
}

void AudioPlayer::SetSourceUrl(std::string url) {
    if (_url != url) {
        _url = url;
        gst_element_set_state(playbin, GST_STATE_NULL);
        _isInitialized = false;
        _isPlaying = false;
        if (!_url.empty()) {
            g_object_set(playbin, "uri", _url.c_str(), NULL);
            if (playbin->current_state != GST_STATE_READY) {
                gst_element_set_state(playbin, GST_STATE_READY);
            }
        }
    }
}

// static
void AudioPlayer::SourceSetup(GstElement *playbin, GstElement *source, GstElement **p_src) {
    // Allow sources from unencrypted / misconfigured connections
    if (g_object_class_find_property(G_OBJECT_GET_CLASS(source), "ssl-strict") != 0) {
        g_object_set(G_OBJECT(source), "ssl-strict", FALSE, NULL);
    }
}

// static
gboolean AudioPlayer::OnBusMessage(GstBus *bus, GstMessage *message, gpointer user_data) {
    AudioPlayer *self = reinterpret_cast<AudioPlayer *>(user_data);

    switch (GST_MESSAGE_TYPE(message)) {
        case GST_MESSAGE_ERROR: {
            GError *err;
            gchar *debug;

            gst_message_parse_error(message, &err, &debug);
            self->OnMediaError(err, debug);
            g_error_free(err);
            g_free(debug);
            break;
        }
        case GST_MESSAGE_STATE_CHANGED:
            GstState old_state, new_state;

            gst_message_parse_state_changed(message, &old_state, &new_state, NULL);
            self->OnMediaStateChange(message->src, &old_state, &new_state);
            break;
        case GST_MESSAGE_EOS:
            gst_element_set_state(self->playbin, GST_STATE_READY);
            self->OnPlaybackEnded();
            break;
        case GST_MESSAGE_DURATION_CHANGED:
            self->OnDurationUpdate();
            break;
        case GST_MESSAGE_ASYNC_DONE:
            if (!self->_isSeekCompleted) {
                self->OnSeekCompleted();
                self->_isSeekCompleted = true;
            }
            break;
        default:
            // For more GstMessage types see:
            // https://gstreamer.freedesktop.org/documentation/gstreamer/gstmessage.html?gi-language=c#enumerations
            break;
    }

    // Continue watching for messages
    return TRUE;
}

gboolean AudioPlayer::DispatchEventQueue(gpointer user_data) {
    AudioPlayer *self = reinterpret_cast<AudioPlayer *>(user_data);

    for (int i = 0; i < self->event_queue.size(); i++) {
        struct event event = self->event_queue.front();
        self->_channel->InvokeMethod(event.function, std::make_unique<flutter::EncodableValue>(event.value));
        self->event_queue.pop();
    }

    return G_SOURCE_CONTINUE;
}

// Compare with refresh_ui in
// https://gstreamer.freedesktop.org/documentation/tutorials/basic/toolkit-integration.html?gi-language=c#walkthrough
// static
gboolean AudioPlayer::OnRefresh(gpointer user_data) {
    AudioPlayer *self = reinterpret_cast<AudioPlayer *>(user_data);

    // We do not want to update anything unless we are in the PAUSED or PLAYING states
    if (self->playbin->current_state == GST_STATE_PLAYING) {
        self->OnPositionUpdate();
    }
    return G_SOURCE_CONTINUE;
}

void AudioPlayer::SetPlayback(int64_t seekTo, double rate) {
    if (!_isInitialized) {
        return;
    }
    // See:
    // https://gstreamer.freedesktop.org/documentation/tutorials/basic/playback-speed.html?gi-language=c
    if (!_isSeekCompleted) {
        return;
    }
    if (rate == 0) {
        // Do not set rate if it's 0, rather pause.
        Pause();
        return;
    }

    if (_playbackRate != rate) {
        _playbackRate = rate;
    }
    _isSeekCompleted = false;

    GstEvent *seek_event;
    if (rate > 0) {
        seek_event = gst_event_new_seek(rate, GST_FORMAT_TIME, GstSeekFlags(GST_SEEK_FLAG_FLUSH | GST_SEEK_FLAG_ACCURATE), GST_SEEK_TYPE_SET, seekTo * GST_MSECOND, GST_SEEK_TYPE_NONE, -1);
    } else {
        seek_event = gst_event_new_seek(rate, GST_FORMAT_TIME, GstSeekFlags(GST_SEEK_FLAG_FLUSH | GST_SEEK_FLAG_ACCURATE), GST_SEEK_TYPE_SET, 0, GST_SEEK_TYPE_SET, seekTo * GST_MSECOND);
    }
    if (!gst_element_send_event(playbin, seek_event)) {
        Logger::Error(std::string("Could not set playback to position ") + std::to_string(seekTo) + std::string(" and rate ") + std::to_string(rate) + std::string("."));
        _isSeekCompleted = true;
    }
}

void AudioPlayer::OnMediaError(GError *error, gchar *debug) {
    std::ostringstream oss;
    oss << "Error: " << error->code << "; message=" << error->message;
    g_print("%s\n", oss.str().c_str());
    if (this->_channel) {
        flutter::EncodableValue map = flutter::EncodableValue(flutter::EncodableMap{
            {flutter::EncodableValue("playerId"), flutter::EncodableValue(_id.c_str())},
            {flutter::EncodableValue("value"), flutter::EncodableValue(oss.str().c_str())}});

        std::unique_ptr<flutter::EncodableValue> arguments = std::make_unique<flutter::EncodableValue>(map);

        InvokeLater("audio.onError", map);
        // this->_channel->InvokeMethod("audio.onError", std::move(arguments));
    }
}

void AudioPlayer::OnMediaStateChange(GstObject *src, GstState *old_state, GstState *new_state) {
    if (strcmp(GST_OBJECT_NAME(src), "playbin") == 0) {
        if (*new_state >= GST_STATE_READY) {
            if (!this->_isInitialized) {
                this->_isInitialized = true;
                if (this->_isPlaying) {
                    Resume();
                } else {
                    Pause();  // Need to set to pause state, in order to get duration
                }
            }
        } else if (this->_isInitialized) {
            this->_isInitialized = false;
        }
    }
}

void AudioPlayer::OnPositionUpdate() {
    if (this->_channel) {
        flutter::EncodableValue map = flutter::EncodableValue(flutter::EncodableMap{
            {flutter::EncodableValue("playerId"), flutter::EncodableValue(_id.c_str())},
            {flutter::EncodableValue("value"), flutter::EncodableValue(GetPosition())}});

        std::unique_ptr<flutter::EncodableValue> arguments = std::make_unique<flutter::EncodableValue>(map);
        InvokeLater("audio.onCurrentPosition", map);
        // this->_channel->InvokeMethod("audio.onCurrentPosition", std::move(arguments));
    }
}

void AudioPlayer::OnDurationUpdate() {
    if (this->_channel) {
        flutter::EncodableValue map = flutter::EncodableValue(flutter::EncodableMap{
            {flutter::EncodableValue("playerId"), flutter::EncodableValue(_id.c_str())},
            {flutter::EncodableValue("value"), flutter::EncodableValue(GetDuration())}});

        std::unique_ptr<flutter::EncodableValue> arguments = std::make_unique<flutter::EncodableValue>(map);
        InvokeLater("audio.onDuration", map);
        // this->_channel->InvokeMethod("audio.onDuration", std::move(arguments));
    }
}

void AudioPlayer::OnSeekCompleted() {
    if (this->_channel) {
        OnPositionUpdate();

        flutter::EncodableValue map = flutter::EncodableValue(flutter::EncodableMap{
            {flutter::EncodableValue("playerId"), flutter::EncodableValue(_id.c_str())},
            {flutter::EncodableValue("value"), flutter::EncodableValue(true)}});

        std::unique_ptr<flutter::EncodableValue> arguments = std::make_unique<flutter::EncodableValue>(map);
        InvokeLater("audio.onSeekComplete", map);
        // this->_channel->InvokeMethod("audio.onSeekComplete", std::move(arguments));
    }
}

void AudioPlayer::OnPlaybackEnded() {
    SetPosition(0);
    if (GetLooping()) {
        Play();
    }
    if (this->_channel) {
        flutter::EncodableValue map = flutter::EncodableValue(flutter::EncodableMap{
            {flutter::EncodableValue("playerId"), flutter::EncodableValue(_id.c_str())},
            {flutter::EncodableValue("value"), flutter::EncodableValue(true)}});

        std::unique_ptr<flutter::EncodableValue> arguments = std::make_unique<flutter::EncodableValue>(map);
        InvokeLater("audio.onComplete", map);
        // this->_channel->InvokeMethod("audio.onComplete", std::move(arguments));
    }
}

void AudioPlayer::InvokeLater(std::string name, flutter::EncodableValue value) {
    struct event e;
    e.function = name;
    e.value = std::move(value);

    event_queue.push(e);
}
