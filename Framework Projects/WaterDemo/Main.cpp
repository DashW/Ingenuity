// Water demo - Richard Copperwaite

#include "pch.h"
#include <RealtimeApp.h>
#include <GeoBuilder.h>
#include <WavefrontLoader.h>
#define _X86_
#define _USE_MATH_DEFINES
#include <math.h>

using namespace Ingenuity;

const float fullCircle = (float)(M_PI * 2);

class MeshDemo : public RealtimeApp
{
	Gpu::Font * font;
	Gpu::Model * floor, * water;
	Gpu::DirectionalLight * light;
	Gpu::Camera * camera;
	Gpu::Texture * floorTex, * floorNormal, * waterTex, * waterN1, * waterN2;
	Gpu::CubeMap * cubeMap;
	Gpu::Effect * waterEffect;

	float modelScale, modelPos;
	float cameraRadius, cameraAngle;
	unsigned assetTicket;

public:
	virtual void Begin() override
	{
		font = 0; floor = 0; water = 0; light = 0; camera = 0;
		floorTex = 0; floorNormal = 0; cubeMap = 0; assetTicket = -1;

		font = gpu->CreateGpuFont(40,L"Arial");

		modelScale = 8.0f; modelPos = 7.0f;

		Files::Directory * appDir = files->GetKnownDirectory(Files::AppDir);
		Files::Directory * frameworkDir = files->GetKnownDirectory(Files::FrameworkDir);

		AssetBatch batch;
		batch.emplace_back(appDir, L"WaterDemo/grassCubeMap.dds", CubeMapAsset);
		batch.emplace_back(appDir, L"WaterDemo/floor_color.bmp", TextureAsset);
		batch.emplace_back(appDir, L"WaterDemo/floor_nmap.bmp", TextureAsset);
		batch.emplace_back(appDir, L"WaterDemo/whiteTex.dds", TextureAsset);
		batch.emplace_back(appDir, L"WaterDemo/wave0.dds", TextureAsset);
		batch.emplace_back(appDir, L"WaterDemo/wave1.dds", TextureAsset);
		batch.emplace_back(frameworkDir, L"WaterShader.xml", ShaderAsset);
		assetTicket = assets->Load(batch);

		GeoBuilder builder;

		floor = new Gpu::Model();
		floor->mesh = builder.BuildGrid(20.0f,20.0f,2,2,&Gpu::Rect(0.0f,0.0f,5.0f,5.0f),true)->GpuOnly(gpu);
		floor->position.y = -2.0f;

		water = new Gpu::Model();
		water->mesh = builder.BuildGrid(20.0f,20.0f,2,2,&Gpu::Rect(0.0f,0.0f,2.0f,2.0f),true)->GpuOnly(gpu);
		//water->effect = waterEffect = new GpuEffect(gpu->GetShader("WaterShader"));
		water->texture = waterTex;
		water->normalMap = waterN1;
		water->cubeMap = cubeMap;
		water->color = glm::vec4(0.5f,0.5f,1.0f,0.85f);

		camera = new Gpu::Camera();
		cameraRadius = 14.0f;
		camera->position.y = 8.0f;
		cameraAngle = 0.0f;
		camera->fovOrHeight = (float)(M_PI_4);
		camera->nearClip = 1.f;
		camera->farClip = 5000.f;
		camera->position.x = 0.0f;
		camera->position.z = cameraRadius;

		light = new Gpu::DirectionalLight();
		light->direction = glm::vec3(sinf(-0.5f),0.75f,cosf(-0.5f));
	}

	virtual void End() override
	{
		//delete cubeMap;

		delete water->mesh;
		delete water->effect;
		delete water;

		delete floor->mesh;
		delete floor;

		delete light;
		delete camera;
		delete font;
	}

	virtual void Update(float secs) override
	{
		if(assets->IsLoaded(assetTicket))
		{
			Files::Directory * appDir = files->GetKnownDirectory(Files::AppDir);
			Files::Directory * frameworkDir = files->GetKnownDirectory(Files::FrameworkDir);

			floor->texture = floorTex = assets->GetAsset<Gpu::Texture>(appDir, L"WaterDemo/floor_nmap.bmp");
			floor->normalMap = floorNormal = assets->GetAsset<Gpu::Texture>(appDir, L"WaterDemo/floor_color.bmp");
			water->texture = waterTex = assets->GetAsset<Gpu::Texture>(appDir, L"WaterDemo/whiteTex.dds");
			water->cubeMap = cubeMap = assets->GetAsset<Gpu::CubeMap>(appDir, L"WaterDemo/grassCubeMap.dds");
			water->normalMap = waterN1 = assets->GetAsset<Gpu::Texture>(appDir, L"WaterDemo/wave0.dds");
			waterN2 = assets->GetAsset<Gpu::Texture>(appDir, L"WaterDemo/wave1.dds");

			Gpu::Shader * waterShader = assets->GetAsset<Gpu::Shader>(frameworkDir, L"WaterShader.xml");
			if(waterShader)
			{
				waterEffect = new Gpu::Effect(waterShader);
				waterEffect->SetParam(0, waterN2);
				waterEffect->SetParam(1, 0.0f);
				waterEffect->SetParam(2, 0.0f);
				waterEffect->SetParam(3, 0.0f);
				waterEffect->SetParam(4, 0.0f);
			}
			water->effect = waterEffect;

			assetTicket = -1;
		}

		cameraAngle = cameraAngle + (secs/2.0f);
		if(cameraAngle > fullCircle) cameraAngle -= fullCircle;

		//camera->x = sin(cameraAngle)*cameraRadius;
		//camera->z = cos(cameraAngle)*cameraRadius;
		
		//float lightAngle = static_cast<float>(/*cameraAngle*/-0.5f);
		//if(lightAngle < 0.0f) lightAngle += fullCircle;
		
		//light->SetDirection(sin(lightAngle),0.25f,cos(lightAngle));

		const float flowspeed = secs * 0.05f;

		float & offsetX1 = waterEffect->params[1]->fvalue;
		float & offsetY1 = waterEffect->params[2]->fvalue;
		float & offsetX2 = waterEffect->params[3]->fvalue;
		float & offsetY2 = waterEffect->params[4]->fvalue;

		offsetX1 += flowspeed;
		if(offsetX1 > 1.0f) offsetX1 -= 1.0f;
		offsetX2 -= flowspeed;
		if(offsetX2 < 0.0f) offsetX2 += 1.0f;
		offsetY1 += flowspeed;
		if(offsetY1 > 1.0f) offsetY1 -= 1.0f;
		offsetY2 += flowspeed;
		if(offsetY2 > 1.0f) offsetY2 -= 1.0f;

		floor->rotation.y = cameraAngle;
		water->rotation.y = cameraAngle;
	}

	virtual void Draw() override
	{
		gpu->DrawGpuModel(floor, camera, (Gpu::Light**) &light, 1);
		gpu->DrawGpuModel(water, camera, (Gpu::Light**) &light, 1);

		gpu->DrawGpuText(font,L"Direct3D", 0.0f, 0.0f, false);
	}

};

MAIN_WITH(MeshDemo)