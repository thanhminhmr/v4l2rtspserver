/* ---------------------------------------------------------------------------
** This software is in the public domain, furnished "as is", without technical
** support, and with no warranty, express or implied, as to its usefulness for
** any purpose.
**
** TSServerMediaSubsession.h
**
** -------------------------------------------------------------------------*/

#pragma once

#include "MemoryBufferSink.h"
#include "UnicastServerMediaSubsession.h"
#include <map>

// -----------------------------------------
//    ServerMediaSubsession for HLS
// -----------------------------------------
class TSServerMediaSubsession : public UnicastServerMediaSubsession {
public:
	static TSServerMediaSubsession *createNew(
			UsageEnvironment &env, StreamReplicator *videoReplicator, StreamReplicator *audioReplicator,
			const unsigned int sliceDuration
	) {
		return new TSServerMediaSubsession(env, videoReplicator, audioReplicator, sliceDuration);
	}

protected:
	TSServerMediaSubsession(
			UsageEnvironment &env, StreamReplicator *videoReplicator, StreamReplicator *audioReplicator,
			unsigned int sliceDuration
	);
	~TSServerMediaSubsession() override;

	float getCurrentNPT(void *streamToken) override;
	[[nodiscard]] float duration() const override;
	void seekStream(
			unsigned clientSessionId, void *streamToken, double &seekNPT, double streamDuration, std::uint64_t &numBytes
	) override;
	FramedSource *getStreamSource(void *streamToken) override;

protected:
	unsigned int m_slice;
	MemoryBufferSink *m_hlsSink;
};
