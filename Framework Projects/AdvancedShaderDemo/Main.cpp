// Compute & Geometry Shader demo - Richard Copperwaite

#include "pch.h"
#include <RealtimeApp.h>
#include <PlatformApi.h>
#include <GeoBuilder.h>
#include <WavefrontLoader.h>
#include <InputState.h>
#include <IsoSurface.h>
#define _X86_
#define _USE_MATH_DEFINES
#include <math.h>

#include <cstring>
#include <sstream>

using namespace Ingenuity;

const float fullCircle = (float)(M_PI * 2);

class AdvancedShaderDemo : public RealtimeApp
{
	Gpu::Font * font;
	Gpu::Model * skybox;
	Gpu::DirectionalLight * light;
	Gpu::Camera * camera;
	Gpu::CubeMap * cubeMap;
	Gpu::Effect * particleInsertEffect = 0;
	Gpu::Effect * particleUpdateEffect = 0;
	Gpu::Effect * particleRenderEffect = 0;
	Gpu::Effect * indirectTriangleEffect = 0;
	Gpu::Texture * particleTex = 0;
	Gpu::ParamBuffer * triParamBuf = 0;

	float modelScale, modelPos;
	float cameraRadius, cameraAngle;
	float timePassed = 0.0f;
	float timeSinceParticleInsert = 0.0f;

	int cubemapTicket;

	wchar_t drawCallText[100];

	// BASIC COMPUTE STRUCTURES

	struct BufType
	{
		int i;
		float f;
	};

	static const unsigned NUM_STRUCTS = 1024;

	BufType vBuf0[NUM_STRUCTS];
	BufType vBuf1[NUM_STRUCTS];
	BufType results[NUM_STRUCTS];

	Gpu::ParamBuffer * paramBuf0 = 0;
	Gpu::ParamBuffer * paramBuf1 = 0;
	Gpu::ParamBuffer * resultBuf = 0;

	// PARTICLE SIMULATION STRUCTURES

	struct Particle
	{
		glm::vec3 position;
		glm::vec3 direction;
		float time;
	};

	static const unsigned NUM_PARTICLES = 512 * 512;

	Particle pData[262144];

	Gpu::ParamBuffer * particleBuf0 = 0;
	Gpu::ParamBuffer * particleBuf1 = 0;

	static const unsigned NUM_DEBUG_VALS = 8;

	unsigned debugData[NUM_DEBUG_VALS];

	Gpu::ParamBuffer * debugBuf = 0;

	// The throttle time is calculated as the number of particles per insertion,
	// times the maximum particle lifetime and divided by the total buffer size.

	const float particleInsertInterval = 8.0f * 30.0f / static_cast<float>(NUM_PARTICLES);

public:
	virtual void Begin() override
	{
		font = 0; skybox = 0; light = 0; camera = 0; 
		cubeMap = 0; cubemapTicket = -1; 

		font = gpu->CreateGpuFont(40,L"Arial");

		modelScale = 8.0f; modelPos = 7.0f;

		skybox = new Gpu::Model();
		skybox->mesh = GeoBuilder().BuildSkyCube()->GpuOnly(gpu);
		skybox->scale = glm::vec4(20.0f);
		skybox->backFaceCull = false;

		Files::Directory * appDir = files->GetKnownDirectory(Files::AppDir);
		Files::Directory * frameworkDir = files->GetKnownDirectory(Files::FrameworkDir);
		AssetBatch batch;
		batch.emplace_back(appDir, L"AdvancedShaderDemo\\skybox_clearsky.dds", CubeMapAsset, "skybox");
		batch.emplace_back(appDir, L"AdvancedShaderDemo\\Particle.png", TextureAsset, "particleTex");
		batch.emplace_back(frameworkDir, L"SkyShader.xml", ShaderAsset, "skyshader");
		batch.emplace_back(frameworkDir, L"BasicCompute.xml", ShaderAsset, "basicCompute");
		batch.emplace_back(frameworkDir, L"ParticleSystemInsert.xml", ShaderAsset, "particleInsert");
		batch.emplace_back(frameworkDir, L"ParticleSystemUpdate.xml", ShaderAsset, "particleUpdate");
		batch.emplace_back(frameworkDir, L"ParticleSystemRender.xml", ShaderAsset, "particleRender");
		batch.emplace_back(frameworkDir, L"IndirectTriShader.xml", ShaderAsset, "indirectTri");
		cubemapTicket = assets->Load(batch);

		camera = new Gpu::Camera();
		cameraRadius = 20.0f;
		camera->position.y = 2.0f;
		cameraAngle = 0.0f;
		camera->fovOrHeight = (float)(M_PI_4);
		camera->nearClip = 1.f;
		camera->farClip = 5000.f;
		camera->position.x = 0.0f;
		camera->position.z = cameraRadius;

		light = new Gpu::DirectionalLight();
		light->direction = glm::vec3(sinf(-0.5f),0.25f,cosf(-0.5f));

		for(unsigned i = 0; i < NUM_STRUCTS; ++i)
		{
			vBuf0[i].i = i;
			vBuf0[i].f = (float)i;

			vBuf1[i].i = i;
			vBuf1[i].f = (float)i;
		}

		paramBuf0 = gpu->CreateParamBuffer(NUM_STRUCTS, vBuf0, sizeof(BufType));
		paramBuf1 = gpu->CreateParamBuffer(NUM_STRUCTS, vBuf1, sizeof(BufType));
		resultBuf = gpu->CreateParamBuffer(NUM_STRUCTS, 0, sizeof(BufType), -1);

		for(unsigned i = 0; i < NUM_PARTICLES; ++i)
		{
			pData[i].position = glm::vec3(0.0f);
			pData[i].direction = glm::vec3(0.0f, 0.0f, 1.0f);
			pData[i].time = 0.0f;
		}

		particleBuf0 = gpu->CreateParamBuffer(NUM_PARTICLES, pData, sizeof(Particle), 0);
		particleBuf1 = gpu->CreateParamBuffer(NUM_PARTICLES, pData, sizeof(Particle), 0);

		std::memset(debugData, 0, NUM_DEBUG_VALS * sizeof(unsigned));
		debugBuf = gpu->CreateParamBuffer(NUM_DEBUG_VALS * sizeof(unsigned), debugData, 0, -1);

		std::wstringstream debugOutput;
		for(unsigned i = 0; i < NUM_DEBUG_VALS; ++i) debugOutput << debugData[i] << ", ";
		debugOutput << std::endl;
		OutputDebugString(debugOutput.str().c_str());
	}

	virtual void End() override
	{
		if(paramBuf0) delete paramBuf0;
		if(paramBuf1) delete paramBuf1;
		if(resultBuf) delete resultBuf;
		if(particleBuf0) delete particleBuf0;
		if(particleBuf1) delete particleBuf1;
		if(debugBuf) delete debugBuf;
		if(triParamBuf) delete triParamBuf;

		if(particleInsertEffect) delete particleInsertEffect;
		if(particleUpdateEffect) delete particleUpdateEffect;
		if(particleRenderEffect) delete particleRenderEffect;
		if(indirectTriangleEffect) delete indirectTriangleEffect;

		if(skybox->effect)
		{
			delete skybox->effect;
		}
		delete skybox->mesh;
		delete skybox;
		delete light;
		delete camera;
		delete font;
	}

	virtual void Update(float secs) override
	{
		KeyState & keys = input->GetKeyState();

		if(assets->IsLoaded(cubemapTicket))
		{
			cubeMap = assets->GetAsset<Gpu::CubeMap>("skybox");
			//model->cubeMap = cubeMap;

			particleTex = assets->GetAsset<Gpu::Texture>("particleTex");

			Gpu::Shader * skyShader = assets->GetAsset<Gpu::Shader>("skyshader");
			if(skyShader)
			{
				skybox->effect = new Gpu::Effect(skyShader);
				skybox->cubeMap = cubeMap;
			}

			cubemapTicket = -1;

			Gpu::Shader * computeShader = assets->GetAsset<Gpu::Shader>("basicCompute");
			if(computeShader)
			{
				Gpu::Effect computeEffect(computeShader);

				computeEffect.SetParam(0, paramBuf0);
				computeEffect.SetParam(1, paramBuf1);
				computeEffect.SetParam(2, resultBuf);

				gpu->Compute(&computeEffect, NUM_STRUCTS);

				gpu->GetParamBufferData(resultBuf, results, 0, NUM_STRUCTS * sizeof(BufType));

				for(unsigned i = 0; i < NUM_STRUCTS; ++i)
				{
					if(results[i].i != vBuf0[i].i + vBuf1[i].i)
					{
						__debugbreak();
					}
				}
			}

			Gpu::Shader * insertShader = assets->GetAsset<Gpu::Shader>("particleInsert");
			if(insertShader)
			{
				particleInsertEffect = new Gpu::Effect(insertShader);
			}

			Gpu::Shader * updateShader = assets->GetAsset<Gpu::Shader>("particleUpdate");
			if(updateShader)
			{
				particleUpdateEffect = new Gpu::Effect(updateShader);
			}

			Gpu::Shader * renderShader = assets->GetAsset<Gpu::Shader>("particleRender");
			if(renderShader)
			{
				particleRenderEffect = new Gpu::Effect(renderShader);
			}

			Gpu::Shader * indirectTriShader = assets->GetAsset<Gpu::Shader>("indirectTri");
			if(indirectTriShader)
			{
				indirectTriangleEffect = new Gpu::Effect(indirectTriShader);
				triParamBuf = gpu->CreateParamBuffer(1000, 0, 4);
			}
		}

		cameraAngle = cameraAngle + (secs / 2.0f);
		if(cameraAngle > fullCircle) cameraAngle -= fullCircle;

		timeSinceParticleInsert += secs;

		if(particleInsertEffect && timeSinceParticleInsert > particleInsertInterval)
		{
			particleInsertEffect->SetParam(0, particleBuf0);

			static const float scale = 2.0f;
			float fRandomX = ((float)rand() / (float)RAND_MAX * scale - scale / 2.0f);
			float fRandomY = ((float)rand() / (float)RAND_MAX * scale - scale / 2.0f);
			float fRandomZ = ((float)rand() / (float)RAND_MAX * scale - scale / 2.0f);

			glm::vec3 randomVec = glm::vec3(fRandomX, fRandomY, fRandomZ);
			randomVec = glm::normalize(randomVec);

			particleInsertEffect->SetParam(1, 0.0f);
			particleInsertEffect->SetParam(2, 0.0f);
			particleInsertEffect->SetParam(3, 0.0f);

			particleInsertEffect->SetParam(4, randomVec.x);
			particleInsertEffect->SetParam(5, randomVec.y);
			particleInsertEffect->SetParam(6, randomVec.z);

				gpu->CopyParamBufferSize(particleBuf0, debugBuf, 0 * sizeof(unsigned));

			gpu->Compute(particleInsertEffect, 1);

				gpu->CopyParamBufferSize(particleBuf0, debugBuf, 1 * sizeof(unsigned));

				gpu->GetParamBufferData(debugBuf, debugData, 0, NUM_DEBUG_VALS * sizeof(unsigned));

				std::wstringstream debugOutput;
				for(unsigned i = 0; i < NUM_DEBUG_VALS; ++i) debugOutput << debugData[i] << ", ";
				debugOutput << std::endl;
				OutputDebugString(debugOutput.str().c_str());

			timeSinceParticleInsert = 0.0f;
		}

		if(particleUpdateEffect)
		{
			particleUpdateEffect->SetParam(0, particleBuf1);
			particleUpdateEffect->SetParam(1, particleBuf0);

			particleUpdateEffect->SetParam(2, secs);

			particleUpdateEffect->SetParam(3, 50.0f);
			particleUpdateEffect->SetParam(4, 0.0f);
			particleUpdateEffect->SetParam(5, 0.0f);

				gpu->CopyParamBufferSize(particleBuf0, debugBuf, 0);
				gpu->CopyParamBufferSize(particleBuf1, debugBuf, 1 * sizeof(unsigned));

				gpu->GetParamBufferData(debugBuf, debugData, 0, NUM_DEBUG_VALS * sizeof(unsigned));

				std::wstringstream debugOutput;
				for(unsigned i = 0; i < NUM_DEBUG_VALS; ++i) debugOutput << debugData[i] << ", ";
				debugOutput << std::endl;
				OutputDebugString(debugOutput.str().c_str());

			gpu->Compute(particleUpdateEffect, NUM_PARTICLES / 512);

				gpu->CopyParamBufferSize(particleBuf0, debugBuf, 0);
				gpu->CopyParamBufferSize(particleBuf1, debugBuf, 1 * sizeof(unsigned));

				gpu->GetParamBufferData(debugBuf, debugData, 0, NUM_DEBUG_VALS * sizeof(unsigned));

				debugOutput = std::wstringstream();
				for(unsigned i = 0; i < NUM_DEBUG_VALS; ++i) debugOutput << debugData[i] << ", ";
				debugOutput << std::endl;
				OutputDebugString(debugOutput.str().c_str());
		}

		//if(keys.downKeys[0x39]) // SPACE
		//{
		//	model->wireframe = !model->wireframe;
		//}

		_snwprintf_s(drawCallText, 100, L"Frame Time: %3.1fms", secs * 1000.0f);
	}

	virtual void Draw() override
	{
		//gpu->DrawGpuModel(model, camera, (Gpu::Light**) &light, 1);

		//gpu->DrawGpuModel(skybox, camera, 0, 0);

		if(particleRenderEffect)
		{
			glm::uvec2 backbufferSize;
			gpu->GetBackbufferSize(backbufferSize.x, backbufferSize.y);
			glm::mat4 viewMatrix = camera->GetViewMatrix();
			glm::mat4 projMatrix = camera->GetProjMatrix(float(backbufferSize.x) / float(backbufferSize.y));
			glm::vec4 consumer(50.0f, 0.0f, 0.0f, 1.0f);

			Gpu::FloatArray viewMatrixFloats((float*)&viewMatrix, 16);
			Gpu::FloatArray projMatrixFloats((float*)&projMatrix, 16);
			Gpu::FloatArray consumerFloats((float*)&consumer, 4);

			particleRenderEffect->SetParam(0, particleBuf1);
			particleRenderEffect->SetParam(1, &viewMatrixFloats);
			particleRenderEffect->SetParam(2, &projMatrixFloats);
			particleRenderEffect->SetParam(3, &consumerFloats);
			particleRenderEffect->SetParam(4, particleTex);

			gpu->SetDepthMode(Gpu::DepthMode_Read);
			gpu->SetBlendMode(Gpu::BlendMode_Additive);

			gpu->DrawIndirect(particleRenderEffect, particleBuf1);

			gpu->SetDepthMode(Gpu::DepthMode_ReadWrite);
			gpu->SetBlendMode(Gpu::BlendMode_Alpha);

			Gpu::ParamBuffer * temp = particleBuf0;
			particleBuf0 = particleBuf1;
			particleBuf1 = temp;
		}

		//if(indirectTriangleEffect)
		//{
		//	gpu->DrawIndirect(indirectTriangleEffect, triParamBuf);
		//}

		//gpu->DrawGpuText(font, drawCallText, 0.0f, 0.0f, false);
	}

};

MAIN_WITH(AdvancedShaderDemo)