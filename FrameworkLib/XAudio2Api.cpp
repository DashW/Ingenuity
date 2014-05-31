#include "stdafx.h"

#ifdef USE_XAUDIO2_AUDIOAPI

#include "XAudio2Api.h"
#include "SoundFileReader.h"
#include <xaudio2fx.h>

namespace Ingenuity {
namespace XAudio2 {

XAudio2::Item::~Item()
{
	if(playBuffer) delete playBuffer;
	if(sourceVoice) sourceVoice->DestroyVoice();
}

XAudio2::Api::Api()
	: xAudioEngine(0), xAudioMasteringVoice(0)
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

	// Set the effect on the mastering voice
	xAudioMasteringVoice->SetEffectChain(&effectChain);

	volumeMeter->Release();
}

XAudio2::Api::~Api()
{
	if(xAudioMasteringVoice) xAudioMasteringVoice->DestroyVoice();
	if(xAudioEngine) xAudioEngine->Release();
}

Audio::Item * XAudio2::Api::CreateAudioItemFromWaveFormat(tWAVEFORMATEX * wfx, char * buffer, unsigned bufferLength)
{
	XAudio2::Item * newItem = new XAudio2::Item();

	newItem->isPlaying = false;

	newItem->playBuffer = new XAUDIO2_BUFFER();
	newItem->playBuffer->AudioBytes = bufferLength;  //size of the audio buffer in bytes
	newItem->playBuffer->pAudioData = (BYTE*)buffer;  //buffer containing audio data
	newItem->playBuffer->Flags = XAUDIO2_END_OF_STREAM; // tell the source voice not to expect any data after this buffer

	if(xAudioEngine && SUCCEEDED(xAudioEngine->CreateSourceVoice(&newItem->sourceVoice,
		wfx, 0, XAUDIO2_DEFAULT_FREQ_RATIO, 0, 0, 0)))
	{
		return newItem;
	}
	else
	{
		delete newItem->playBuffer;
		delete newItem;
		return 0;
	}
}

void XAudio2::Api::Play(Audio::Item* item, bool loop)
{
	if(!item || !xAudioEngine) return;

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
	hr = xItem->sourceVoice->SubmitSourceBuffer(xItem->playBuffer);
	if(SUCCEEDED(hr))
	{
		hr = xItem->sourceVoice->Start(0, XAUDIO2_COMMIT_NOW);
		if(SUCCEEDED(hr)) xItem->isPlaying = true;
	}
}

void XAudio2::Api::Pause(Audio::Item* item)
{

}

void XAudio2::Api::Stop(Audio::Item* item)
{
	XAudio2::Item * xItem = static_cast<XAudio2::Item*>(item);

	HRESULT hr = xItem->sourceVoice->Stop();
	if(SUCCEEDED(hr))
	{
		hr = xItem->sourceVoice->FlushSourceBuffers();
	}
}

void XAudio2::Api::SetVolume(Audio::Item* item, float volume)
{
	XAudio2::Item * xItem = static_cast<XAudio2::Item*>(item);

	xItem->sourceVoice->SetVolume(volume);
}

float XAudio2::Api::GetAmplitude(Audio::Item * item)
{
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

} // namespace XAudio2
} // namespace Ingenuity

#endif