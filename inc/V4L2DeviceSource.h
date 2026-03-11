/* ---------------------------------------------------------------------------
** This software is in the public domain, furnished "as is", without technical
** support, and with no warranty, express or implied, as to its usefulness for
** any purpose.
**
** V4L2DeviceSource.h
**
**  live555 source
**
** -------------------------------------------------------------------------*/

#pragma once

#include <iomanip>
#include <iostream>
#include <list>
#include <mutex>
#include <string>
#include <thread>

// live555
#include <liveMedia.hh>

#include "DeviceInterface.h"

// -----------------------------------------
//    Video Device Source
// -----------------------------------------
class V4L2DeviceSource : public FramedSource {
public:
	// ---------------------------------
	// Captured frame
	// ---------------------------------
	struct Frame {
		Frame(char *buffer, int size, timeval timestamp, char *allocatedBuffer = nullptr)
			: m_buffer(buffer), m_size(size), m_timestamp(timestamp), m_allocatedBuffer(allocatedBuffer) {};
		Frame(const Frame &);
		Frame &operator=(const Frame &);
		~Frame() { delete[] m_allocatedBuffer; };

		char *m_buffer;
		unsigned int m_size;
		timeval m_timestamp;
		char *m_allocatedBuffer;
	};

	// ---------------------------------
	// Compute simple stats
	// ---------------------------------
	class Stats {
	public:
		Stats(const std::string &msg) : m_fps(0), m_fps_sec(0), m_size(0), m_msg(msg) {};

	public:
		int notify(int tv_sec, int framesize);

	protected:
		int m_fps;
		int m_fps_sec;
		int m_size;
		const std::string m_msg;
	};

	// ---------------------------------
	// Capture Mode
	// ---------------------------------
	enum CaptureMode { CAPTURE_LIVE555_THREAD = 0, CAPTURE_INTERNAL_THREAD, NOCAPTURE };

public:
	static V4L2DeviceSource *createNew(
			UsageEnvironment &env, DeviceInterface *device, int outputFd, unsigned int queueSize,
			CaptureMode captureMode
	);
	std::string getAuxLine() { return m_auxLine; }
	std::basic_string<uint8_t> getLastFrame() {
		std::lock_guard lock(m_lastFrameMutex);
		std::basic_string frame(m_lastFrame);
		return frame;
	}

	DeviceInterface *getDevice() const { return m_device; }
	void postFrame(char *frame, int frameSize, const timeval &ref);
	virtual std::list<std::basic_string<uint8_t>> getInitFrames() { return {}; }

protected:
	V4L2DeviceSource(
			UsageEnvironment &env, DeviceInterface *device, int outputFd, unsigned int queueSize,
			CaptureMode captureMode
	);
	~V4L2DeviceSource() override;

protected:
	virtual void *thread();
	static void deliverFrameStub(void *clientData) { static_cast<V4L2DeviceSource *>(clientData)->deliverFrame(); };
	void deliverFrame();
	static void incomingPacketHandlerStub(void *clientData, int mask) {
		static_cast<V4L2DeviceSource *>(clientData)->incomingPacketHandler();
	};
	void incomingPacketHandler();
	int getNextFrame();
	void processFrame(char *frame, int frameSize, const timeval &ref);
	void queueFrame(char *frame, int frameSize, const timeval &tv, char *allocatedBuffer = nullptr);

	// split packet in frames
	virtual std::list<std::pair<uint8_t const *, size_t>> splitFrames(uint8_t const *frame, size_t frameSize);

	// overide FramedSource
	virtual void doGetNextFrame();

protected:
	std::list<Frame *> m_captureQueue;
	Stats m_in;
	Stats m_out;
	EventTriggerId m_eventTriggerId;
	int m_outfd;
	DeviceInterface *m_device;
	unsigned int m_queueSize;
	std::thread m_thread;
	std::mutex m_mutex;
	std::string m_auxLine;
	std::mutex m_lastFrameMutex;
	std::basic_string<uint8_t> m_lastFrame;
};
