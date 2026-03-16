/* ---------------------------------------------------------------------------
** This software is in the public domain, furnished "as is", without technical
** support, and with no warranty, express or implied, as to its usefulness for
** any purpose.
**
** BaseServerMediaSubsession.h
**
** -------------------------------------------------------------------------*/

#pragma once

#include <sys/stat.h>

#include <fstream>
#include <iomanip>
#include <iostream>
#include <sstream>
#include <string>

// live555
#include <liveMedia.hh>

// libv4l2cpp
#include "V4l2Device.h"

// v4l2rtspserver
#include "V4L2DeviceSource.h"
#include "logger.h"

#ifdef HAVE_ALSA
#include "ALSACapture.h"
#endif

// ---------------------------------
//   BaseServerMediaSubsession
// ---------------------------------
class BaseServerMediaSubsession {
public:
	BaseServerMediaSubsession(StreamReplicator *replicator) : m_replicator(replicator) {
		if (auto *deviceSource = dynamic_cast<V4L2DeviceSource *>(replicator->inputSource())) {
			if (DeviceInterface *device = deviceSource->getDevice(); device->getVideoFormat() >= 0) {
				m_format = getVideoRtpFormat(device->getVideoFormat());
			} else {
				m_format = getAudioRtpFormat(device->getAudioFormat(), device->getSampleRate(), device->getChannels());
			}
			LOG(NOTICE) << "RTP format:" << m_format;
		}
	}

	// -----------------------------------------
	//    convert V4L2 pix format to RTP mime
	// -----------------------------------------
	static std::string getVideoRtpFormat(const unsigned int format) {
		switch (format) {
		case V4L2_PIX_FMT_HEVC:
			return "video/H265";
		case V4L2_PIX_FMT_H264:
			return "video/H264";
		case V4L2_PIX_FMT_MJPEG:
		case V4L2_PIX_FMT_JPEG:
			return "video/JPEG";
		case V4L2_PIX_FMT_VP8:
			return "video/VP8";
		case V4L2_PIX_FMT_VP9:
			return "video/VP9";
		case V4L2_PIX_FMT_YUV444:
		case V4L2_PIX_FMT_UYVY:
		case V4L2_PIX_FMT_NV12:
		case V4L2_PIX_FMT_Y41P:
		case V4L2_PIX_FMT_BGR24:
		case V4L2_PIX_FMT_BGR32:
		case V4L2_PIX_FMT_RGB24:
		case V4L2_PIX_FMT_RGB32:
			return "video/RAW";
		default:
			return {};
		}
	}

	static std::string getAudioRtpFormat(const int format, const int sampleRate, const int channels) {
		std::ostringstream os;
#ifdef HAVE_ALSA
		os << "audio/";
		switch (format) {
		case SND_PCM_FORMAT_A_LAW:
			os << "PCMA";
			break;
		case SND_PCM_FORMAT_MU_LAW:
			os << "PCMU";
			break;
		case SND_PCM_FORMAT_S8:
			os << "L8";
			break;
		case SND_PCM_FORMAT_S24_BE:
		case SND_PCM_FORMAT_S24_LE:
			os << "L24";
			break;
		case SND_PCM_FORMAT_S32_BE:
		case SND_PCM_FORMAT_S32_LE:
			os << "L32";
			break;
		case SND_PCM_FORMAT_MPEG:
			os << "MPEG";
			break;
		default:
			os << "L16";
			break;
		}
		os << "/" << sampleRate << "/" << channels;
#endif
		return os.str();
	}

public:
	static FramedSource *createSource(UsageEnvironment &env, FramedSource *videoES, const std::string &format);
	static RTPSink *createSink(
			UsageEnvironment &env, Groupsock *rtpGroupSock, unsigned char rtpPayloadTypeIfDynamic,
			const std::string &format, V4L2DeviceSource *source
	);
	static char const *getAuxLine(V4L2DeviceSource *source, RTPSink *rtpSink);

	[[nodiscard]] std::string getLastFrame() const {
		if (auto *deviceSource = dynamic_cast<V4L2DeviceSource *>(m_replicator->inputSource())) {
			return deviceSource->getLastFrame();
		}
		return "";
	}

	[[nodiscard]] std::string getFormat() const { return m_format; }

protected:
	StreamReplicator *m_replicator;
	std::string m_format;
};
