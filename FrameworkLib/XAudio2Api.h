#pragma once

#include "AudioApi.h"
#include <xaudio2.h>
//#include "XAudio2SoundPlayer.h"

#pragma comment(lib,"xaudio2.lib")

namespace Ingenuity {
namespace XAudio2 {

struct Item : public Audio::Item
{
	IXAudio2SourceVoice*    sourceVoice;
	XAUDIO2_BUFFER*			playBuffer;
	bool                    isPlaying;

	Item() :
		sourceVoice(0),
		playBuffer(0),
		isPlaying(false) {}
	virtual ~Item();
};

class Api : public Audio::Api
{
	IXAudio2 *xAudioEngine;
	IXAudio2MasteringVoice *xAudioMasteringVoice;

	//bool GetSample(UINT8* buffer, UINT32 maxbufferlength, UINT32* bufferlength);

	//XAudio2SoundPlayer *soundPlayer;

public:
	Api();
	virtual ~Api();

	//virtual bool IsPlaying(AudioItem *item);
	virtual Audio::Item * CreateAudioItemFromWaveFormat(tWAVEFORMATEX * wfx, char * buffer, unsigned bufferLength);

	virtual void Play(Audio::Item * item, bool loop = false) override;
	virtual void Pause(Audio::Item * item) override;
	virtual void Stop(Audio::Item * item) override;
	virtual void SetVolume(Audio::Item * item, float volume) override;

	virtual float GetAmplitude(Audio::Item * item = 0) override;

};

} // namespace XAudio2
} // namespace Ingenuity
