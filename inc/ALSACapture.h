/* ---------------------------------------------------------------------------
** This software is in the public domain, furnished "as is", without technical
** support, and with no warranty, express or implied, as to its usefulness for
** any purpose.
**
** ALSACapture.h
**
** V4L2 RTSP streamer
**
** ALSA capture overide of V4l2Capture
**
** -------------------------------------------------------------------------*/

#pragma once

#include <list>

#include "logger.h"
#include <alsa/asoundlib.h>

#include "DeviceInterface.h"

struct ALSACaptureParameters {
	ALSACaptureParameters(
			const char *devName, const std::list<snd_pcm_format_t> &formatList, const unsigned int sampleRate,
			const unsigned int channels
	)
		: m_devName(devName), formatList(formatList), m_sampleRate(sampleRate), m_channels(channels) {}

	std::string m_devName;
	std::list<snd_pcm_format_t> formatList;
	unsigned int m_sampleRate;
	unsigned int m_channels;
};

class ALSACapture : public DeviceInterface {
public:
	static ALSACapture *createNew(const ALSACaptureParameters &params);
	~ALSACapture() override;
	void close();

protected:
	explicit ALSACapture(const ALSACaptureParameters &params);
	int configureFormat(snd_pcm_hw_params_t *hw_params);

public:
	size_t read(std::span<uint8_t> buffer) override;
	int getFd() override;
	unsigned long getBufferSize() override { return m_bufferSize; }

	[[nodiscard]] unsigned int getSampleRate() const { return m_params.m_sampleRate; }
	[[nodiscard]] unsigned int getChannels() const { return m_params.m_channels; }
	[[nodiscard]] int getAudioFormat() const { return m_fmt; }
	std::list<int> getAudioFormatList() { return m_fmtList; }

private:
	snd_pcm_t *m_pcm;
	unsigned long m_bufferSize;
	unsigned long m_periodSize;
	ALSACaptureParameters m_params;
	snd_pcm_format_t m_fmt;
	std::list<int> m_fmtList;
};
