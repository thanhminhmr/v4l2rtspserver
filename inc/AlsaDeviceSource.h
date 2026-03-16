//
// Created by user on 2026-03-16.

#pragma once

#include <list>
#include <string>

#include <alsa/pcm.h>

#include <FramedSource.hh>


/**
 * Parameters for the ALSA device source.
 */
struct AlsaDeviceParameters {
	AlsaDeviceParameters(
			const std::string &deviceName, const std::list<snd_pcm_format_t> &formatList, const size_t sampleRate,
			const size_t channels
	)
		: deviceName(deviceName), formatList(formatList), sampleRate(sampleRate), channels(channels) {}

	const std::string &deviceName;
	const std::list<snd_pcm_format_t> &formatList;
	size_t sampleRate;
	size_t channels;
};

class AlsaDeviceSource final : FramedSource {
public:
	explicit AlsaDeviceSource(UsageEnvironment &env, const AlsaDeviceParameters &parameters);
	~AlsaDeviceSource() override = default;

	void doGetNextFrame() override;

	void close();

private:
	const AlsaDeviceParameters &parameters;

	snd_pcm_t *soundHandle = nullptr;
	snd_pcm_format_t sampleFormat = SND_PCM_FORMAT_UNKNOWN;
	size_t sampleRate = 0;
	size_t channels = 0;
	size_t bufferSize = 0;
	size_t periodSize = 0;
};
