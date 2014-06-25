#pragma once

#include "AudioApi.h"
#include <xaudio2.h>
//#include <Audio.h>
//#include "XAudio2SoundPlayer.h"

#pragma comment(lib,"xaudio2.lib")

namespace Ingenuity {
namespace XAudio2 {

class Api;

struct Item : public Audio::Item
{
	Api*                    xAudio;
	IXAudio2SourceVoice*    sourceVoice;
	XAUDIO2_BUFFER*         playBuffer;
	float                   duration;
	unsigned                sampleRate;
	int						samplesPlayedBeforeReset;
	bool                    isPlaying;
	bool					isPaused;

	Item(Api * api) :
		xAudio(api),
		sourceVoice(0),
		playBuffer(0),
		sampleRate(0),
		samplesPlayedBeforeReset(0),
		isPlaying(false),
		isPaused(false) {}
	virtual ~Item();
};

class Api : public Audio::Api
{
	IXAudio2 * xAudioEngine;
	IXAudio2MasteringVoice * xAudioMasteringVoice;

	bool globallyPaused;

	//bool GetSample(UINT8* buffer, UINT32 maxbufferlength, UINT32* bufferlength);

	//XAudio2SoundPlayer *soundPlayer;
	void PauseItem(Item * xItem, bool unpause);

public:
	std::vector<Item*> playingItems;

	Api();
	virtual ~Api();

	//virtual bool IsPlaying(AudioItem *item);
	virtual Audio::Item * CreateAudioItemFromWaveFormat(tWAVEFORMATEX * wfx, char * buffer, unsigned bufferLength);

	virtual void Play(Audio::Item * item, float seek = 0.0f, bool loop = false) override;
	virtual void Pause(Audio::Item * item = 0) override;
	virtual void Stop(Audio::Item * item = 0) override;
	virtual void SetVolume(Audio::Item * item, float volume) override;

	virtual float GetAmplitude(Audio::Item * item = 0) override;
	virtual float GetDuration(Audio::Item * item) override;
	virtual float GetProgress(Audio::Item * item) override;

	void OnWindowClosed();
};

} // namespace XAudio2
} // namespace Ingenuity
