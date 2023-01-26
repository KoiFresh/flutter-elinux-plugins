import 'dart:async';

import 'package:audioplayers_platform_interface/audioplayers_platform_interface.dart';
import 'package:audioplayers_platform_interface/method_channel_audioplayers_platform.dart';
import 'package:flutter/services.dart';
import 'package:flutter/widgets.dart';

/// The Main Entry Point of The Dart Side for the AudioPlayersElinuxPlugin
/// Extends the functionality of the default Implemnetation of
/// MethodChannelAudioplayersPlatform by polling the platform side every second,
class AudioplayersElinuxPlugin extends MethodChannelAudioplayersPlatform {
  AudioplayersElinuxPlugin._() {
    Timer.periodic(const Duration(seconds: 1), (Timer timer) {
      _channel.invokeMethod<void>('audioPlayerOnRefresh');
    });
  }

  final MethodChannel _channel = const MethodChannel('xyz.luan/audioplayers');

  /// used to register this implementation of the plugin
  static void registerWith() {
    WidgetsFlutterBinding.ensureInitialized();

    AudioplayersPlatform.instance = AudioplayersElinuxPlugin._();
  }
}
