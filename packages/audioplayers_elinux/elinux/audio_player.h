#pragma once

#include <flutter/basic_message_channel.h>
#include <flutter/method_channel.h>
#include <flutter/plugin_registrar.h>

// STL headers
#include <functional>
#include <map>
#include <memory>
#include <sstream>
#include <string>

class AudioPlayer {
public:
    AudioPlayer(std::string id);

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

};
