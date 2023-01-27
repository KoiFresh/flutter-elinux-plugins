#pragma once

#include <flutter/basic_message_channel.h>
#include <flutter/method_channel.h>
#include <flutter/plugin_registrar.h>
#include <flutter/standard_method_codec.h>

// STL headers
#include <functional>
#include <map>
#include <memory>
#include <queue>
#include <sstream>
#include <string>
#include <variant>

extern "C" {
#include <gst/gst.h>
}

struct event {
    std::string function;
    flutter::EncodableValue value;
};

class AudioPlayer {
   public:
    AudioPlayer(std::string id, flutter::MethodChannel<flutter::EncodableValue> *channel);

    virtual ~AudioPlayer();

    int64_t GetPosition();

    int64_t GetDuration();

    bool GetLooping();

    void Play();

    void Pause();

    void Resume();

    void Dispose();

    void SetBalance(float balance);

    void SetLooping(bool isLooping);

    void SetVolume(double volume);

    void SetPlaybackRate(double rate);

    void SetPosition(int64_t position);

    void SetSourceUrl(std::string url);

   private:
    // Gst members
    GstElement *playbin;
    GstElement *source;
    GstElement *panorama;
    GstBus *bus;
    std::queue<event> event_queue;

    bool _isInitialized = false;
    bool _isPlaying = false;
    bool _isLooping = false;
    bool _isSeekCompleted = true;
    double _playbackRate = 1.0;

    std::string _url{};
    std::string _id;
    flutter::MethodChannel<flutter::EncodableValue> *_channel;

    static void SourceSetup(GstElement *playbin, GstElement *source, GstElement **p_src);

    // static GstBusSyncReply OnBusMessage(GstBus *bus, GstMessage *message, gpointer data);
    static gboolean OnBusMessage(GstBus *bus, GstMessage *message, gpointer data);

    static gboolean DispatchEventQueue(gpointer user_data);

    static gboolean OnRefresh(gpointer user_data);

    void SetPlayback(int64_t seekTo, double rate);

    void OnMediaError(GError *error, gchar *debug);

    void OnMediaStateChange(GstObject *src, GstState *old_state, GstState *new_state);

    void OnPositionUpdate();

    void OnDurationUpdate();

    void OnSeekCompleted();

    void OnPlaybackEnded();

    void InvokeLater(std::string name, flutter::EncodableValue value);
};
