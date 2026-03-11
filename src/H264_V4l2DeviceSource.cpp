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
std::list<std::pair<uint8_t const *, size_t>>
H264_V4L2DeviceSource::splitFrames(uint8_t const *frame, const size_t frameSize) {
	std::list<std::pair<uint8_t const *, size_t>> frameList;

	size_t bufSize = frameSize;
	size_t size = 0;
	uint8_t frameType = 0;
	uint8_t const *buffer = extractFrame(frame, bufSize, size, frameType);
	while (buffer != nullptr) {
		switch (frameType & 0x1F) {
		case 7:
			LOG(INFO) << "SPS size:" << size << " bufSize:" << bufSize;
			m_sps.assign(buffer, size);
			m_pps.clear();
			break;
		case 8:
			LOG(INFO) << "PPS size:" << size << " bufSize:" << bufSize;
			m_pps.assign(buffer, size);
			break;
		case 5:
			LOG(INFO) << "IDR size:" << size << " bufSize:" << bufSize;
			if (m_repeatConfig && !m_sps.empty() && !m_pps.empty()) {
				frameList.emplace_back(m_sps.data(), m_sps.size());
				frameList.emplace_back(m_pps.data(), m_pps.size());
			}
			if (!m_sps.empty() && !m_pps.empty()) {
				std::lock_guard lock(m_lastFrameMutex);
				m_lastFrame.assign(H26X_MARKER, sizeof(H26X_MARKER));
				m_lastFrame.append(m_sps.data(), m_sps.size());
				m_lastFrame.append(H26X_MARKER, sizeof(H26X_MARKER));
				m_lastFrame.append(m_pps.data(), m_pps.size());
				m_lastFrame.append(H26X_MARKER, sizeof(H26X_MARKER));
				m_lastFrame.append(buffer, size);
			}
			break;
		default:
			break;
		}

		if (!m_sps.empty() && !m_pps.empty()) {
			u_int32_t profile_level_id = 0;
			if (m_sps.size() >= 4)
				profile_level_id = m_sps[1] << 16 | m_sps[2] << 8 | m_sps[3];

			char const *const sps_base64 = base64Encode(reinterpret_cast<char const *>(m_sps.data()), m_sps.size());
			char const *const pps_base64 = base64Encode(reinterpret_cast<char const *>(m_pps.data()), m_pps.size());

			std::ostringstream os;
			os << "profile-level-id=" << std::hex << std::setw(6) << std::setfill('0') << profile_level_id;
			os << ";sprop-parameter-sets=" << sps_base64 << "," << pps_base64;
			m_auxLine.assign(os.str());

			delete[] sps_base64;
			delete[] pps_base64;
		}
		frameList.emplace_back(buffer, size);

		buffer = extractFrame(&buffer[size], bufSize, size, frameType);
	}
	return frameList;
}

std::list<std::basic_string<uint8_t>> H264_V4L2DeviceSource::getInitFrames() {
	std::list<std::basic_string<uint8_t>> frameList;
	frameList.push_back(getFrameWithMarker(m_sps));
	frameList.push_back(getFrameWithMarker(m_pps));
	return frameList;
}

bool H264_V4L2DeviceSource::isKeyFrame(const char *const buffer, const int size) {
	return size > 4 && (buffer[4] & 0x1F) == 5;
}