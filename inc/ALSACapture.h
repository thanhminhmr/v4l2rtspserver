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
			const char *devname, const std::list<snd_pcm_format_t> &formatList, unsigned int sampleRate,
			unsigned int channels
	)
		: m_devName(devname), m_formatList(formatList), m_sampleRate(sampleRate), m_channels(channels) {}

	std::string m_devName;
	std::list<snd_pcm_format_t> m_formatList;
	unsigned int m_sampleRate;
	unsigned int m_channels;
};

class ALSACapture : public DeviceInterface {
public:
	static ALSACapture *createNew(const ALSACaptureParameters &params);
	~ALSACapture() override;
	void close();

protected:
	ALSACapture(const ALSACaptureParameters &params);
	int configureFormat(snd_pcm_hw_params_t *hw_params);

public:
	ssize_t read(char *buffer, size_t bufferSize) override;
	int getFd() override;
	size_t getBufferSize() override { return m_bufferSize; }

	int getSampleRate() override { return m_params.m_sampleRate; }
	int getChannels() override { return m_params.m_channels; }
	int getAudioFormat() override { return m_fmt; }
	std::list<int> getAudioFormatList() override { return m_fmtList; }

private:
	snd_pcm_t *m_pcm;
	size_t m_bufferSize;
	size_t m_periodSize;
	ALSACaptureParameters m_params;
	snd_pcm_format_t m_fmt;
	std::list<int> m_fmtList;
};
