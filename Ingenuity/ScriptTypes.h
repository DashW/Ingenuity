#pragma once

#include "ScriptInterpreter.h"
#include <RealtimeApp.h>
#include <vector>

namespace Ingenuity {

struct LocalMesh;

enum ScriptPtrType
{
	TypeGpuComplexModel,
	TypeGpuEffect,
	TypeGpuTexture,
	TypeGpuCubeMap,
	TypeGpuVolumeTexture,
	TypeGpuCamera,
	TypeGpuFont,
	TypeGpuLight,
	TypeGpuDrawSurface,
	TypeGpuScene,
	TypeGpuShader,
	TypeGpuInstanceBuffer,
	TypeGpuParamBuffer,

	TypePlatformWindow,
	TypeFloatArray,
	TypeHeightParser,
	TypeImageBuffer,
	TypeAudioItem,
	TypeIsoSurface,
	TypeSVGParser,
	TypePhysicsWorld,
	TypePhysicsObject,
	TypePhysicsMaterial,
	TypePhysicsRagdoll,
	TypePhysicsSpring,
	TypeNetSocket,
	TypeLeapHelper,
	TypeSpoutSender,
	TypeSpoutReceiver,

	TypeCount
};

class ScriptTypes
{
	ScriptTypes();

public:
	static void RegisterWith(ScriptInterpreter * interpreter)
	{
		// Types
		for(unsigned i = 0; i < TypeCount; ++i)
		{
			typeHandles[i] = interpreter->RegisterPointerType();
		}
	}

	static inline bool CheckPtrType(ScriptParam & param, ScriptPtrType type)
	{ 
		return param.CheckPointer(typeHandles[type]); 
	}

	static inline unsigned GetHandle(ScriptPtrType type)
	{
		return typeHandles[type];
	}

private:
	static unsigned typeHandles[TypeCount];
};

} // namespace Ingenuity
