/* ---------------------------------------------------------------------------
** This software is in the public domain, furnished "as is", without technical
** support, and with no warranty, express or implied, as to its usefulness for
** any purpose.
**
** ServerMediaSubsession.h
**
** -------------------------------------------------------------------------*/

#pragma once

#include "BaseServerMediaSubsession.h"

// -----------------------------------------
//    ServerMediaSubsession for Unicast
// -----------------------------------------
class UnicastServerMediaSubsession : public BaseServerMediaSubsession, public OnDemandServerMediaSubsession {
public:
	static UnicastServerMediaSubsession *createNew(UsageEnvironment &env, StreamReplicator *replicator);

protected:
	UnicastServerMediaSubsession(UsageEnvironment &env, StreamReplicator *replicator)
		: BaseServerMediaSubsession(replicator), OnDemandServerMediaSubsession(env, False) {}

	FramedSource *createNewStreamSource(unsigned clientSessionId, unsigned &estBitrate) override;
	RTPSink *createNewRTPSink(
			Groupsock *rtpGroupSock, unsigned char rtpPayloadTypeIfDynamic, FramedSource *inputSource
	) override;
	char const *getAuxSDPLine(RTPSink *rtpSink, FramedSource *inputSource) override;
};
