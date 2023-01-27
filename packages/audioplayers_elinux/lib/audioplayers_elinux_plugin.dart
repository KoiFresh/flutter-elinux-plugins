import 'dart:async';

import 'package:audioplayers_platform_interface/audioplayers_platform_interface.dart';
import 'package:audioplayers_platform_interface/method_channel_audioplayers_platform.dart';
import 'package:flutter/services.dart';
import 'package:flutter/widgets.dart';

/// channel used for non player specific calls
const String kAudioplayersGlobalChannelName = 'xyz.luan/audioplayers.global';

/// method used to run a single event loop iteration
const String kAudioplayersGlobalChannelApiLoop = 'loop';

/// The Main Entry Point of The Dart Side for the AudioPlayersElinuxPlugin
/// Extends the functionality of the default Implemnetation of
/// MethodChannelAudioplayersPlatform by polling the platform side every second,
class AudioplayersElinuxPlugin extends MethodChannelAudioplayersPlatform {
  AudioplayersElinuxPlugin._() {
    /// go with 60 fps
    Timer.periodic(const Duration(milliseconds: 16), (Timer timer) {
      _channel.invokeMethod<void>(kAudioplayersGlobalChannelApiLoop);
    });
  }

  final MethodChannel _channel = const MethodChannel(
    kAudioplayersGlobalChannelName,
  );

  /// used to register this implementation of the plugin
  static void registerWith() {
    WidgetsFlutterBinding.ensureInitialized();

    AudioplayersPlatform.instance = AudioplayersElinuxPlugin._();
  }
}
