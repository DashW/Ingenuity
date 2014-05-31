// Crate Demo - texturing - Richard Copperwaite

#include "pch.h"

#include <RealtimeApp.h>
#include <GeoBuilder.h>
#include <GpuShaders.h>
#include <HeightParser.h>
#include <InputState.h>
#define _X86_
#define _USE_MATH_DEFINES
#include <math.h>
#include <Debug.h>

using namespace Ingenuity;

const float fullCircle = (float)(M_PI * 2);

class HeightmapDemo : public RealtimeApp
{
	Gpu::Model * model;
	Gpu::Font * font;
	Gpu::DirectionalLight * light;
	Gpu::Camera * camera;
	Gpu::Effect * multiTexEffect;
	float cameraRadius, cameraAngle;
	int loadTicket;
public:
	virtual void Begin() override
	{
		font = 0; model = new Gpu::Model();
		multiTexEffect = 0; loadTicket = -1;

		gpu->SetClearColor(1.0f, 1.0f, 1.0f, 1.0f);

		Files::Directory * appDir = files->GetKnownDirectory(Files::AppDir);
		Files::Directory * frameworkDir = files->GetKnownDirectory(Files::FrameworkDir);

		AssetBatch assetBatch;
		assetBatch.emplace_back(appDir, L"hmblend.dds", TextureAsset, "hmblend");
		assetBatch.emplace_back(appDir, L"grass.dds", TextureAsset, "grass");
		assetBatch.emplace_back(appDir, L"dirt.dds", TextureAsset, "dirt");
		assetBatch.emplace_back(appDir, L"stone.dds", TextureAsset, "stone");
		assetBatch.emplace_back(frameworkDir, L"MultiTextureAnimY.xml", ShaderAsset, "multitex");
		loadTicket = assets->Load(assetBatch);

		struct HeightmapResponse : public Files::Response
		{
			HeightmapDemo * app;
			HeightmapResponse(HeightmapDemo * app) : app(app) {}
			virtual void Respond() override
			{
				deleteOnComplete = true;
				if(buffer)
				{
					HeightParser parser;
					parser.ParseHeightmap(buffer,129);
					parser.SetScale(2.0f, 0.004f, 2.0f);
					app->model->mesh = parser.GetMesh(&Gpu::Rect(0.0f, 0.0f, 1.0f, 1.0f))->GpuOnly(app->gpu);
				}
			}
		};
		files->OpenAndRead(appDir, L"hmheight.raw", new HeightmapResponse(this));

		font = gpu->CreateGpuFont(40,L"Arial");

		camera = new Gpu::Camera();
		cameraRadius = 2.0f;
		camera->position.y = 1.5f;
		cameraAngle = 0.0f;
		camera->fovOrHeight = (float)(M_PI_4);

		light = new Gpu::DirectionalLight();
	}
	virtual void End() override
	{
		if(multiTexEffect) delete multiTexEffect;
		if(model)
		{
			if(model->mesh) delete model->mesh;
			delete model;
		}
		delete camera;
		delete light;
		delete font;
	}
	virtual void Update(float secs) override
	{
		if(loadTicket > -1 && assets->IsLoaded(loadTicket))
		{
			model->texture = assets->GetAsset<Gpu::Texture>("hmblend");

			Gpu::Shader * shader = assets->GetAsset<Gpu::Shader>("multitex");
			if(shader)
			{
				multiTexEffect = new Gpu::Effect(shader);
				model->effect = multiTexEffect;
				model->effect->SetParam(0, assets->GetAsset<Gpu::Texture>("grass"));
				model->effect->SetParam(1, assets->GetAsset<Gpu::Texture>("dirt"));
				model->effect->SetParam(2, assets->GetAsset<Gpu::Texture>("stone"));
			}

			loadTicket = -1;
		}

		cameraAngle = cameraAngle + secs;
		if(cameraAngle > fullCircle) cameraAngle -= fullCircle;

		camera->position.x = sin(cameraAngle)*cameraRadius;
		camera->position.z = cos(cameraAngle)*cameraRadius;
		
		KeyState & keyboard = input->GetKeyState();

		if(multiTexEffect)
		{
			float & progress = multiTexEffect->params[4]->fvalue;

			if(keyboard.keys[0x48]) // UP
			{
				if(progress < 1.0f) progress += secs * 0.5f;
				//Debug::Get().Break();
			}
			if(keyboard.keys[0x50]) // DOWN
			{
				if(progress > 0.0f) progress -= secs * 0.5f;
			}
			if(keyboard.downKeys[0x39]) // SPACE
			{
				model->effect = (model->effect ? 0 : multiTexEffect);
			}
		}

		float lightAngle = static_cast<float>(cameraAngle - M_PI_4);
		if(lightAngle < 0.0f) lightAngle += fullCircle;
		light->direction = glm::vec3(sinf(lightAngle),1.0f,cosf(lightAngle));
	}
	virtual void Draw() override
	{
		gpu->DrawGpuModel(model, camera, (Gpu::Light**) &light, 1);
		gpu->DrawGpuText(font,L"Direct3D", 0.0f, 0.0f, false);
	}
};

MAIN_WITH(HeightmapDemo)