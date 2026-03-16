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

#include <list>
#include <mutex>
#include <string>
#include <thread>

// live555
#include <FramedSource.hh>
#include <UsageEnvironment.hh>
#include <utility>

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
		Frame(char *buffer, const size_t size, const timeval timestamp, char *allocatedBuffer = nullptr)
			: m_buffer(buffer), m_size(size), m_timestamp(timestamp), m_allocatedBuffer(allocatedBuffer) {};
		Frame(const Frame &);
		Frame &operator=(const Frame &);
		~Frame() { delete[] m_allocatedBuffer; };

		char *m_buffer;
		size_t m_size;
		timeval m_timestamp;
		char *m_allocatedBuffer;
	};

	// ---------------------------------
	// Compute simple stats
	// ---------------------------------
	class Stats {
	public:
		Stats(std::string msg) : m_fps(0), m_fps_sec(0), m_size(0), m_msg(std::move(msg)) {};

	public:
		int notify(long tv_sec, size_t framesize);

	protected:
		int m_fps;
		long m_fps_sec;
		size_t m_size;
		const std::string m_msg;
	};

	// ---------------------------------
	// Capture Mode
	// ---------------------------------
	enum CaptureMode { CAPTURE_LIVE555_THREAD = 0, CAPTURE_INTERNAL_THREAD, NO_CAPTURE };

public:
	static V4L2DeviceSource *createNew(
			UsageEnvironment &env, DeviceInterface *device, int outputFd, unsigned int queueSize,
			CaptureMode captureMode
	);
	std::string getAuxLine() { return m_auxLine; }
	std::string getLastFrame() {
		std::lock_guard lock(m_lastFrameMutex);
		std::string frame(m_lastFrame);
		return frame;
	}
	DeviceInterface *getDevice() { return m_device; }
	void postFrame(char *frame, int frameSize, const timeval &ref);
	virtual std::list<std::string> getInitFrames() { return {}; }
	virtual bool isKeyFrame(const char *, int) { return false; }

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
	ssize_t getNextFrame();
	void processFrame(char *frame, size_t frameSize, const timeval &ref);
	void postFrame(char *frame, ssize_t frameSize, const timeval &ref);
	void queueFrame(char *frame, size_t frameSize, const timeval &tv, char *allocatedBuffer = nullptr);

	// split packet in frames
	virtual std::list<std::pair<std::uint8_t *, size_t>> splitFrames(std::uint8_t *frame, unsigned frameSize);

	// overide FramedSource
	void doGetNextFrame() override;

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
	std::string m_lastFrame;
};
