/* ---------------------------------------------------------------------------
** This software is in the public domain, furnished "as is", without technical
** support, and with no warranty, express or implied, as to its usefulness for
** any purpose.
**
** TSServerMediaSubsession.cpp
**
** -------------------------------------------------------------------------*/

#include "TSServerMediaSubsession.h"
#include "AddH26xMarkerFilter.h"
#include <algorithm>
#include <cstdint>
#include <memory>

TSServerMediaSubsession::TSServerMediaSubsession(
		UsageEnvironment &env, StreamReplicator *videoreplicator, StreamReplicator *audioreplicator,
		unsigned int sliceDuration
)
	: UnicastServerMediaSubsession(env, videoreplicator), m_slice(0) {
	// Create a source
	FramedSource *source = videoreplicator->createStreamReplica();
	MPEG2TransportStreamFromESSource *muxer = MPEG2TransportStreamFromESSource::createNew(env);

	if (m_format == "video/H264") {
		// add marker
		FramedSource *filter = new AddH26xMarkerFilter(env, source);
		// mux to TS
		muxer->addNewVideoSource(filter, 5);
	} else if (m_format == "video/H265") {
		// add marker
		FramedSource *filter = new AddH26xMarkerFilter(env, source);
		// mux to TS
		muxer->addNewVideoSource(filter, 6);
	}

	if (m_format == "audio/MPEG") {
		// mux to TS
		muxer->addNewAudioSource(source, 1);
	}

	FramedSource *tsSource = createSource(env, muxer, "video/MP2T");

	// Start Playing the HLS Sink
	m_hlsSink = MemoryBufferSink::createNew(env, OutPacketBuffer::maxSize, sliceDuration);
	m_hlsSink->startPlaying(*tsSource, nullptr, nullptr);
}

TSServerMediaSubsession::~TSServerMediaSubsession() { Medium::close(m_hlsSink); }

float TSServerMediaSubsession::getCurrentNPT(void *streamToken) { return (m_hlsSink->firstTime()); }

float TSServerMediaSubsession::duration() const { return (m_hlsSink->duration()); }

void TSServerMediaSubsession::seekStream(
		unsigned clientSessionId, void *streamToken, double &seekNPT, double streamDuration, u_int64_t &numBytes
) {
	m_slice = seekNPT / m_hlsSink->getSliceDuration();
	seekNPT = m_slice * m_hlsSink->getSliceDuration();
	numBytes = m_hlsSink->getBufferSize(m_slice);
	std::cout << "seek seekNPT:" << seekNPT << " slice:" << m_slice << " numBytes:" << numBytes << std::endl;
}

FramedSource *TSServerMediaSubsession::getStreamSource(void *streamToken) {
	FramedSource *source = nullptr;

	std::string buffer = m_hlsSink->getBuffer(m_slice);
	unsigned int size = buffer.size();
	if (size != 0) {
		auto content = std::make_unique<std::uint8_t[]>(size);
		std::copy(buffer.begin(), buffer.end(), content.get());
		source = ByteStreamMemoryBufferSource::createNew(envir(), content.release(), size);
	}
	return source;
}
