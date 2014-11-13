// Mesh-loading demo - Richard Copperwaite

#include "pch.h"
#include <RealtimeApp.h>
#include <GeoBuilder.h>
#include <WavefrontLoader.h>
#include <InputState.h>
#include <IsoSurface.h>
#define _X86_
#define _USE_MATH_DEFINES
#include <math.h>

using namespace Ingenuity;

const float fullCircle = (float)(M_PI * 2);

class DynamicMeshDemo : public RealtimeApp
{
	Gpu::Font * font;
	Gpu::Model * model, * skybox;
	Gpu::DirectionalLight * light;
	Gpu::Camera * camera;
	Gpu::CubeMap * cubeMap;
	IsoSurface * isoSurface;

	float modelScale, modelPos;
	float cameraRadius, cameraAngle;
	float timePassed;

	int cubemapTicket;

	wchar_t drawCallText[100];

public:
	virtual void Begin() override
	{
		font = 0; model = 0; skybox = 0; light = 0; camera = 0; 
		isoSurface = 0; cubeMap = 0; cubemapTicket = -1; timePassed = 0.0f;

		font = gpu->CreateGpuFont(40,L"Arial");

		modelScale = 8.0f; modelPos = 7.0f;

		isoSurface = new IsoSurface(40, gpu);
		//isoSurface->SetThreshold(1.0f);
		isoSurface->AddMetaball(glm::vec3(), 5.0f);
		isoSurface->AddMetaball(glm::vec3(), 5.0f);
		isoSurface->AddMetaball(glm::vec3(), 5.0f);

		model = new Gpu::Model();
		model->mesh = isoSurface->GetMesh();

		skybox = new Gpu::Model();
		skybox->mesh = GeoBuilder().BuildSkyCube()->GpuOnly(gpu);
		skybox->scale = glm::vec4(20.0f);
		skybox->backFaceCull = false;

		Files::Directory * appDir = files->GetKnownDirectory(Files::AppDir);
		Files::Directory * frameworkDir = files->GetKnownDirectory(Files::FrameworkDir);
		AssetBatch batch;
		batch.emplace_back(appDir, L"DynamicMeshDemo\\skybox_clearsky.dds", CubeMapAsset, "skybox");
		batch.emplace_back(frameworkDir, L"SkyShader.xml", ShaderAsset, "skyshader");
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
	}

	virtual void End() override
	{
		if(skybox->effect)
		{
			delete skybox->effect;
		}
		delete skybox->mesh;
		delete skybox;
		delete model;
		delete isoSurface;
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
			model->cubeMap = cubeMap;

			Gpu::Shader * skyShader = assets->GetAsset<Gpu::Shader>("skyshader");
			if(skyShader)
			{
				skybox->effect = new Gpu::Effect(skyShader);
				skybox->cubeMap = cubeMap;
			}

			cubemapTicket = -1;
		}

		//update balls' position
		timePassed += secs;

		float c = 2.0f*(float)cos(timePassed / 0.6f);

		IsoSurface::Metaball * metaball = isoSurface->GetMetaball(0);
		if(metaball)
		{
			metaball->position.x = -4.0f*(float)cos(timePassed / 0.7f) - c;
			metaball->position.y = 4.0f*(float)sin(timePassed / 0.6f) - c;
		}

		metaball = isoSurface->GetMetaball(1);
		if(metaball)
		{
			metaball->position.x = 5.0f*(float)sin(timePassed / 0.4f) + c;
			metaball->position.y = 5.0f*(float)cos(timePassed / 0.4f) - c;
		}

		metaball = isoSurface->GetMetaball(2);
		if(metaball)
		{
			metaball->position.x = -5.0f*(float)cos(timePassed / 0.4f) - 0.2f*(float)sin(timePassed / 0.6f);
			metaball->position.y = 5.0f*(float)sin(timePassed / 0.5f) - 0.2f*(float)sin(timePassed / 0.4f);
		}

		isoSurface->UpdateObjects();
		isoSurface->UpdateMesh(gpu);
		model->mesh = isoSurface->GetMesh();

		cameraAngle = cameraAngle + (secs/2.0f);
		if(cameraAngle > fullCircle) cameraAngle -= fullCircle;

		if(keys.downKeys[0x39]) // SPACE
		{
			model->wireframe = !model->wireframe;
		}

		model->rotation.y = cameraAngle + float(M_PI);

		_snwprintf_s(drawCallText, 100, L"Frame Time: %3.1fms  Num Metaballs: %d", secs * 1000.0f, isoSurface->GetNumMetaballs());
	}

	virtual void Draw() override
	{
		gpu->DrawGpuModel(model, camera, (Gpu::Light**) &light, 1);

		gpu->DrawGpuModel(skybox, camera, 0, 0);

		gpu->DrawGpuText(font, drawCallText, 0.0f, 0.0f, false);
	}

};

MAIN_WITH(DynamicMeshDemo)