#include "stdafx.h"

#ifdef USE_XAUDIO2_AUDIOAPI

#include "XAudio2Api.h"
#include "SoundFileReader.h"
#include <xaudio2fx.h>

namespace Ingenuity {
namespace XAudio2 {

XAudio2::Item::~Item()
{
	xAudio->Stop(this);
	if(sourceVoice) sourceVoice->DestroyVoice();
	if(playBuffer) delete playBuffer;
}

IAsset * XAudio2::Item::GetAsset()
{
	return xAudio->CreateAudioItemFromWaveFormat(&wfx, (char*)playBuffer->pAudioData, playBuffer->AudioBytes);
}

XAudio2::Api::Api() : 
	xAudioEngine(0),
	xAudioMasteringVoice(0),
	globallyPaused(false)
{
	UINT32 flags = 0;

	HRESULT hr = XAudio2Create(&xAudioEngine, flags);

#if defined(_DEBUG) || defined(DEBUG)
	XAUDIO2_DEBUG_CONFIGURATION debugConfig = { 0 };
	debugConfig.BreakMask = XAUDIO2_LOG_ERRORS;
	debugConfig.TraceMask = XAUDIO2_LOG_ERRORS;
	xAudioEngine->SetDebugConfiguration(&debugConfig);
#endif

	xAudioEngine->CreateMasteringVoice(&xAudioMasteringVoice, XAUDIO2_DEFAULT_CHANNELS, 48000);

	XAUDIO2_VOICE_DETAILS masteringVoiceDetails;
	xAudioMasteringVoice->GetVoiceDetails(&masteringVoiceDetails);

	// Create volume meter effect
	IUnknown * volumeMeter;
	XAudio2CreateVolumeMeter(&volumeMeter);

	XAUDIO2_EFFECT_DESCRIPTOR effectDescriptor;
	effectDescriptor.pEffect = volumeMeter;
	effectDescriptor.InitialState = true;
	effectDescriptor.OutputChannels = masteringVoiceDetails.InputChannels;

	XAUDIO2_EFFECT_CHAIN effectChain;
	effectChain.EffectCount = 1;
	effectChain.pEffectDescriptors = &effectDescriptor;

	// Set the effect on the mastering voice
	xAudioMasteringVoice->SetEffectChain(&effectChain);

	volumeMeter->Release();
}

XAudio2::Api::~Api()
{
	if(xAudioMasteringVoice) xAudioMasteringVoice->DestroyVoice();
	if(xAudioEngine) xAudioEngine->Release();
}

void XAudio2::Api::PauseItem(XAudio2::Item * xItem, bool unpause)
{

}

Audio::Item * XAudio2::Api::CreateAudioItemFromWaveFormat(tWAVEFORMATEX * wfx, char * buffer, unsigned bufferLength)
{
	XAudio2::Item * newItem = new XAudio2::Item(this);

	newItem->isPlaying = false;

	newItem->playBuffer = new XAUDIO2_BUFFER();
	newItem->playBuffer->AudioBytes = bufferLength;  //size of the audio buffer in bytes
	newItem->playBuffer->pAudioData = (BYTE*)buffer;  //buffer containing audio data
	newItem->playBuffer->Flags = XAUDIO2_END_OF_STREAM; // tell the source voice not to expect any data after this buffer

	if(xAudioEngine && SUCCEEDED(xAudioEngine->CreateSourceVoice(&newItem->sourceVoice,
		wfx, 0, XAUDIO2_DEFAULT_FREQ_RATIO, 0, 0, 0)))
	{
		newItem->wfx = *wfx;

		unsigned bytesPerSample = (wfx->wBitsPerSample / 8) * wfx->nChannels;
		newItem->sampleRate = wfx->nSamplesPerSec;
		newItem->duration = float(bufferLength) / (float(bytesPerSample) * float(newItem->sampleRate));

		// Create volume meter effect
		IUnknown * volumeMeter;
		XAudio2CreateVolumeMeter(&volumeMeter);

		XAUDIO2_EFFECT_DESCRIPTOR effectDescriptor;
		effectDescriptor.pEffect = volumeMeter;
		effectDescriptor.InitialState = true;
		effectDescriptor.OutputChannels = 2;

		XAUDIO2_EFFECT_CHAIN effectChain;
		effectChain.EffectCount = 1;
		effectChain.pEffectDescriptors = &effectDescriptor;

		newItem->sourceVoice->SetEffectChain(&effectChain);

		return newItem;
	}
	else
	{
		delete newItem;
		return 0;
	}
}

void XAudio2::Api::Play(Audio::Item * item, float seek, bool loop)
{
	if(!item) return;

	XAudio2::Item *xItem = static_cast<XAudio2::Item *>(item);
	//
	// Setup buffer
	//
	//XAUDIO2_BUFFER playBuffer = { 0 };
	//playBuffer.AudioBytes = xItem->playBuffer->PlayLength;
	//playBuffer.pAudioData = xItem->playBuffer->pAudioData;
	//playBuffer.Flags = XAUDIO2_END_OF_STREAM;

	//
	// In case it is playing, stop it and flush the buffers.
	//
	HRESULT hr = xItem->sourceVoice->Stop();
	if(SUCCEEDED(hr))
	{
		hr = xItem->sourceVoice->FlushSourceBuffers();
	}

	//
	// Submit the sound buffer and (re)start (ignore any 'stop' failures)
	//
	unsigned seekSamples = unsigned(float(xItem->sampleRate) * float(seek));
	xItem->playBuffer->PlayBegin = seekSamples;

	hr = xItem->sourceVoice->SubmitSourceBuffer(xItem->playBuffer);
	if(SUCCEEDED(hr))
	{
		XAUDIO2_VOICE_STATE voiceState = { 0 };
		xItem->sourceVoice->GetState(&voiceState);

		xItem->samplesPlayedBeforeReset = int(voiceState.SamplesPlayed) - int(seekSamples);
		xItem->isPlaying = true;

		playingItems.push_back(xItem);

		if(!globallyPaused)
		{
			hr = xItem->sourceVoice->Start(0, XAUDIO2_COMMIT_NOW);
		}
	}
}

void XAudio2::Api::Pause(Audio::Item* item)
{
	// this is very limited: when unpausing, it'll unpause EVERYTHING!

	if(item)
	{
		XAudio2::Item * xItem = static_cast<XAudio2::Item*>(item);
		if(xItem->isPaused)
		{
			xItem->sourceVoice->Start();
			xItem->isPaused = false;
		}
		else
		{
			xItem->sourceVoice->Stop();
			xItem->isPaused = true;
		}
	}
	else
	{
		for(unsigned i = 0; i < playingItems.size(); ++i)
		{
			if(globallyPaused)
			{
				if(!playingItems[i]->isPaused)
				{
					playingItems[i]->sourceVoice->Start();
				}
			}
			else
			{
				if(!playingItems[i]->isPaused)
				{
					playingItems[i]->sourceVoice->Stop();
				}
			}
		}
		globallyPaused = !globallyPaused;
	}
}

void XAudio2::Api::Stop(Audio::Item* item)
{
	if(item)
	{
		XAudio2::Item * xItem = static_cast<XAudio2::Item*>(item);

		HRESULT hr = xItem->sourceVoice->Stop();
		if(SUCCEEDED(hr))
		{
			hr = xItem->sourceVoice->FlushSourceBuffers();

			for(unsigned i = 0; i < playingItems.size(); ++i)
			{
				if(playingItems[i] == xItem)
				{
					playingItems.erase(playingItems.begin() + i);
					break;
				}
			}
		}
	}
	else
	{
		for(unsigned i = 0; i < playingItems.size(); ++i)
		{
			HRESULT hr = playingItems[i]->sourceVoice->Stop();
			if(SUCCEEDED(hr))
			{
				hr = playingItems[i]->sourceVoice->FlushSourceBuffers();
			}
		}
		playingItems.clear();
	}
}

void XAudio2::Api::SetVolume(Audio::Item* item, float volume)
{
	if(!item) return;
	XAudio2::Item * xItem = static_cast<XAudio2::Item*>(item);

	xItem->sourceVoice->SetVolume(volume);
}

void XAudio2::Api::SetSpeed(Audio::Item* item, float speed)
{
	if(!item) return;
	XAudio2::Item * xItem = static_cast<XAudio2::Item*>(item);

	xItem->sourceVoice->SetFrequencyRatio(speed);

	xItem->playSpeed = speed;
}

float XAudio2::Api::GetAmplitude(Audio::Item * item)
{
	if(!xAudioEngine) return 0.0f;
	float peakLevels[2];
	float rmsLevels[2];
	XAUDIO2FX_VOLUMEMETER_LEVELS volumeMeter;
	volumeMeter.pPeakLevels = peakLevels;
	volumeMeter.pRMSLevels = rmsLevels;
	volumeMeter.ChannelCount = 2;

	if(item)
	{
		XAudio2::Item * xaudio2Item = static_cast<XAudio2::Item*>(item);
		xaudio2Item->sourceVoice->GetEffectParameters(0, &volumeMeter, sizeof(volumeMeter));
	}
	else
	{
		xAudioMasteringVoice->GetEffectParameters(0, &volumeMeter, sizeof(volumeMeter));
	}

	return (peakLevels[0] + peakLevels[1]) / 2;
}

float XAudio2::Api::GetDuration(Audio::Item * item)
{
	if(!item) return 0.0f;
	XAudio2::Item * xaudio2Item = static_cast<XAudio2::Item*>(item);

	return xaudio2Item->duration;
}

float XAudio2::Api::GetProgress(Audio::Item * item)
{
	if(!item) return 0.0f;
	XAudio2::Item * xaudio2Item = static_cast<XAudio2::Item*>(item);

	XAUDIO2_VOICE_STATE voiceState = { 0 };

	xaudio2Item->sourceVoice->GetState(&voiceState);

	unsigned samplesPlayed = unsigned(voiceState.SamplesPlayed);

	return float(samplesPlayed - xaudio2Item->samplesPlayedBeforeReset) / float(xaudio2Item->sampleRate);
}

} // namespace XAudio2
} // namespace Ingenuity

#endif