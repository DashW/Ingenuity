// THIS CODE AND INFORMATION IS PROVIDED "AS IS" WITHOUT WARRANTY OF
// ANY KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING BUT NOT LIMITED TO
// THE IMPLIED WARRANTIES OF MERCHANTABILITY AND/OR FITNESS FOR A
// PARTICULAR PURPOSE.
//
// Copyright (c) Microsoft Corporation. All rights reserved

#include "stdafx.h"
#include <xaudio2.h>
#include "SoundFileReader.h"
//#include "RandomAccessReader.h"

#pragma optimize( "", off )

//
// 4 Character Tags in the RIFF File of Interest (read backwards)
//
//const UINT32 FOURCC_RIFF_TAG      = 'FFIR';
//const UINT32 FOURCC_FORMAT_TAG    = ' tmf';
//const UINT32 FOURCC_DATA_TAG      = 'atad';
//const UINT32 FOURCC_WAVE_FILE_TAG = 'EVAW';

#define fourccRIFF 'FFIR'
#define fourccDATA 'atad'
#define fourccFMT ' tmf'
#define fourccWAVE 'EVAW'
#define fourccXWMA 'AMWX'
#define fourccDPDS 'sdpd'

namespace Ingenuity {

//
// The header of every 'chunk' of data in the RIFF file
//
struct ChunkHeader
{
    UINT32 tag;
    UINT32 size;
};

HRESULT SoundFileReader::FindChunk(char * buffer, DWORD fourcc, DWORD & dwChunkSize, DWORD & dwChunkDataPosition)
{
	LARGE_INTEGER li = { 0 };

	HRESULT hr = S_OK;
	//if( INVALID_SET_FILE_POINTER == SetFilePointerEx( hFile, li, NULL, FILE_BEGIN ) )
	//	return HRESULT_FROM_WIN32( GetLastError() );

	DWORD dwChunkType;
	DWORD dwChunkDataSize;
	DWORD dwRIFFDataSize = 0;
	DWORD dwFileType;
	DWORD bytesRead = 0;
	DWORD dwOffset = 0;

	unsigned bufferOffset = 0;

	while(hr == S_OK && bufferOffset < bufferLength)
	{
		//DWORD dwRead;
		//if( 0 == ReadFile( hFile, &dwChunkType, sizeof(DWORD), &dwRead, NULL ) )
		//	hr = HRESULT_FROM_WIN32( GetLastError() );
		memcpy(&dwChunkType, &buffer[bufferOffset], sizeof(DWORD));
		bufferOffset += sizeof(DWORD);

		//if( 0 == ReadFile( hFile, &dwChunkDataSize, sizeof(DWORD), &dwRead, NULL ) )
		//	hr = HRESULT_FROM_WIN32(GetLastError());
		memcpy(&dwChunkDataSize, &buffer[bufferOffset], sizeof(DWORD));
		bufferOffset += sizeof(DWORD);

		switch(dwChunkType)
		{
		case fourccRIFF:
			dwRIFFDataSize = dwChunkDataSize;
			dwChunkDataSize = 4;
			//if( 0 == ReadFile( hFile, &dwFileType, sizeof(DWORD), &dwRead, NULL ) )
			//	hr = HRESULT_FROM_WIN32( GetLastError() );
			memcpy(&dwFileType, &buffer[bufferOffset], sizeof(DWORD));
			bufferOffset += sizeof(DWORD);
			break;

		default:
			li.QuadPart = dwChunkDataSize;
			//if( INVALID_SET_FILE_POINTER == SetFilePointerEx( hFile, li, NULL, FILE_CURRENT ) )
			bufferOffset += dwChunkDataSize;
			if(bufferOffset > bufferLength)
				return S_FALSE;
		}

		dwOffset += sizeof(DWORD) * 2;

		if(dwChunkType == fourcc)
		{
			dwChunkSize = dwChunkDataSize;
			dwChunkDataPosition = dwOffset;
			return S_OK;
		}

		dwOffset += dwChunkDataSize;

		if(bytesRead >= dwRIFFDataSize) return S_FALSE;

	}

	return S_FALSE;

}

HRESULT SoundFileReader::ReadChunkData(char * input, void * output, DWORD buffersize, DWORD bufferoffset)
{
	HRESULT hr = S_OK;
	LARGE_INTEGER li;
	li.QuadPart = bufferoffset;
	//if( INVALID_SET_FILE_POINTER == SetFilePointerEx( hFile, li, NULL, FILE_BEGIN ) )
	//	return HRESULT_FROM_WIN32( GetLastError() );
	//DWORD dwRead;
	//if( 0 == ReadFile( hFile, buffer, buffersize, &dwRead, NULL ) )
	//	hr = HRESULT_FROM_WIN32( GetLastError() );
	memcpy(output, &input[bufferoffset], buffersize);
	return hr;
}

//--------------------------------------------------------------------------------------
// Name: SoundFileReader constructor
// Desc: Any failure to construct will throw.
//       If the constructor succeeds this is a fully usable object
//--------------------------------------------------------------------------------------
SoundFileReader::SoundFileReader(Audio::Api * audio, Files::Api * files, Files::Directory * directory, const wchar_t * path) :
SimpleLoader(files, directory, path, AudioAsset), audio(audio), soundData(0), soundDataLength(0), soundFormat(0)
{ }

SoundFileReader::~SoundFileReader()
{
	if(soundData) delete[] soundData;
	if(soundFormat) delete soundFormat;
}

void SoundFileReader::Respond()
{
	if(buffer)
	{
		WAVEFORMATEXTENSIBLE wfx = { 0 };

		//
		// Open the file for random read access
		//
		//RandomAccessReader^ riffFile = ref new RandomAccessReader(fileName);
		//uint64 fileSize = riffFile->GetFileSize();
		//
		//#if !defined(WINAPI_FAMILY) || WINAPI_FAMILY != WINAPI_FAMILY_APP
		//
		//	HANDLE hFile = CreateFile(
		//		path,
		//		GENERIC_READ,
		//		FILE_SHARE_READ,
		//		NULL,
		//		OPEN_EXISTING,
		//		0,
		//		NULL );
		//
		//#else // WINAPI_FAMILY == WINAPI_FAMILY_APP
		//
		//	HANDLE hFile = CreateFile2(
		//		path,
		//		GENERIC_READ,
		//		FILE_SHARE_READ,
		//		OPEN_EXISTING,
		//		NULL );
		//
		//#endif

		//if( INVALID_HANDLE_VALUE == hFile )
		//	return HRESULT_FROM_WIN32( GetLastError() );

		//if( INVALID_SET_FILE_POINTER == SetFilePointer( hFile, 0, NULL, FILE_BEGIN ) )
		//	return HRESULT_FROM_WIN32( GetLastError() );

		//
		// Locate, read and validate the RIFF chunk
		//
		// Read beyond the riff header.
		//ChunkHeader chunkHeader;
		//ReadHeader(riffFile, riffLoc, chunkHeader);

		//uint32 tag = 0;
		//Platform::Array<byte>^ riffData = riffFile->Read(sizeof(tag));
		//tag = *reinterpret_cast<uint32*>(riffData->Data);
		//if (tag != FOURCC_WAVE_FILE_TAG)
		//{
		//    // Only support .wav files
		//    throw ref new Platform::FailureException();
		//}
		//uint64 riffChildrenStart = riffLoc + sizeof(chunkHeader) + sizeof(tag);
		//uint64 riffChildrenEnd   = riffLoc + sizeof(chunkHeader) + chunkHeader.size;

		DWORD dwChunkSize;
		DWORD dwChunkPosition;
		//check the file type, should be fourccWAVE or 'XWMA'
		FindChunk(buffer, fourccRIFF, dwChunkSize, dwChunkPosition);
		DWORD filetype;
		ReadChunkData(buffer, &filetype, sizeof(DWORD), dwChunkPosition);
		//if (filetype != fourccWAVE)
		//	return S_FALSE;

		//
		// Find, read and validate the format chunk (a child within the RIFF chunk)
		//
		//uint64 formatLoc = FindChunk(riffFile, FOURCC_FORMAT_TAG, riffChildrenStart, riffChildrenEnd);
		//ReadHeader(riffFile, formatLoc, chunkHeader);
		//if (chunkHeader.size < sizeof(WAVEFORMATEX))
		//{
		//    // Format data of unsupported size; must be unsupported format
		//    throw ref new Platform::FailureException();
		//}
		//m_soundFormat = riffFile->Read(chunkHeader.size);
		//WAVEFORMATEX format = *reinterpret_cast<WAVEFORMATEX*>(m_soundFormat->Data);
		//if (format.wFormatTag != WAVE_FORMAT_PCM
		//    && format.wFormatTag != WAVE_FORMAT_ADPCM)
		//{
		//    // This is not PCM or ADPCM, which is all we support
		//    throw ref new Platform::FailureException();
		//}

		if(!SUCCEEDED(FindChunk(buffer, fourccFMT, dwChunkSize, dwChunkPosition)))
		{
			return;
		}
		ReadChunkData(buffer, &wfx, dwChunkSize, dwChunkPosition);

		//
		// Locate, the PCM data in the data chunk
		//
		//uint64 dataChunkStart = FindChunk(riffFile, FOURCC_DATA_TAG, riffChildrenStart, riffChildrenEnd);
		//ReadHeader(riffFile, dataChunkStart, chunkHeader);

		//
		// Now read the PCM data and setup the buffer
		//
		//m_soundData = riffFile->Read(chunkHeader.size);

		// WE NEED TO REMOVE THIS STEP FROM THE INITIAL LOAD, 
		// ADD THE METADATA TO THE AUDIOITEM, GIVE THE AUDIOAPI
		// ACCESS TO THE FILEAPI, AND LEAVE THE FILE OPEN UNTIL
		// THE AUDIOAPI HAS DECIDED WHAT TO DO WITH IT.

		if(!SUCCEEDED(FindChunk(buffer, fourccDATA, dwChunkSize, dwChunkPosition)))
		{
			return;
		}
		BYTE * pDataBuffer = new BYTE[dwChunkSize];
		ReadChunkData(buffer, pDataBuffer, dwChunkSize, dwChunkPosition);

		soundFormat = new WAVEFORMATEX(wfx.Format);

		soundDataLength = dwChunkSize;
		soundData = (char*)pDataBuffer;

		//CloseHandle(hFile);

		//if(xAudioEngine && SUCCEEDED(xAudioEngine->CreateSourceVoice(&newItem->sourceVoice,
		//	nextSound.GetSoundFormat(), 0, XAUDIO2_DEFAULT_FREQ_RATIO, 0, 0, 0))) 
		//{
		//}

		asset = audio->CreateAudioItemFromWaveFormat(soundFormat, soundData, soundDataLength);
	}
	else
	{
		// Could not load audio file, break!
	}
}

//--------------------------------------------------------------------------------------
// Name: SoundFileReader::GetSoundFormat
// Desc: Sound Format Accessor
//--------------------------------------------------------------------------------------
WAVEFORMATEX* SoundFileReader::GetSoundFormat() const
{
	return soundFormat;
}

} // namespace Ingenuity
