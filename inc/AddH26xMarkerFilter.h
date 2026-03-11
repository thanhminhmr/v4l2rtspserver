/* ---------------------------------------------------------------------------
** This software is in the public domain, furnished "as is", without technical
** support, and with no warranty, express or implied, as to its usefulness for
** any purpose.
**
** AddH26xMarkerFilter.h
**
** -------------------------------------------------------------------------*/

#pragma once

#include <algorithm>
#include <cstdint>
#include <memory>
#include <vector>

class AddH26xMarkerFilter : public FramedFilter {
public:
	AddH26xMarkerFilter(UsageEnvironment &env, FramedSource *inputSource)
		: FramedFilter(env, inputSource), m_bufferSize(OutPacketBuffer::maxSize) {
		m_buffer.resize(m_bufferSize);
	}

	~AddH26xMarkerFilter() = default;

private:
	static void afterGettingFrame(
			void *clientData, unsigned frameSize, unsigned numTruncatedBytes, struct timeval presentationTime,
			unsigned durationInMicroseconds
	) {
		auto *sink = reinterpret_cast<AddH26xMarkerFilter *>(clientData);
		sink->afterGettingFrame(frameSize, numTruncatedBytes, presentationTime);
	}

	void afterGettingFrame(unsigned frameSize, unsigned numTruncatedBytes, struct timeval presentationTime) {
		fPresentationTime = presentationTime;
		fDurationInMicroseconds = 0;
		if (numTruncatedBytes > 0) {
			envir() << "AddH26xMarkerFilter::afterGettingFrame(): The input frame data was too large for our buffer "
					   "size truncated:"
					<< numTruncatedBytes << " bufferSize:" << m_bufferSize << "\n";
			m_bufferSize += numTruncatedBytes;
			m_buffer.resize(m_bufferSize);
			fFrameSize = 0;
		} else {
			char marker[] = {0, 0, 0, 1};
			fFrameSize = frameSize + sizeof(marker);
			if (fFrameSize > fMaxSize) {
				fNumTruncatedBytes = fFrameSize - fMaxSize;
				envir() << "AddH26xMarkerFilter::afterGettingFrame(): buffer too small truncated:" << fNumTruncatedBytes
						<< " bufferSize:" << fFrameSize << "\n";
			} else {
				fNumTruncatedBytes = 0;
				std::copy_n(marker, sizeof(marker), fTo);
				std::copy_n(m_buffer.data(), frameSize, fTo + sizeof(marker));
			}
		}
		afterGetting(this);
	}

	void doGetNextFrame() override {
		if (fInputSource != nullptr) {
			fInputSource->getNextFrame(m_buffer.data(), m_bufferSize, afterGettingFrame, this, handleClosure, this);
		}
	}

	std::vector<std::uint8_t> m_buffer;
	unsigned int m_bufferSize;
};
