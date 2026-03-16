/* ---------------------------------------------------------------------------
** This software is in the public domain, furnished "as is", without technical
** support, and with no warranty, express or implied, as to its usefulness for
** any purpose.
**
** DeviceSource.h
**
**  live555 source
**
** -------------------------------------------------------------------------*/

#pragma once

#include "DeviceInterface.h"
#include "V4l2Capture.h"

// -----------------------------------------
//    Video Device Capture Interface
// -----------------------------------------
class VideoCaptureAccess : public DeviceInterface {
public:
	explicit VideoCaptureAccess(V4l2Capture *device) : DeviceInterface(true), m_device(device) {}
	~VideoCaptureAccess() override { delete m_device; }

	size_t read(std::span<uint8_t> buffer) override { return m_device->read(reinterpret_cast<char *>(buffer.data()), buffer.size()); }
	int getFd() override { return m_device->getFd(); }
	unsigned long getBufferSize() override { return m_device->getBufferSize(); }

	[[nodiscard]] unsigned int getWidth() const { return m_device->getWidth(); }
	[[nodiscard]] unsigned int getHeight() const { return m_device->getHeight(); }
	[[nodiscard]] unsigned int getVideoFormat() const { return m_device->getFormat(); }

protected:
	V4l2Capture *m_device;
};
