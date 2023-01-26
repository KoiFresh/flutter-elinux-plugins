# audioplayers_elinux

The implementation of the audioplayers plugin for flutter elinux. APIs are designed to be API compatible with the the original [`audioplayers`](https://github.com/bluefireteam/audioplayers) plugin.

## Required libraries

This plugin uses [GStreamer](https://gstreamer.freedesktop.org/) internally.

```Shell
$ sudo apt install libgstreamer1.0-dev

$ sudo apt install libgstreamer-plugins-base1.0-dev \
    gstreamer1.0-plugins-base gstreamer1.0-plugins-good \
    gstreamer1.0-plugins-bad gstreamer1.0-plugins-ugly gstreamer1.0-libav
```

## Usage

### pubspec.yaml

```yaml
dependencies:
  audioplayers: ^3.0.0
  camera_elinux:
    git:
      url: https://github.com/KoiFresh/flutter-elinux-plugins.git
      path: packages/audioplayers_elinux
      ref: feature/audioplayers
```

### Source code

Import `audioplayers` in your Dart code:

See also: https://github.com/bluefireteam/audioplayers/blob/main/packages/audioplayers/example/lib/main.dart

```dart
import 'package:audioplayers/audioplayers.dart';
```
