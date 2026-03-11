/* ---------------------------------------------------------------------------
** This software is in the public domain, furnished "as is", without technical
** support, and with no warranty, express or implied, as to its usefulness for
** any purpose.
**
** H265_V4l2DeviceSource.cpp
**
** H265 V4L2 Live555 source
**
** -------------------------------------------------------------------------*/

#include <sstream>

// live555
#include <Base64.hh>

// project
#include "H265_V4l2DeviceSource.h"
#include "logger.h"

// split packet in frames
std::list<std::pair<uint8_t const *, size_t>>
H265_V4L2DeviceSource::splitFrames(uint8_t const *frame, const size_t frameSize) {
	std::list<std::pair<uint8_t const *, size_t>> frameList;

	size_t bufSize = frameSize;
	size_t size = 0;
	uint8_t frameType = 0;
	uint8_t const *buffer = extractFrame(frame, bufSize, size, frameType);
	while (buffer != nullptr) {
		switch ((frameType & 0x7E) >> 1) {
		case 32:
			LOG(INFO) << "VPS size:" << size << " bufSize:" << bufSize;
			m_vps.assign(buffer, size);
			m_sps.clear();
			m_pps.clear();
			break;
		case 33:
			LOG(INFO) << "SPS size:" << size << " bufSize:" << bufSize;
			m_sps.assign(buffer, size);
			break;
		case 34:
			LOG(INFO) << "PPS size:" << size << " bufSize:" << bufSize;
			m_pps.assign(buffer, size);
			break;
		case 19:
		case 20:
			LOG(INFO) << "IDR size:" << size << " bufSize:" << bufSize;
			if (m_repeatConfig && !m_vps.empty() && !m_sps.empty() && !m_pps.empty()) {
				frameList.emplace_back(m_vps.data(), m_vps.size());
				frameList.emplace_back(m_sps.data(), m_sps.size());
				frameList.emplace_back(m_pps.data(), m_pps.size());
			}
			if (!m_vps.empty() && !m_sps.empty() && !m_pps.empty()) {
				std::lock_guard lock(m_lastFrameMutex);
				m_lastFrame.assign(H26X_MARKER, sizeof(H26X_MARKER));
				m_lastFrame.append(m_vps.data(), m_vps.size());
				m_lastFrame.append(H26X_MARKER, sizeof(H26X_MARKER));
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

		if (!m_vps.empty() && !m_sps.empty() && !m_pps.empty()) {
			char const *const vps_base64 = base64Encode(reinterpret_cast<char const *>(m_vps.data()), m_vps.size());
			char const *const sps_base64 = base64Encode(reinterpret_cast<char const *>(m_sps.data()), m_sps.size());
			char const *const pps_base64 = base64Encode(reinterpret_cast<char const *>(m_pps.data()), m_pps.size());

			std::ostringstream os;
			os << "sprop-vps=" << vps_base64;
			os << ";sprop-sps=" << sps_base64;
			os << ";sprop-pps=" << pps_base64;
			m_auxLine.assign(os.str());

			delete[] vps_base64;
			delete[] sps_base64;
			delete[] pps_base64;
		}
		frameList.emplace_back(buffer, size);

		buffer = extractFrame(&buffer[size], bufSize, size, frameType);
	}
	return frameList;
}

std::list<std::basic_string<uint8_t>> H265_V4L2DeviceSource::getInitFrames() {
	std::list<std::basic_string<uint8_t>> frameList;
	frameList.push_back(getFrameWithMarker(m_vps));
	frameList.push_back(getFrameWithMarker(m_sps));
	frameList.push_back(getFrameWithMarker(m_pps));
	return frameList;
}

bool H265_V4L2DeviceSource::isKeyFrame(const char *buffer, const int size) {
	if (size > 4) {
		const int frameType = (buffer[4] & 0x7E) >> 1;
		return frameType == 19 || frameType == 20;
	}
	return false;
}