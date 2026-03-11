/* ---------------------------------------------------------------------------
** This software is in the public domain, furnished "as is", without technical
** support, and with no warranty, express or implied, as to its usefulness for
** any purpose.
**
** H26x_V4l2DeviceSource.cpp
**
** H264/H265 V4L2 Live555 source
**
** -------------------------------------------------------------------------*/

#include <algorithm>
#include <sstream>

// project
#include "H26x_V4l2DeviceSource.h"
#include "logger.h"

// extract a frame in Annex B bitstream
uint8_t const *
H26X_V4L2DeviceSource::extractFrame(uint8_t const *frame, size_t &size, size_t &outputSize, uint8_t &frameType) {
	outputSize = 0;
	frameType = 0;

	// short circuit
	if (size < sizeof(H26X_SHORT_MARKER))
		return nullptr;

	// end of input frame data
	uint8_t const *const frameLimit = frame + size;

	// find the marker. Note that anything that could match a long marker will match the short marker
	uint8_t const *const startFrame =
			std::search(frame, frameLimit, H26X_SHORT_MARKER, H26X_SHORT_MARKER + sizeof(H26X_SHORT_MARKER));
	if (startFrame == frameLimit) {
		LOG(INFO) << "No marker found";
		return nullptr;
	}

	// set the frame type
	uint8_t const * const outFrame = &startFrame[sizeof(H26X_SHORT_MARKER)];
	frameType = outFrame[0];

	// find the next marker. Might not find it
	uint8_t const *const endFrame =
			std::search(outFrame, frameLimit, H26X_SHORT_MARKER, H26X_SHORT_MARKER + sizeof(H26X_SHORT_MARKER));

	if (endFrame != frameLimit) {
		outputSize = endFrame - outFrame;
	} else {
		outputSize = size;
	}
	size -= outputSize;

	return outFrame;
}

std::basic_string<uint8_t> H26X_V4L2DeviceSource::getFrameWithMarker(const std::basic_string<uint8_t> &frame) {
	std::basic_string<uint8_t> frameWithMarker;
	frameWithMarker.append(H26X_MARKER, sizeof(H26X_MARKER));
	frameWithMarker.append(frame);
	return frameWithMarker;
}
