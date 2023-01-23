#include "include/audioplayers_elinux/audioplayers_elinux_plugin.h"

#include <flutter/method_channel.h>
#include <flutter/plugin_registrar.h>
#include <flutter/standard_method_codec.h>

#include <variant>

#include <map>
#include <memory>
#include <sstream>

#include <iostream>
#include <string>

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
        static void RegisterWithRegistrar(flutter::PluginRegistrar *registrar);

        AudioplayersElinuxPlugin();

        virtual ~AudioplayersElinuxPlugin();

    private:
        // Called when a method is called on this plugin's channel from Dart.
        void HandleMethodCall(const flutter::MethodCall<flutter::EncodableValue> &method_call, std::unique_ptr<flutter::MethodResult<flutter::EncodableValue>> result);

        void HandleSetSourceUrlCall(const flutter::EncodableValue* message, std::unique_ptr<flutter::MethodResult<flutter::EncodableValue>> result);
    };

    // static
    void AudioplayersElinuxPlugin::RegisterWithRegistrar(flutter::PluginRegistrar *registrar) {
        auto channel = std::make_unique<flutter::MethodChannel<flutter::EncodableValue>>(registrar->messenger(), kAudioplayersChannelName, &flutter::StandardMethodCodec::GetInstance());

        auto plugin = std::make_unique<AudioplayersElinuxPlugin>();

        channel->SetMethodCallHandler([plugin_pointer = plugin.get()](const auto &call, auto result) {
            plugin_pointer->HandleMethodCall(call, std::move(result));
        });

        registrar->AddPlugin(std::move(plugin));
    }

    AudioplayersElinuxPlugin::AudioplayersElinuxPlugin() {}

    AudioplayersElinuxPlugin::~AudioplayersElinuxPlugin() {}

    void AudioplayersElinuxPlugin::HandleMethodCall(const flutter::MethodCall<flutter::EncodableValue> &method_call, std::unique_ptr<flutter::MethodResult<flutter::EncodableValue>> result) {

        if (method_call.method_name().compare(kAudioplayersChannelApiPause) == 0) {
            result->NotImplemented();

        } else if (method_call.method_name().compare(kAudioplayersChannelApiResume) == 0) {
            result->NotImplemented();

        } else if (method_call.method_name().compare(kAudioplayersChannelApiStop) == 0) {
            result->NotImplemented();

        } else if (method_call.method_name().compare(kAudioplayersChannelApiRelease) == 0) {
            result->NotImplemented();

        } else if (method_call.method_name().compare(kAudioplayersChannelApiSeek) == 0) {
            result->NotImplemented();

        } else if (method_call.method_name().compare(kAudioplayersChannelApiSetSourceUrl) == 0) {
            HandleSetSourceUrlCall(method_call.arguments(), std::move(result));

        } else if (method_call.method_name().compare(kAudioplayersChannelApiGetDuration) == 0) {
            result->NotImplemented();
        
        } else if (method_call.method_name().compare(kAudioplayersChannelApiSetVolume) == 0) {
            result->NotImplemented();
        
        } else if (method_call.method_name().compare(kAudioplayersChannelApiGetCurrentPosition) == 0) {
            result->NotImplemented();

        } else if (method_call.method_name().compare(kAudioplayersChannelApiSetPlaybackRate) == 0) {
            result->NotImplemented();

        } else if (method_call.method_name().compare(kAudioplayersChannelApiSetReleaseMode) == 0) {
            result->NotImplemented();

        } else if (method_call.method_name().compare(kAudioplayersChannelApiSetPlayerMode) == 0) {
            result->NotImplemented();

        } else if (method_call.method_name().compare(kAudioplayersChannelApiSetBalance) == 0) {
            result->NotImplemented();

        } else  {
            result->NotImplemented();
        }
    } // HandleMethodCall
}  // namespace

void AudioplayersElinuxPlugin::HandleSetSourceUrlCall(const flutter::EncodableValue* message, std::unique_ptr<flutter::MethodResult<flutter::EncodableValue>> result) {
    result->NotImplemented();
}

void AudioplayersElinuxPluginRegisterWithRegistrar(FlutterDesktopPluginRegistrarRef registrar) {
  AudioplayersElinuxPlugin::RegisterWithRegistrar(flutter::PluginRegistrarManager::GetInstance()->GetRegistrar<flutter::PluginRegistrar>(registrar));
}
