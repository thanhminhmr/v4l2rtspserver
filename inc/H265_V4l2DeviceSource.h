/* ---------------------------------------------------------------------------
** This software is in the public domain, furnished "as is", without technical
** support, and with no warranty, express or implied, as to its usefulness for
** any purpose.
**
** H265_V4l2DeviceSource.h
**
** H265 V4L2 live555 source
**
** -------------------------------------------------------------------------*/

#pragma once

// project
#include "H26x_V4l2DeviceSource.h"

class H265_V4L2DeviceSource final : public H26X_V4L2DeviceSource {
public:
	H265_V4L2DeviceSource(
			UsageEnvironment &env, DeviceInterface *device, const int outputFd, const size_t queueSize,
			const CaptureMode captureMode, const bool repeatConfig
	)
		: H26X_V4L2DeviceSource(env, device, outputFd, queueSize, captureMode, repeatConfig) {}

protected:
	// overide V4L2DeviceSource
	std::list<std::pair<uint8_t const *, size_t>> splitFrames(uint8_t const *frame, size_t frameSize) override;
	std::list<std::basic_string<uint8_t>> getInitFrames() override;

protected:
	std::basic_string<uint8_t> m_vps;
};
