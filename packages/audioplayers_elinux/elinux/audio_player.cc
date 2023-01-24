#include "audio_player.h"
#include "Logger.h"

AudioPlayer::AudioPlayer(std::string id) {}

AudioPlayer::~AudioPlayer() {}

int64_t AudioPlayer::GetPosition() {
	return 0;
}

int64_t AudioPlayer::GetDuration() {
	return 0;
}

bool AudioPlayer::GetLooping() {
	return false;
}

void AudioPlayer::Play() {}

void AudioPlayer::Pause() {}

void AudioPlayer::Resume() {}

void AudioPlayer::Dispose() {}

void AudioPlayer::SetBalance(float balance) {}

void AudioPlayer::SetLooping(bool isLooping) {}

void AudioPlayer::SetVolume(double volume) {}

void AudioPlayer::SetPlaybackRate(double rate) {}

void AudioPlayer::SetPosition(int64_t position) {}

void AudioPlayer::SetSourceUrl(std::string url) {}
