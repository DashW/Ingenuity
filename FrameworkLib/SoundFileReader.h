// THIS CODE AND INFORMATION IS PROVIDED "AS IS" WITHOUT WARRANTY OF
// ANY KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING BUT NOT LIMITED TO
// THE IMPLIED WARRANTIES OF MERCHANTABILITY AND/OR FITNESS FOR A
// PARTICULAR PURPOSE.
//
// Copyright (c) Microsoft Corporation. All rights reserved

#pragma once

#include "AssetMgr.h"
#include "AudioApi.h"

struct tWAVEFORMATEX;

namespace Ingenuity {

class SoundFileReader : public SimpleLoader
{
public:
	SoundFileReader(Audio::Api * audio, Files::Api * files, Files::Directory * d, const wchar_t * p);
	virtual ~SoundFileReader();

	virtual void Respond() override;

	char * GetSoundData() const { return soundData; }
	unsigned GetSoundDataLength() const { return soundDataLength; }
	tWAVEFORMATEX* GetSoundFormat() const;

private:
	HRESULT FindChunk(char * buffer, DWORD fourcc, DWORD & dwChunkSize, DWORD & dwChunkDataPosition);
	HRESULT ReadChunkData(char * input, void * output, DWORD buffersize, DWORD bufferoffset);

	Audio::Api * audio;
	//XAUDIO2_BUFFER    *soundData;
	char * soundData;
	unsigned soundDataLength;
	tWAVEFORMATEX    *soundFormat;
};

} // namespace Ingenuity
