/* ---------------------------------------------------------------------------
** This software is in the public domain, furnished "as is", without technical
** support, and with no warranty, express or implied, as to its usefulness for
** any purpose.
**
** DeviceInterface.h
**
** -------------------------------------------------------------------------*/

#pragma once
#include <cstddef>
#include <list>
#include <sys/types.h>

// ---------------------------------
// Device Interface
// ---------------------------------
class DeviceInterface {
public:
	virtual ssize_t read(char *buffer, size_t bufferSize) = 0;
	virtual int getFd() = 0;
	virtual size_t getBufferSize() = 0;
	virtual int getWidth() { return -1; }
	virtual int getHeight() { return -1; }
	virtual int getVideoFormat() { return -1; }
	virtual std::list<int> getVideoFormatList() { return {}; }
	virtual int getSampleRate() { return -1; }
	virtual int getChannels() { return -1; }
	virtual int getAudioFormat() { return -1; }
	virtual std::list<int> getAudioFormatList() { return {}; }
	virtual ~DeviceInterface() = default;
};
