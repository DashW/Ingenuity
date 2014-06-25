#pragma once

#include "AudioApi.h"

namespace Ingenuity {
namespace Audio {

class DummyApi : public Api
{

public:

	virtual Item * CreateAudioItemFromWaveFormat(tWAVEFORMATEX * wfx, char * buffer, unsigned dataLength) override { return 0; }

	virtual void Play(Item * item, bool loop = false) override {};
	virtual void Pause(Item * item) override {};
	virtual void Stop(Item * item = 0) override {};
	virtual void SetVolume(Item * item, float volume) override {};

	virtual float GetAmplitude(Item * item = 0) override { return 0.0f; }
	virtual float GetDuration(Item * item) override { return 0.0f; }
	virtual float GetProgress(Item * item) override { return 0.0f; }

};

} // namespace Audio
} // namespace Ingenuity
