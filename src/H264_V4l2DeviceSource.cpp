/* ---------------------------------------------------------------------------
** This software is in the public domain, furnished "as is", without technical
** support, and with no warranty, express or implied, as to its usefulness for
** any purpose.
**
** H264_V4l2DeviceSource.cpp
**
** H264 V4L2 Live555 source
**
** -------------------------------------------------------------------------*/

#include <cstdint>
#include <memory>
#include <sstream>

// live555
#include <Base64.hh>

// project
#include "H264_V4l2DeviceSource.h"
#include "logger.h"

// ---------------------------------
// H264 V4L2 FramedSource
// ---------------------------------

// split packet in frames
std::list<std::pair<std::uint8_t *, size_t>>
H264_V4L2DeviceSource::splitFrames(std::uint8_t *frame, unsigned frameSize) {
	std::list<std::pair<std::uint8_t *, size_t>> frameList;

	size_t bufSize = frameSize;
	size_t size = 0;
	int frameType = 0;
	std::uint8_t *buffer = this->extractFrame(frame, bufSize, size, frameType);
	while (buffer != nullptr) {
		switch (frameType & 0x1F) {
		case 7:
			LOG(INFO) << "SPS size:" << size << " bufSize:" << bufSize;
			m_sps.assign(reinterpret_cast<const char *>(buffer), size);
			m_pps.clear();
			break;
		case 8:
			LOG(INFO) << "PPS size:" << size << " bufSize:" << bufSize;
			m_pps.assign(reinterpret_cast<const char *>(buffer), size);
			break;
		case 5:
			LOG(INFO) << "IDR size:" << size << " bufSize:" << bufSize;
			if (m_repeatConfig && !m_sps.empty() && !m_pps.empty()) {
				frameList.push_back(std::pair<std::uint8_t *, size_t>(
						reinterpret_cast<std::uint8_t *>(const_cast<char *>(m_sps.c_str())), m_sps.size()
				));
				frameList.push_back(std::pair<std::uint8_t *, size_t>(
						reinterpret_cast<std::uint8_t *>(const_cast<char *>(m_pps.c_str())), m_pps.size()
				));
			}
			if (!m_sps.empty() && !m_pps.empty()) {
				std::lock_guard<std::mutex> lock(m_lastFrameMutex);
				m_lastFrame.assign(H264marker, sizeof(H264marker));
				m_lastFrame.append(m_sps.c_str(), m_sps.size());
				m_lastFrame.append(H264marker, sizeof(H264marker));
				m_lastFrame.append(m_pps.c_str(), m_pps.size());
				m_lastFrame.append(H264marker, sizeof(H264marker));
				m_lastFrame.append(reinterpret_cast<const char *>(buffer), size);
			}
			break;
		default:
			break;
		}

		if (!m_sps.empty() && !m_pps.empty()) {
			u_int32_t profile_level_id = 0;
			if (m_sps.size() >= 4)
				profile_level_id = (static_cast<std::uint8_t>(m_sps[1]) << 16) |
								   (static_cast<std::uint8_t>(m_sps[2]) << 8) | static_cast<std::uint8_t>(m_sps[3]);

			std::unique_ptr<char[]> sps_base64(base64Encode(m_sps.c_str(), m_sps.size()));
			std::unique_ptr<char[]> pps_base64(base64Encode(m_pps.c_str(), m_pps.size()));

			std::ostringstream os;
			os << "profile-level-id=" << std::hex << std::setw(6) << std::setfill('0') << profile_level_id;
			os << ";sprop-parameter-sets=" << sps_base64.get() << "," << pps_base64.get();
			m_auxLine.assign(os.str());
		}
		frameList.push_back(std::pair<std::uint8_t *, size_t>(buffer, size));

		buffer = this->extractFrame(&buffer[size], bufSize, size, frameType);
	}
	return frameList;
}

std::list<std::string> H264_V4L2DeviceSource::getInitFrames() {
	std::list<std::string> frameList;
	frameList.push_back(this->getFrameWithMarker(m_sps));
	frameList.push_back(this->getFrameWithMarker(m_pps));
	return frameList;
}

bool H264_V4L2DeviceSource::isKeyFrame(const char *buffer, int size) {
	bool res = false;
	if (size > 4) {
		int frameType = buffer[4] & 0x1F;
		res = (frameType == 5);
	}
	return res;
}