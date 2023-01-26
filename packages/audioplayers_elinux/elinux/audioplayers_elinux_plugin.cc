#include "include/audioplayers_elinux/audioplayers_elinux_plugin.h"

#include <flutter/method_channel.h>
#include <flutter/plugin_registrar.h>
#include <flutter/standard_method_codec.h>

#include <iostream>
#include <map>
#include <memory>
#include <sstream>
#include <string>
#include <variant>

#include "Logger.h"
#include "audio_player.h"

template <typename T>
T get_args_value(const flutter::EncodableValue* args, std::string key, T default_value) {
    const flutter::EncodableValue& fl_args = *args;

    if (std::holds_alternative<flutter::EncodableMap>(fl_args)) {
        flutter::EncodableMap map = std::get<flutter::EncodableMap>(fl_args);
        flutter::EncodableValue& fl_key = map[flutter::EncodableValue(key)];

        if (std::holds_alternative<T>(fl_key)) {
            return std::get<T>(fl_key);
        }
    }

    return default_value;
}

static std::unique_ptr<flutter::MethodChannel<flutter::EncodableValue>> channel;

namespace {

constexpr char kAudioplayersChannelName[] = "xyz.luan/audioplayers";
constexpr char kAudioplayersChannelApiPause[] = "pause";
constexpr char kAudioplayersChannelApiResume[] = "resume";
constexpr char kAudioplayersChannelApiStop[] = "stop";
constexpr char kAudioplayersChannelApiRelease[] = "release";
constexpr char kAudioplayersChannelApiSeek[] = "seek";
constexpr char kAudioplayersChannelApiSetSourceUrl[] = "setSourceUrl";
constexpr char kAudioplayersChannelApiGetDuration[] = "getDuration";
constexpr char kAudioplayersChannelApiSetVolume[] = "setVolume";
constexpr char kAudioplayersChannelApiGetCurrentPosition[] = "getCurrentPosition";
constexpr char kAudioplayersChannelApiSetPlaybackRate[] = "setPlaybackRate";
constexpr char kAudioplayersChannelApiSetReleaseMode[] = "setReleaseMode";
constexpr char kAudioplayersChannelApiSetPlayerMode[] = "setPlayerMode";
constexpr char kAudioplayersChannelApiSetBalance[] = "setBalance";

class AudioplayersElinuxPlugin : public flutter::Plugin {
   public:
    static void RegisterWithRegistrar(flutter::PluginRegistrar* registrar);

    AudioplayersElinuxPlugin();

    virtual ~AudioplayersElinuxPlugin();

   private:
    std::map<std::string, std::unique_ptr<AudioPlayer>> audioPlayers;

    AudioPlayer* GetPlayerById(std::string id);

    // Called when a method is called on this plugin's channel from Dart.
    void HandleMethodCall(const flutter::MethodCall<flutter::EncodableValue>& method_call, std::unique_ptr<flutter::MethodResult<flutter::EncodableValue>> result);

    void HandlePauseCall(AudioPlayer* player, const flutter::EncodableValue* message, std::unique_ptr<flutter::MethodResult<flutter::EncodableValue>> result);
    void HandleResumeCall(AudioPlayer* player, const flutter::EncodableValue* message, std::unique_ptr<flutter::MethodResult<flutter::EncodableValue>> result);
    void HandleStopCall(AudioPlayer* player, const flutter::EncodableValue* message, std::unique_ptr<flutter::MethodResult<flutter::EncodableValue>> result);
    void HandleReleaseCall(AudioPlayer* player, const flutter::EncodableValue* message, std::unique_ptr<flutter::MethodResult<flutter::EncodableValue>> result);
    void HandleSeekCall(AudioPlayer* player, const flutter::EncodableValue* message, std::unique_ptr<flutter::MethodResult<flutter::EncodableValue>> result);
    void HandleSetSourceUrlCall(AudioPlayer* player, const flutter::EncodableValue* message, std::unique_ptr<flutter::MethodResult<flutter::EncodableValue>> result);
    void HandleGetDurationCall(AudioPlayer* player, const flutter::EncodableValue* message, std::unique_ptr<flutter::MethodResult<flutter::EncodableValue>> result);
    void HandleSetVolumeCall(AudioPlayer* player, const flutter::EncodableValue* message, std::unique_ptr<flutter::MethodResult<flutter::EncodableValue>> result);
    void HandleGetCurrentPositionCall(AudioPlayer* player, const flutter::EncodableValue* message, std::unique_ptr<flutter::MethodResult<flutter::EncodableValue>> result);
    void HandleSetPlaybackRateCall(AudioPlayer* player, const flutter::EncodableValue* message, std::unique_ptr<flutter::MethodResult<flutter::EncodableValue>> result);
    void HandleSetReleaseModeCall(AudioPlayer* player, const flutter::EncodableValue* message, std::unique_ptr<flutter::MethodResult<flutter::EncodableValue>> result);
    void HandleSetBalanceCall(AudioPlayer* player, const flutter::EncodableValue* message, std::unique_ptr<flutter::MethodResult<flutter::EncodableValue>> result);
};

// static
void AudioplayersElinuxPlugin::RegisterWithRegistrar(flutter::PluginRegistrar* registrar) {
    channel = std::make_unique<flutter::MethodChannel<flutter::EncodableValue>>(registrar->messenger(), kAudioplayersChannelName, &flutter::StandardMethodCodec::GetInstance());

    auto plugin = std::make_unique<AudioplayersElinuxPlugin>();

    channel->SetMethodCallHandler([plugin_pointer = plugin.get()](const auto& call, auto result) {
        plugin_pointer->HandleMethodCall(call, std::move(result));
    });

    registrar->AddPlugin(std::move(plugin));
}

AudioplayersElinuxPlugin::AudioplayersElinuxPlugin() {}

AudioplayersElinuxPlugin::~AudioplayersElinuxPlugin() {}

void AudioplayersElinuxPlugin::HandleMethodCall(const flutter::MethodCall<flutter::EncodableValue>& method_call, std::unique_ptr<flutter::MethodResult<flutter::EncodableValue>> result) {
    if (method_call.method_name().compare("audioPlayerOnRefresh") == 0) {
        for (const auto& [id, player] : audioPlayers) {
            AudioPlayer::OnRefresh(player.get());
        }

        result->Success(flutter::EncodableValue(1));
        return;
    }

    std::string playerId = get_args_value<std::string>(method_call.arguments(), "playerId", "");

    if (playerId.empty()) {
        Logger::Error("Empty PlayerId received on " + std::string(method_call.method_name()));
        result->Success(flutter::EncodableValue(0));  // failure
        return;
    }

    AudioPlayer* player = GetPlayerById(playerId);

    if (method_call.method_name().compare(kAudioplayersChannelApiPause) == 0) {
        HandlePauseCall(player, method_call.arguments(), std::move(result));

    } else if (method_call.method_name().compare(kAudioplayersChannelApiResume) == 0) {
        HandleResumeCall(player, method_call.arguments(), std::move(result));

    } else if (method_call.method_name().compare(kAudioplayersChannelApiStop) == 0) {
        HandleStopCall(player, method_call.arguments(), std::move(result));

    } else if (method_call.method_name().compare(kAudioplayersChannelApiRelease) == 0) {
        HandleReleaseCall(player, method_call.arguments(), std::move(result));

    } else if (method_call.method_name().compare(kAudioplayersChannelApiSeek) == 0) {
        HandleSeekCall(player, method_call.arguments(), std::move(result));

    } else if (method_call.method_name().compare(kAudioplayersChannelApiSetSourceUrl) == 0) {
        HandleSetSourceUrlCall(player, method_call.arguments(), std::move(result));

    } else if (method_call.method_name().compare(kAudioplayersChannelApiGetDuration) == 0) {
        HandleGetDurationCall(player, method_call.arguments(), std::move(result));

    } else if (method_call.method_name().compare(kAudioplayersChannelApiSetVolume) == 0) {
        HandleSetVolumeCall(player, method_call.arguments(), std::move(result));

    } else if (method_call.method_name().compare(kAudioplayersChannelApiGetCurrentPosition) == 0) {
        HandleGetCurrentPositionCall(player, method_call.arguments(), std::move(result));

    } else if (method_call.method_name().compare(kAudioplayersChannelApiSetPlaybackRate) == 0) {
        HandleSetPlaybackRateCall(player, method_call.arguments(), std::move(result));

    } else if (method_call.method_name().compare(kAudioplayersChannelApiSetReleaseMode) == 0) {
        HandleSetReleaseModeCall(player, method_call.arguments(), std::move(result));

    } else if (method_call.method_name().compare(kAudioplayersChannelApiSetPlayerMode) == 0) {
        // TODO: check support for low latency mode:
        // https://gstreamer.freedesktop.org/documentation/additional/design/latency.html?gi-language=c
        result->NotImplemented();

    } else if (method_call.method_name().compare(kAudioplayersChannelApiSetBalance) == 0) {
        HandleSetBalanceCall(player, method_call.arguments(), std::move(result));

    } else {
        result->NotImplemented();
    }
}  // HandleMethodCall
}  // namespace

AudioPlayer* AudioplayersElinuxPlugin::GetPlayerById(std::string id) {
    auto searchPlayer = audioPlayers.find(id);

    if (searchPlayer != audioPlayers.end()) {
        return searchPlayer->second.get();
    }

    auto player = std::make_unique<AudioPlayer>(id, channel.get());
    auto playerPtr = player.get();
    audioPlayers.insert(std::make_pair(id, std::move(player)));

    return playerPtr;
}

void AudioplayersElinuxPlugin::HandlePauseCall(AudioPlayer* player, const flutter::EncodableValue* message, std::unique_ptr<flutter::MethodResult<flutter::EncodableValue>> result) {
    player->Pause();

    result->Success(flutter::EncodableValue(1));  // success
}

void AudioplayersElinuxPlugin::HandleResumeCall(AudioPlayer* player, const flutter::EncodableValue* message, std::unique_ptr<flutter::MethodResult<flutter::EncodableValue>> result) {
    player->Resume();

    result->Success(flutter::EncodableValue(1));  // success
}

void AudioplayersElinuxPlugin::HandleStopCall(AudioPlayer* player, const flutter::EncodableValue* message, std::unique_ptr<flutter::MethodResult<flutter::EncodableValue>> result) {
    player->Pause();
    player->SetPosition(0);

    result->Success(flutter::EncodableValue(1));  // success
}

void AudioplayersElinuxPlugin::HandleReleaseCall(AudioPlayer* player, const flutter::EncodableValue* message, std::unique_ptr<flutter::MethodResult<flutter::EncodableValue>> result) {
    player->Pause();
    player->SetPosition(0);

    result->Success(flutter::EncodableValue(1));  // success
}

void AudioplayersElinuxPlugin::HandleSeekCall(AudioPlayer* player, const flutter::EncodableValue* message, std::unique_ptr<flutter::MethodResult<flutter::EncodableValue>> result) {
    int position = get_args_value<int>(message, "position", player->GetPosition());
    player->SetPosition(position);

    result->Success(flutter::EncodableValue(1));  // success
}

void AudioplayersElinuxPlugin::HandleSetSourceUrlCall(AudioPlayer* player, const flutter::EncodableValue* message, std::unique_ptr<flutter::MethodResult<flutter::EncodableValue>> result) {
    std::string url = get_args_value<std::string>(message, "url", "");

    if (url.empty()) {
        Logger::Error("Null URL received on setSourceUrl");
        result->Success(flutter::EncodableValue(0));  // failure
        return;
    }

    bool isLocalFile = get_args_value<bool>(message, "isLocal", false);
    if (isLocalFile) {
        url = std::string("file://") + url;
    }

    try {
        player->SetSourceUrl(url);
    } catch (...) {
        Logger::Error("Error setting url to '" + url + "'.");
        result->Success(flutter::EncodableValue(0));  // failure
    }

    result->Success(flutter::EncodableValue(1));  // success
}

void AudioplayersElinuxPlugin::HandleGetDurationCall(AudioPlayer* player, const flutter::EncodableValue* message, std::unique_ptr<flutter::MethodResult<flutter::EncodableValue>> result) {
    int64_t duration = player->GetDuration();

    result->Success(flutter::EncodableValue(duration));  // success
}

void AudioplayersElinuxPlugin::HandleSetVolumeCall(AudioPlayer* player, const flutter::EncodableValue* message, std::unique_ptr<flutter::MethodResult<flutter::EncodableValue>> result) {
    double volume = get_args_value<double>(message, "volume", 1.0);
    player->SetVolume(volume);

    result->Success(flutter::EncodableValue(1));  // success
}

void AudioplayersElinuxPlugin::HandleGetCurrentPositionCall(AudioPlayer* player, const flutter::EncodableValue* message, std::unique_ptr<flutter::MethodResult<flutter::EncodableValue>> result) {
    int64_t position = player->GetPosition();

    result->Success(flutter::EncodableValue(position));  // success
}

void AudioplayersElinuxPlugin::HandleSetPlaybackRateCall(AudioPlayer* player, const flutter::EncodableValue* message, std::unique_ptr<flutter::MethodResult<flutter::EncodableValue>> result) {
    double rate = get_args_value<double>(message, "playbackRate", 1.0);
    player->SetPlaybackRate(rate);

    result->Success(flutter::EncodableValue(1));  // success
}

void AudioplayersElinuxPlugin::HandleSetReleaseModeCall(AudioPlayer* player, const flutter::EncodableValue* message, std::unique_ptr<flutter::MethodResult<flutter::EncodableValue>> result) {
    std::string releaseMode = get_args_value<std::string>(message, "releaseMode", "");

    if (releaseMode.empty()) {
        Logger::Error("Error calling setReleaseMode, releaseMode cannot be null");

        result->Success(flutter::EncodableValue(0));  // failure
        return;
    }

    bool looping = (releaseMode.find("loop") != std::string::npos);
    player->SetLooping(looping);

    result->Success(flutter::EncodableValue(1));  // success
}

void AudioplayersElinuxPlugin::HandleSetBalanceCall(AudioPlayer* player, const flutter::EncodableValue* message, std::unique_ptr<flutter::MethodResult<flutter::EncodableValue>> result) {
    double balance = get_args_value<double>(message, "balance", 0.0);
    player->SetBalance(balance);

    result->Success(flutter::EncodableValue(1));  // success
}

void AudioplayersElinuxPluginRegisterWithRegistrar(FlutterDesktopPluginRegistrarRef registrar) {
    AudioplayersElinuxPlugin::RegisterWithRegistrar(flutter::PluginRegistrarManager::GetInstance()->GetRegistrar<flutter::PluginRegistrar>(registrar));
}
