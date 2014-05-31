#pragma once

#include "AssetMgr.h"

struct tWAVEFORMATEX;

namespace Ingenuity {
namespace Audio {

struct Item : public IAsset
{
	virtual AssetType GetType() override { return AudioAsset; }
	virtual IAsset * GetAsset() override { return this; }
};

class Api
{

public:
	virtual ~Api() {};

	virtual Item * CreateAudioItemFromWaveFormat(tWAVEFORMATEX * wfx, char * buffer, unsigned dataLength) = 0;

	virtual void Play(Item * item, bool loop = false) = 0;
	virtual void Pause(Item * item) = 0;
	virtual void Stop(Item * item) = 0;
	virtual void SetVolume(Item * item, float volume) = 0;

	virtual float GetAmplitude(Item * item = 0) = 0;

};

} // namespace Audio
} // namespace Ingenuity
