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
#include <cstdint>

// live555
#include <Base64.hh>

// project
#include "H26x_V4l2DeviceSource.h"
#include "logger.h"

namespace {

template <typename T> const T *memmem(const T *haystack, size_t haystackSize, const T *needle, size_t needleSize) {
	if (needleSize == 0 || haystackSize < needleSize) {
		return nullptr;
	}
	auto it = std::search(haystack, haystack + haystackSize - needleSize + 1, needle, needle + needleSize);
	return (it != haystack + haystackSize - needleSize + 1) ? it : nullptr;
}

} // namespace

std::uint8_t *H26X_V4L2DeviceSource::extractFrame(std::uint8_t *frame, size_t &size, size_t &outsize, int &frameType) {
	std::uint8_t *outFrame = nullptr;
	outsize = 0;
	unsigned int markerlength = 0;
	frameType = 0;

	std::uint8_t *startFrame = reinterpret_cast<std::uint8_t *>(
			::memmem(frame, size, reinterpret_cast<const std::uint8_t *>(H264marker), sizeof(H264marker))
	);
	if (startFrame != nullptr) {
		markerlength = sizeof(H264marker);
	} else {
		startFrame = reinterpret_cast<std::uint8_t *>(
				::memmem(frame, size, reinterpret_cast<const std::uint8_t *>(H264shortmarker), sizeof(H264shortmarker))
		);
		if (startFrame != nullptr) {
			markerlength = sizeof(H264shortmarker);
		}
	}
	if (startFrame != nullptr) {
		frameType = startFrame[markerlength];

		size_t remainingSize = size - (startFrame - frame + markerlength);
		std::uint8_t *endFrame = reinterpret_cast<std::uint8_t *>(::memmem(
				&startFrame[markerlength], remainingSize, reinterpret_cast<const std::uint8_t *>(H264marker),
				sizeof(H264marker)
		));
		if (endFrame == nullptr) {
			endFrame = reinterpret_cast<std::uint8_t *>(::memmem(
					&startFrame[markerlength], remainingSize, reinterpret_cast<const std::uint8_t *>(H264shortmarker),
					sizeof(H264shortmarker)
			));
		}

		if (m_keepMarker) {
			size -= static_cast<size_t>(startFrame - frame);
			outFrame = startFrame;
		} else {
			size -= static_cast<size_t>(startFrame - frame + markerlength);
			outFrame = &startFrame[markerlength];
		}

		if (endFrame != nullptr) {
			outsize = static_cast<size_t>(endFrame - outFrame);
		} else {
			outsize = size;
		}
		size -= outsize;
	} else if (size >= sizeof(H264shortmarker)) {
		LOG(INFO) << "No marker found";
	}

	return outFrame;
}

std::string H26X_V4L2DeviceSource::getFrameWithMarker(const std::string &frame) {
	std::string frameWithMarker;
	frameWithMarker.append(H264marker, sizeof(H264marker));
	frameWithMarker.append(frame);
	return frameWithMarker;
}
