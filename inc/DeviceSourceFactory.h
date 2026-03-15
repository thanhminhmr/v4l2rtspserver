/* ---------------------------------------------------------------------------
** This software is in the public domain, furnished "as is", without technical
** support, and with no warranty, express or implied, as to its usefulness for
** any purpose.
**
** DeviceSourceFactory.h
**
** V4L2 live555 source
**
** -------------------------------------------------------------------------*/

#pragma once

#include <StreamReplicator.hh>
#include <MediaSink.hh>
#include <linux/videodev2.h>

#include "H264_V4l2DeviceSource.h"
#include "H265_V4l2DeviceSource.h"
#include "V4L2DeviceSource.h"

class DeviceSourceFactory {
public:
	static FramedSource *createFramedSource(
			UsageEnvironment *env, const int format, DeviceInterface *devCapture, const int queueSize = 5,
			const V4L2DeviceSource::CaptureMode captureMode = V4L2DeviceSource::CAPTURE_INTERNAL_THREAD,
			const int outFd = -1, const bool repeatConfig = true
	) {
		FramedSource *source = nullptr;
		if (format == V4L2_PIX_FMT_H264) {
			source = H264_V4L2DeviceSource::createNew(
					*env, devCapture, outFd, queueSize, captureMode, repeatConfig, false
			);
		} else if (format == V4L2_PIX_FMT_HEVC) {
			source = H265_V4L2DeviceSource::createNew(
					*env, devCapture, outFd, queueSize, captureMode, repeatConfig, false
			);
		} else {
			source = V4L2DeviceSource::createNew(*env, devCapture, outFd, queueSize, captureMode);
		}
		return source;
	}

	static StreamReplicator *createStreamReplicator(
			UsageEnvironment *env, const int format, DeviceInterface *devCapture, const int queueSize = 5,
			const V4L2DeviceSource::CaptureMode captureMode = V4L2DeviceSource::CAPTURE_INTERNAL_THREAD,
			const int outFd = -1, const bool repeatConfig = true
	) {
		StreamReplicator *replicator = nullptr;
		FramedSource *framedSource =
				createFramedSource(env, format, devCapture, queueSize, captureMode, outFd, repeatConfig);
		if (framedSource != nullptr) {
			// extend buffer size if needed
			if (devCapture->getBufferSize() > OutPacketBuffer::maxSize) {
				OutPacketBuffer::maxSize = devCapture->getBufferSize();
			}
			replicator = StreamReplicator::createNew(*env, framedSource, false);
		}
		return replicator;
	}
};
