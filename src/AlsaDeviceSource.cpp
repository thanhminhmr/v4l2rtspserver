//
// Created by user on 2026-03-16.
//

#include "AlsaDeviceSource.h"

#include <logger.h>

static snd_pcm_format_t configureSampleFormat(
		snd_pcm_t *const soundHandle, snd_pcm_hw_params_t *const hardwareParams, const AlsaDeviceParameters &parameters
) {
	for (const auto format : parameters.formatList) {
		if (const int err = snd_pcm_hw_params_set_format(soundHandle, hardwareParams, format); err < 0) {
			LOG(WARN) << "Cannot set sample format!" << " device=" << parameters.deviceName << " format=" << format
					  << " error=" << snd_strerror(err);
		} else {
			LOG(NOTICE) << "Set sample format success." << " device=" << parameters.deviceName << " format=" << format;
			return format;
		}
	}
	return SND_PCM_FORMAT_UNKNOWN;
}

static size_t configureSampleRate(
		snd_pcm_t *const soundHandle, snd_pcm_hw_params_t *const hardwareParams, const AlsaDeviceParameters &parameters
) {
	unsigned int sampleRate = parameters.sampleRate;
	if (const int err = snd_pcm_hw_params_set_rate_near(soundHandle, hardwareParams, &sampleRate, nullptr); err < 0) {
		LOG(ERROR) << "Cannot set sample rate!" << " device=" << parameters.deviceName << " rate=" << sampleRate
				   << " error=" << snd_strerror(err);
		return 0;
	}
	LOG(NOTICE) << "Set sample rate success." << " device=" << parameters.deviceName << " rate=" << sampleRate;
	return sampleRate;
}

static size_t configureChannels(
		snd_pcm_t *const soundHandle, snd_pcm_hw_params_t *const hardwareParams, const AlsaDeviceParameters &parameters
) {
	unsigned int channels = parameters.channels;
	if (const int err = snd_pcm_hw_params_set_channels_near(soundHandle, hardwareParams, &channels); err < 0) {
		LOG(ERROR) << "Cannot set channels!" << " device=" << parameters.deviceName << " channels=" << channels
				   << " error=" << snd_strerror(err);
		return 0;
	}
	LOG(NOTICE) << "Set channels success." << " device=" << parameters.deviceName << " channels=" << channels;
	return channels;
}

AlsaDeviceSource::AlsaDeviceSource(UsageEnvironment &env, const AlsaDeviceParameters &parameters)
	: FramedSource(env), parameters(parameters) {

	LOG(NOTICE) << "Opening device." << " device=" << parameters.deviceName;
	snd_pcm_hw_params_t *hardwareParams = nullptr;

	// open PCM device
	if (const int err = snd_pcm_open(&soundHandle, parameters.deviceName.c_str(), SND_PCM_STREAM_CAPTURE, 0); err < 0) {
		LOG(ERROR) << "Cannot open device!" << " device=" << parameters.deviceName << " error=" << snd_strerror(err);
		return;
	}

	// configure hardware params
	if (const int err = snd_pcm_hw_params_malloc(&hardwareParams); err < 0) {
		LOG(ERROR) << "Cannot allocate hardware params!" << " device=" << parameters.deviceName
				   << " error=" << snd_strerror(err);
		goto AlsaDeviceSource_failed;
	}

	// initialize hardware params
	if (const int err = snd_pcm_hw_params_any(soundHandle, hardwareParams); err < 0) {
		LOG(ERROR) << "Cannot initialize hardware params!" << " device=" << parameters.deviceName
				   << " error=" << snd_strerror(err);
		goto AlsaDeviceSource_failed;
	}

	// set access to hardware params
	if (const int err = snd_pcm_hw_params_set_access(soundHandle, hardwareParams, SND_PCM_ACCESS_RW_INTERLEAVED);
		err < 0) {
		LOG(ERROR) << "Cannot set access type!" << " device=" << parameters.deviceName
				   << " error=" << snd_strerror(err);
		goto AlsaDeviceSource_failed;
	}

	// configure hardware sample format
	this->sampleFormat = configureSampleFormat(soundHandle, hardwareParams, parameters);
	if (sampleFormat == SND_PCM_FORMAT_UNKNOWN) {
		goto AlsaDeviceSource_failed;
	}

	// configure hardware sample rate
	this->sampleRate = configureSampleRate(soundHandle, hardwareParams, parameters);
	if (sampleRate == 0) {
		goto AlsaDeviceSource_failed;
	}

	// configure hardware channels
	this->channels = configureChannels(soundHandle, hardwareParams, parameters);
	if (this->channels == 0) {
		goto AlsaDeviceSource_failed;
	}

	// set hardware params to hardware
	if (const int err = snd_pcm_hw_params(soundHandle, hardwareParams); err < 0) {
		LOG(ERROR) << "Cannot set parameters!" << " device=" << parameters.deviceName << " error=" << snd_strerror(err);
		goto AlsaDeviceSource_failed;
	}

	// get buffer size
	if (const int err = snd_pcm_get_params(soundHandle, &bufferSize, &periodSize); err < 0) {
		LOG(ERROR) << "Cannot get parameters!" << " device=" << parameters.deviceName << " error=" << snd_strerror(err);
		goto AlsaDeviceSource_failed;
	}

	// prepare device
	if (const int err = snd_pcm_prepare(soundHandle); err < 0) {
		LOG(ERROR) << "Cannot prepare audio interface!" << " device=" << parameters.deviceName
				   << " error=" << snd_strerror(err);
		goto AlsaDeviceSource_failed;
	}

	// start capture
	if (const int err = snd_pcm_start(soundHandle); err < 0) {
		LOG(ERROR) << "cannot start audio interface!" << " device=" << parameters.deviceName
				   << " error=" << snd_strerror(err);
		goto AlsaDeviceSource_failed;
	}

	LOG(NOTICE) << "Open ALSA device success!" << " device=" << parameters.deviceName << " bufferSize=" << bufferSize
				<< " periodSize:" << periodSize;
	return;

AlsaDeviceSource_failed:
	close();
}

void AlsaDeviceSource::close() {
	if (soundHandle != nullptr) {
		LOG(NOTICE) << "Close ALSA device." << " device=" << parameters.deviceName;
		snd_pcm_close(soundHandle);
		soundHandle = nullptr;
	}
}