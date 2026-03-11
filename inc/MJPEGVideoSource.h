/* ---------------------------------------------------------------------------
** This software is in the public domain, furnished "as is", without technical
** support, and with no warranty, express or implied, as to its usefulness for
** any purpose.
**
** MJPEGVideoSource.h
**
** V4L2 RTSP streamer
**
** MJPEG Source for RTSP server
**
** -------------------------------------------------------------------------*/

#pragma once

#include "JPEGVideoSource.hh"
#include "logger.h"

class MJPEGVideoSource : public JPEGVideoSource {
public:
	static MJPEGVideoSource *createNew(UsageEnvironment &env, FramedSource *source) {
		return new MJPEGVideoSource(env, source);
	}
	void doGetNextFrame() override {
		if (m_inputSource) {
			m_inputSource->getNextFrame(fTo, fMaxSize, afterGettingFrameSub, this, FramedSource::handleClosure, this);
		}
	}
	void doStopGettingFrames() override {
		FramedSource::doStopGettingFrames();
		if (m_inputSource) {
			m_inputSource->stopGettingFrames();
		}
	}
	static void afterGettingFrameSub(
			void *clientData, unsigned frameSize, unsigned numTruncatedBytes, struct timeval presentationTime,
			unsigned durationInMicroseconds
	) {
		auto *source = reinterpret_cast<MJPEGVideoSource *>(clientData);
		source->afterGettingFrame(frameSize, numTruncatedBytes, presentationTime, durationInMicroseconds);
	}

	void afterGettingFrame(
			unsigned frameSize, unsigned numTruncatedBytes, struct timeval presentationTime,
			unsigned durationInMicroseconds
	);
	u_int8_t type() override { return m_restartInterval ? m_type | 0x40 : m_type; };
	u_int8_t qFactor() override { return 128; };
	u_int8_t width() override { return m_width; };
	u_int8_t height() override { return m_height; };
	u_int16_t restartInterval() override { return m_restartInterval; }

	u_int8_t const *quantizationTables(u_int8_t &precision, u_int16_t &length);

protected:
	MJPEGVideoSource(UsageEnvironment &env, FramedSource *source)
		: JPEGVideoSource(env), m_inputSource(source), m_width(0), m_height(0), m_qTable{}, m_qTableSize(0),
		  m_precision(0), m_type(0), m_restartInterval(0) {}
	~MJPEGVideoSource() override { Medium::close(m_inputSource); }

protected:
	FramedSource *m_inputSource;
	u_int8_t m_width;
	u_int8_t m_height;
	u_int8_t m_qTable[128 * 2];
	unsigned int m_qTableSize;
	unsigned int m_precision;
	u_int8_t m_type;
	u_int16_t m_restartInterval;
};
