#include "include/audioplayers_elinux/audioplayers_elinux_plugin.h"

#include <flutter/method_channel.h>
#include <flutter/plugin_registrar.h>
#include <flutter/standard_method_codec.h>

#include <map>
#include <memory>
#include <sstream>

namespace {

class AudioplayersElinuxPlugin : public flutter::Plugin {
 public:
  static void RegisterWithRegistrar(flutter::PluginRegistrar *registrar);

  AudioplayersElinuxPlugin();

  virtual ~AudioplayersElinuxPlugin();

 private:
  // Called when a method is called on this plugin's channel from Dart.
  void HandleMethodCall(
      const flutter::MethodCall<flutter::EncodableValue> &method_call,
      std::unique_ptr<flutter::MethodResult<flutter::EncodableValue>> result);
};

// static
void AudioplayersElinuxPlugin::RegisterWithRegistrar(
    flutter::PluginRegistrar *registrar) {
  auto channel =
      std::make_unique<flutter::MethodChannel<flutter::EncodableValue>>(
          registrar->messenger(), "audioplayers_elinux",
          &flutter::StandardMethodCodec::GetInstance());

  auto plugin = std::make_unique<AudioplayersElinuxPlugin>();

  channel->SetMethodCallHandler(
      [plugin_pointer = plugin.get()](const auto &call, auto result) {
        plugin_pointer->HandleMethodCall(call, std::move(result));
      });

  registrar->AddPlugin(std::move(plugin));
}

AudioplayersElinuxPlugin::AudioplayersElinuxPlugin() {}

AudioplayersElinuxPlugin::~AudioplayersElinuxPlugin() {}

void AudioplayersElinuxPlugin::HandleMethodCall(
    const flutter::MethodCall<flutter::EncodableValue> &method_call,
    std::unique_ptr<flutter::MethodResult<flutter::EncodableValue>> result) {
  if (method_call.method_name().compare("getPlatformVersion") == 0) {
    std::ostringstream version_stream;
    version_stream << "eLinux";
    result->Success(flutter::EncodableValue(version_stream.str()));
  } else {
    result->NotImplemented();
  }
}

}  // namespace

void AudioplayersElinuxPluginRegisterWithRegistrar(
    FlutterDesktopPluginRegistrarRef registrar) {
  AudioplayersElinuxPlugin::RegisterWithRegistrar(
      flutter::PluginRegistrarManager::GetInstance()
          ->GetRegistrar<flutter::PluginRegistrar>(registrar));
}
