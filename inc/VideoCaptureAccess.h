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
	VideoCaptureAccess(V4l2Capture *device) : m_device(device) {}
	~VideoCaptureAccess() override { delete m_device; }

	size_t read(char *buffer, size_t bufferSize) override { return m_device->read(buffer, bufferSize); }
	int getFd() override { return m_device->getFd(); }
	unsigned long getBufferSize() override { return m_device->getBufferSize(); }
	int getWidth() override { return m_device->getWidth(); }
	int getHeight() override { return m_device->getHeight(); }
	int getVideoFormat() override { return m_device->getFormat(); }

protected:
	V4l2Capture *m_device;
};
