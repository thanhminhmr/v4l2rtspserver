/* ---------------------------------------------------------------------------
** This software is in the public domain, furnished "as is", without technical
** support, and with no warranty, express or implied, as to its usefulness for
** any purpose.
**
** H26x_V4l2DeviceSource.h
**
** H264/H265 V4L2 live555 source
**
** -------------------------------------------------------------------------*/

#pragma once

// project
#include "V4L2DeviceSource.h"

// ---------------------------------
// H264 V4L2 FramedSource
// ---------------------------------
constexpr uint8_t H26X_MARKER[] = {0, 0, 0, 1};
constexpr uint8_t H26X_SHORT_MARKER[] = {0, 0, 1};

class H26X_V4L2DeviceSource : public V4L2DeviceSource {
protected:
	H26X_V4L2DeviceSource(
			UsageEnvironment &env, DeviceInterface *device, const int outputFd, const size_t queueSize,
			const CaptureMode captureMode, const bool repeatConfig
	)
		: V4L2DeviceSource(env, device, outputFd, queueSize, captureMode), m_repeatConfig(repeatConfig) {}

	~H26X_V4L2DeviceSource() override = default;

	static uint8_t const *extractFrame(uint8_t const *frame, size_t &size, size_t &outputSize, uint8_t &frameType);
	static std::basic_string<uint8_t> getFrameWithMarker(const std::basic_string<uint8_t> &frame);


	// ----- Fields -----

	std::basic_string<uint8_t> m_sps;
	std::basic_string<uint8_t> m_pps;
	bool m_repeatConfig;
};
