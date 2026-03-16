/* ---------------------------------------------------------------------------
** This software is in the public domain, furnished "as is", without technical
** support, and with no warranty, express or implied, as to its usefulness for
** any purpose.
**
** DeviceInterface.h
**
** -------------------------------------------------------------------------*/

#pragma once
#include <span>

// ---------------------------------
// Device Interface
// ---------------------------------
class DeviceInterface {
protected:
	explicit DeviceInterface(const bool isVideoDevice) : isVideoDevice(isVideoDevice) {}

public:
	virtual size_t read(std::span<uint8_t> buffer);
	virtual int getFd();
	virtual size_t getBufferSize();
	virtual ~DeviceInterface();

	const bool isVideoDevice;
};
