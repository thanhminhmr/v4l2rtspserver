/* ---------------------------------------------------------------------------
** This software is in the public domain, furnished "as is", without technical
** support, and with no warranty, express or implied, as to its usefulness for
** any purpose.
**
** V4l2RTSPServer.cpp
**
** V4L2 RTSP server
**
** -------------------------------------------------------------------------*/

#include <dirent.h>

#include <StreamReplicator.hh>
#include <sstream>

#include "DeviceSourceFactory.h"
#include "V4l2Capture.h"
#include "V4l2Output.h"
#include "V4l2RTSPServer.h"
#include "VideoCaptureAccess.h"
#include "logger.h"

#ifdef HAVE_ALSA
#include "ALSACapture.h"
#endif

StreamReplicator *V4l2RTSPServer::CreateVideoReplicator(
		const V4L2DeviceParameters &inParam, const int queueSize, const V4L2DeviceSource::CaptureMode captureMode,
		const int repeatConfig, const std::string &outputFile, const V4l2IoType ioTypeOut, V4l2Output *&out
) const {

	StreamReplicator *videoReplicator = nullptr;
	if (const std::string videoDev(inParam.m_devName); !videoDev.empty()) {
		// Init video capture
		LOG(NOTICE) << "Create V4L2 Source..." << videoDev;

		if (V4l2Capture *videoCapture = V4l2Capture::create(inParam)) {
			int outfd = -1;

			if (!outputFile.empty()) {
				V4L2DeviceParameters outparam(
						outputFile.c_str(), videoCapture->getFormat(), videoCapture->getWidth(),
						videoCapture->getHeight(), 0, ioTypeOut
				);
				out = V4l2Output::create(outparam);
				if (out != nullptr) {
					outfd = out->getFd();
					LOG(INFO) << "Output fd:" << outfd << " " << outputFile;
				} else {
					LOG(WARN) << "Cannot open output:" << outputFile;
				}
			}

			if (const std::string rtpVideoFormat(
						BaseServerMediaSubsession::getVideoRtpFormat(videoCapture->getFormat())
				);
				rtpVideoFormat.empty()) {
				LOG(FATAL) << "No Streaming format supported for device " << videoDev;
				delete videoCapture;
			} else {
				videoReplicator = DeviceSourceFactory::createStreamReplicator(
						this->env(), videoCapture->getFormat(), new VideoCaptureAccess(videoCapture), queueSize,
						captureMode, outfd, repeatConfig
				);
				if (videoReplicator == nullptr) {
					LOG(FATAL) << "Unable to create source for device " << videoDev;
					delete videoCapture;
				}
			}
		}
	}
	return videoReplicator;
}

std::string getVideoDeviceName(const std::string &devicePath) {
	std::string deviceName(devicePath);
	if (const size_t pos = deviceName.find_last_of('/'); pos != std::string::npos) {
		deviceName.erase(0, pos + 1);
	}
	return deviceName;
}

#ifdef HAVE_ALSA
/* ---------------------------------------------------------------------------
**  get a "deviceid" from uevent sys file
** -------------------------------------------------------------------------*/
std::string getDeviceId(const std::string &evt) {
	std::string deviceId;
	std::istringstream f(evt);
	std::string key;
	while (getline(f, key, '=')) {
		if (std::string value; getline(f, value)) {
			if (key == "PRODUCT" || key == "PCI_SUBSYS_ID") {
				deviceId = value;
				break;
			}
		}
	}
	return deviceId;
}

std::string V4l2RTSPServer::getV4l2Alsa(const std::string &v4l2device) {
	std::string audioDevice(v4l2device);

	std::map<std::string, std::string> videoDevices;
	std::string video4linuxPath("/sys/class/video4linux");
	if (DIR *dp = opendir(video4linuxPath.c_str()); dp != nullptr) {
		dirent *entry = nullptr;
		while ((entry = readdir(dp))) {
			std::string deviceName;
			std::string deviceId;
			if (strstr(entry->d_name, "video") == entry->d_name) {
				std::string ueventPath(video4linuxPath);
				ueventPath.append("/").append(entry->d_name).append("/device/uevent");
				std::ifstream ifsd(ueventPath.c_str());
				deviceId = std::string(std::istreambuf_iterator{ifsd}, {});
				deviceId.erase(deviceId.find_last_not_of('\n') + 1);
			}

			if (!deviceId.empty()) {
				videoDevices[entry->d_name] = getDeviceId(deviceId);
			}
		}
		closedir(dp);
	}

	std::map<std::string, std::string> audioDevices;
	int rcard = -1;
	while (snd_card_next(&rcard) == 0 && rcard >= 0) {
		void **hints = nullptr;
		if (snd_device_name_hint(rcard, "pcm", &hints) >= 0) {
			void **str = hints;
			while (*str) {
				std::ostringstream os;
				os << "/sys/class/sound/card" << rcard << "/device/uevent";

				std::ifstream ifs(os.str().c_str());
				auto deviceId = std::string(std::istreambuf_iterator{ifs}, {});
				deviceId.erase(deviceId.find_last_not_of('\n') + 1);
				deviceId = getDeviceId(deviceId);

				if (!deviceId.empty()) {
					if (!audioDevices.contains(deviceId)) {
						std::string audioName = snd_device_name_get_hint(*str, "NAME");
						audioDevices[deviceId] = audioName;
					}
				}

				str++;
			}

			snd_device_name_free_hint(hints);
		}
	}

	if (auto deviceId = videoDevices.find(getVideoDeviceName(v4l2device)); deviceId != videoDevices.end()) {
		if (auto audioDeviceIt = audioDevices.find(deviceId->second); audioDeviceIt != audioDevices.end()) {
			audioDevice = audioDeviceIt->second;
			std::cout << v4l2device << "=>" << audioDevice << std::endl;
		}
	}

	return audioDevice;
}

snd_pcm_format_t V4l2RTSPServer::decodeAudioFormat(const std::string &fmt) {
	snd_pcm_format_t audioFmt = SND_PCM_FORMAT_UNKNOWN;
	if (fmt == "S16_BE") {
		audioFmt = SND_PCM_FORMAT_S16_BE;
	} else if (fmt == "S16_LE") {
		audioFmt = SND_PCM_FORMAT_S16_LE;
	} else if (fmt == "S24_BE") {
		audioFmt = SND_PCM_FORMAT_S24_BE;
	} else if (fmt == "S24_LE") {
		audioFmt = SND_PCM_FORMAT_S24_LE;
	} else if (fmt == "S32_BE") {
		audioFmt = SND_PCM_FORMAT_S32_BE;
	} else if (fmt == "S32_LE") {
		audioFmt = SND_PCM_FORMAT_S32_LE;
	} else if (fmt == "ALAW") {
		audioFmt = SND_PCM_FORMAT_A_LAW;
	} else if (fmt == "MULAW") {
		audioFmt = SND_PCM_FORMAT_MU_LAW;
	} else if (fmt == "S8") {
		audioFmt = SND_PCM_FORMAT_S8;
	} else if (fmt == "MPEG") {
		audioFmt = SND_PCM_FORMAT_MPEG;
	}
	return audioFmt;
}

StreamReplicator *V4l2RTSPServer::CreateAudioReplicator(
		const std::string &audioDev, const std::list<snd_pcm_format_t> &audioFmtList, const int audioFreq,
		const int audioNbChannels, int verbose, const int queueSize, const V4L2DeviceSource::CaptureMode captureMode
) const {
	StreamReplicator *audioReplicator = nullptr;
	if (!audioDev.empty()) {
		// find the ALSA device associated with the V4L2 device
		const std::string audioDevice = getV4l2Alsa(audioDev);

		// Init audio capture
		LOG(NOTICE) << "Create ALSA Source..." << audioDevice;

		const ALSACaptureParameters param(audioDevice.c_str(), audioFmtList, audioFreq, audioNbChannels);
		if (ALSACapture *audioCapture = ALSACapture::createNew(param)) {
			audioReplicator =
					DeviceSourceFactory::createStreamReplicator(this->env(), 0, audioCapture, queueSize, captureMode);
			if (audioReplicator == nullptr) {
				LOG(FATAL) << "Unable to create source for device " << audioDevice;
				delete audioCapture;
			}
		}
	}
	return audioReplicator;
}
#endif
