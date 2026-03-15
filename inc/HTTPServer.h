/* ---------------------------------------------------------------------------
** This software is in the public domain, furnished "as is", without technical
** support, and with no warranty, express or implied, as to its usefulness for
** any purpose.
**
** HTTPServer.h
**
** V4L2 RTSP streamer
**
** HTTP server that serves HLS & MPEG-DASH playlist and segments
**
** -------------------------------------------------------------------------*/

#pragma once

#include <list>
#include <sstream>
#include <string>

// hacking private members RTSPServer::fWeServeSRTP & RTSPServer::fWeEncryptSRTP
#ifndef _RTSP_SERVER_HH
#define private protected
#include "RTSPServer.hh"
#undef private
#else
#error "RTSPServer.hh already included, cannot hacking access to private members"
#endif

#include <GroupsockHelper.hh> // for "ignoreSigPipeOnSocket()"
#include <utility>

#define TCP_STREAM_SINK_MIN_READ_SIZE 1000
#define TCP_STREAM_SINK_BUFFER_SIZE 10000

class TCPSink : public MediaSink {
public:
	TCPSink(UsageEnvironment &env, const int socketNum)
		: MediaSink(env), fUnwrittenBytesStart(0), fUnwrittenBytesEnd(0), fInputSourceIsOpen(False),
		  fOutputSocketIsWritable(True), fOutputSocketNum(socketNum) {
		ignoreSigPipeOnSocket(socketNum);
	}

protected:
	~TCPSink() override { envir().taskScheduler().disableBackgroundHandling(fOutputSocketNum); }

protected:
	Boolean continuePlaying() override {
		fInputSourceIsOpen = fSource != nullptr;
		processBuffer();
		return True;
	}

private:
	void processBuffer() {
		// First, try writing data to our output socket, if we can:
		if (fOutputSocketIsWritable && numUnwrittenBytes() > 0) {
			const size_t numBytesWritten =
					send(fOutputSocketNum, &fBuffer[fUnwrittenBytesStart], numUnwrittenBytes(), 0);
			if (numBytesWritten < numUnwrittenBytes()) {
				// The output socket is no longer writable.  Set a handler to be called when it becomes writable again.
				fOutputSocketIsWritable = False;
				if (envir().getErrno() !=
					EPIPE) { // on this error, the socket might still be writable, but no longer usable
					envir().taskScheduler().setBackgroundHandling(
							fOutputSocketNum, SOCKET_WRITABLE, socketWritableHandler, this
					);
				}
			}
			if (numBytesWritten > 0) {
				// We wrote at least some of our data.  Update our buffer pointers:
				fUnwrittenBytesStart += numBytesWritten;
				if (fUnwrittenBytesStart > fUnwrittenBytesEnd)
					fUnwrittenBytesStart = fUnwrittenBytesEnd; // sanity check
				if (fUnwrittenBytesStart == fUnwrittenBytesEnd &&
					(!fInputSourceIsOpen || !fSource->isCurrentlyAwaitingData())) {
					fUnwrittenBytesStart = fUnwrittenBytesEnd = 0; // reset the buffer to empty
				}
			}
		}

		// Then, read from our input source, if we can (& we're not already reading from it):
		if (fInputSourceIsOpen && freeBufferSpace() >= TCP_STREAM_SINK_MIN_READ_SIZE &&
			!fSource->isCurrentlyAwaitingData()) {
			fSource->getNextFrame(
					&fBuffer[fUnwrittenBytesEnd], freeBufferSpace(), afterGettingFrame, this, ourOnSourceClosure, this
			);
		} else if (!fInputSourceIsOpen && numUnwrittenBytes() == 0) {
			// We're now done:
			onSourceClosure();
		}
	}

	static void socketWritableHandler(void *clientData, int mask) {
		static_cast<TCPSink *>(clientData)->socketWritableHandler();
	}
	void socketWritableHandler() {
		envir().taskScheduler().disableBackgroundHandling(
				fOutputSocketNum
		); // disable this handler until the next time it's needed
		fOutputSocketIsWritable = True;
		processBuffer();
	}

	static void afterGettingFrame(
			void *clientData, const unsigned frameSize, const unsigned numTruncatedBytes,
			[[maybe_unused]] const timeval presentationTime, [[maybe_unused]] const unsigned durationInMicroseconds
	) {
		static_cast<TCPSink *>(clientData)->afterGettingFrame(frameSize, numTruncatedBytes);
	}

	void afterGettingFrame(const unsigned frameSize, const unsigned numTruncatedBytes) {
		if (numTruncatedBytes > 0) {
			envir() << "TCPStreamSink::afterGettingFrame(): The input frame data was too large for our buffer.  "
					<< numTruncatedBytes
					<< " bytes of trailing data was dropped!  Correct this by increasing the definition of "
					   "\"TCP_STREAM_SINK_BUFFER_SIZE\" in \"include/TCPStreamSink.hh\".\n";
		}
		fUnwrittenBytesEnd += frameSize;
		processBuffer();
	}

	static void ourOnSourceClosure(void *clientData) { static_cast<TCPSink *>(clientData)->ourOnSourceClosure(); }

	void ourOnSourceClosure() {
		// The input source has closed:
		fInputSourceIsOpen = False;
		processBuffer();
	}

	[[nodiscard]] size_t numUnwrittenBytes() const { return fUnwrittenBytesEnd - fUnwrittenBytesStart; }
	[[nodiscard]] size_t freeBufferSpace() const { return TCP_STREAM_SINK_BUFFER_SIZE - fUnwrittenBytesEnd; }

private:
	uint8_t fBuffer[TCP_STREAM_SINK_BUFFER_SIZE]{};
	size_t fUnwrittenBytesStart, fUnwrittenBytesEnd;
	Boolean fInputSourceIsOpen, fOutputSocketIsWritable;
	int fOutputSocketNum;
};

// ---------------------------------------------------------
//  Extend RTSP server to add support for HLS and MPEG-DASH
// ---------------------------------------------------------
class HTTPServer : public RTSPServer {
	class HTTPClientConnection : public RTSPClientConnection {
	public:
		HTTPClientConnection(
				RTSPServer &ourServer, const int clientSocket, sockaddr_storage const &clientAddr, const Boolean useTLS
		)
			: RTSPClientConnection(ourServer, clientSocket, clientAddr, useTLS), m_TCPSink(nullptr),
			  m_StreamToken(nullptr), m_Subsession(nullptr), m_Source(nullptr) {}
		~HTTPClientConnection() override;

	private:
		void sendHeader(const char *contentType, unsigned int contentLength);
		void streamSource(FramedSource *source);
		void streamSource(const std::string &content);
		ServerMediaSubsession *getSubsession(const char *urlSuffix);
		bool sendFile(char const *urlSuffix);
		bool sendM3u8PlayList(char const *urlSuffix);
		bool sendMpdPlayList(char const *urlSuffix);
		void handleHTTPCmd_StreamingGET(char const *urlSuffix, char const *fullRequestStr) override;
		void handleCmd_notFound() override;
		static void afterStreaming(void *clientData);

	private:
		static u_int32_t m_ClientSessionId;
		TCPSink *m_TCPSink;
		void *m_StreamToken;
		ServerMediaSubsession *m_Subsession;
		FramedSource *m_Source;
	};

	class HTTPClientSession : public RTSPClientSession {
	public:
		HTTPClientSession(HTTPServer &ourServer, const u_int32_t sessionId) : RTSPClientSession(ourServer, sessionId) {}
		void handleCmd_SETUP(
				RTSPClientConnection *ourClientConnection, char const *urlPreSuffix, char const *urlSuffix,
				char const *fullRequestStr
		) override;
	};

	class MyUserAuthenticationDatabase : public UserAuthenticationDatabase {
	public:
		MyUserAuthenticationDatabase(char const *realm = nullptr, const Boolean passwordsAreMD5 = False)
			: UserAuthenticationDatabase(realm, passwordsAreMD5) {}
		~MyUserAuthenticationDatabase() override = default;

		[[nodiscard]] std::list<std::string> getUsers() const {
			std::list<std::string> users;
			HashTable::Iterator *iter = HashTable::Iterator::create(*fTable);
			char const *key;
			char *user;
			while ((user = static_cast<char *>(iter->next(key))) != nullptr) {
				users.emplace_back(user);
			}
			return users;
		}

		static MyUserAuthenticationDatabase *
		createNew(const std::list<std::string> &userPasswordList, const char *realm) {
			MyUserAuthenticationDatabase *auth = nullptr;
			if (!userPasswordList.empty()) {
				auth = new MyUserAuthenticationDatabase(realm, realm != nullptr);
				for (const auto &it : userPasswordList) {
					std::istringstream is(it);
					std::string user;
					getline(is, user, ':');
					std::string password;
					getline(is, password);
					auth->addUserRecord(user.c_str(), password.c_str());
				}
			}

			return auth;
		}
	};

public:
	static HTTPServer *createNew(
			UsageEnvironment &env, Port rtspPort, const std::list<std::string> &userPasswordList, const char *realm,
			const unsigned reclamationTestSeconds, const unsigned int hlsSegment, const std::string &webroot,
			const std::string &sslCert, const bool enableRTSPS
	) {
		HTTPServer *httpServer = nullptr;
		const int ourSocketIPv4 = setUpOurSocket(env, rtspPort, AF_INET);
		const int ourSocketIPv6 = setUpOurSocket(env, rtspPort, AF_INET6);

		if (ourSocketIPv4 != -1) {
			MyUserAuthenticationDatabase *authDatabase =
					MyUserAuthenticationDatabase::createNew(userPasswordList, realm);
			httpServer = new HTTPServer(
					env, ourSocketIPv4, ourSocketIPv6, rtspPort, authDatabase, reclamationTestSeconds, hlsSegment,
					webroot, sslCert, enableRTSPS
			);
		}
		return httpServer;
	}

	HTTPServer(
			UsageEnvironment &env, const int ourSocketIPv4, const int ourSocketIPv6, const Port rtspPort,
			MyUserAuthenticationDatabase *authDatabase, const unsigned reclamationTestSeconds,
			const unsigned int hlsSegment, std::string webroot, const std::string &sslCert, const bool enableRTSPS
	)
		: RTSPServer(env, ourSocketIPv4, ourSocketIPv6, rtspPort, authDatabase, reclamationTestSeconds),
		  m_hlsSegment(hlsSegment), m_webroot(std::move(webroot)) {
		if (!m_webroot.empty() && *m_webroot.rend() != '/') {
			m_webroot += "/";
		}
		this->setTLS(sslCert, enableRTSPS);
	}

	ClientConnection *createNewClientConnection(const int clientSocket, sockaddr_storage const &clientAddr) override {
		return new HTTPClientConnection(*this, clientSocket, clientAddr, isRTSPS());
	}

	ClientSession *createNewClientSession(const u_int32_t sessionId) override {
		return new HTTPClientSession(*this, sessionId);
	}

	void setTLS(const std::string &sslCert, const bool enableRTSPS = false, const bool encryptSRTP = true) {
		if (!sslCert.empty()) {
			this->setTLSFileNames(sslCert.c_str(), sslCert.c_str());
			fWeServeSRTP = true;
			fWeEncryptSRTP = encryptSRTP;
			if (enableRTSPS) {
				fOurConnectionsUseTLS = true;
			} else {
				fOurConnectionsUseTLS = false;
			}
		} else {
			fOurConnectionsUseTLS = false;
			fWeServeSRTP = false;
			fWeEncryptSRTP = false;
		}
	}

	bool isRTSPS() { return fOurConnectionsUseTLS; }

	bool isSRTP() { return fWeServeSRTP; }

	bool isSRTPEncrypted() { return fWeEncryptSRTP; }

	void addUserRecord(const char *username, const char *password) {
		if (UserAuthenticationDatabase *auth = this->getAuthenticationDatabaseForCommand(nullptr); auth != nullptr) {
			auth->addUserRecord(username, password);
		}
	}

	void removeUserRecord(const char *username) {
		if (UserAuthenticationDatabase *auth = this->getAuthenticationDatabaseForCommand(nullptr); auth != nullptr) {
			auth->removeUserRecord(username);
		}
	}

	std::list<std::string> getUsers() {
		const auto *auth =
				dynamic_cast<MyUserAuthenticationDatabase *>(this->getAuthenticationDatabaseForCommand(nullptr));
		if (auth != nullptr) {
			return auth->getUsers();
		}
		return {};
	}

private:
	const unsigned int m_hlsSegment;
	std::string m_webroot;
};
