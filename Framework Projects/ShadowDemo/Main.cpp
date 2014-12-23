// Shadow Demo - Richard Copperwaite

// Okay, this is the demo we're going to use to light multiple objects
// with multiple lights of different types.

#include "pch.h"

#define _X86_
#define _USE_MATH_DEFINES

#include <RealtimeApp.h>
#include <GeoBuilder.h>
#include <math.h>
#include <vector>

//#define DIR_LIGHT

using namespace Ingenuity;

const float fullCircle = (float)(M_PI * 2);

class ShadowDemo : public RealtimeApp
{
	Gpu::Font * font;
	Gpu::Model * floor;
	Gpu::Model * cylinder;
	Gpu::Model * sphere;
	Gpu::Model * sprite;
#ifdef DIR_LIGHT
	Gpu::DirectionalLight * light;
#else
	Gpu::SpotLight * light;
#endif
	Gpu::Camera viewCamera, lightCamera, spriteCamera;
	Gpu::DrawSurface * shadowSurface;
	Gpu::Effect * shadowEffect;
	float cameraRadius, cameraAngle, lightRadius;
	int ticket;

public:
	virtual void Begin() override
	{
		gpu->SetClearColor(1.0f, 1.0f, 1.0f, 1.0f);

		font = gpu->CreateGpuFont(40,L"Arial");

		Files::Directory* frameworkDir = files->GetKnownDirectory(Files::FrameworkDir);
		Files::Directory* appDir = files->GetKnownDirectory(Files::AppDir);
		AssetBatch assetBatch;
		assetBatch.emplace_back(frameworkDir, L"ShadowLit.xml", ShaderAsset, "shadowShader");
		assetBatch.emplace_back(appDir, L"floor.dds", TextureAsset, "floorTexture");
		ticket = assets->Load(assetBatch);

		GeoBuilder builder;
		
		floor = new Gpu::Model();
		floor->mesh = builder.BuildGrid(80.0f, 80.0f, 2, 2, &Gpu::Rect(0.0f, 0.0f, 4.0f, 4.0f))->GpuOnly(gpu);

		cylinder = new Gpu::Model();
		cylinder->mesh = builder.BuildCylinder(1.0f,6.0f,20,20)->GpuOnly(gpu);
		cylinder->position.y = 3.0f;
		cylinder->rotation.x = static_cast<float>(M_PI_2); // FIXME

		sphere = new Gpu::Model();
		sphere->mesh = builder.BuildSphere(1.0f, 20, 20)->GpuOnly(gpu);
		sphere->position.y = 7.5f;

		cameraRadius = 80.0f;
		cameraAngle = 0.0f;
		viewCamera.position.y = 15.0f;
		viewCamera.fovOrHeight = (float)(M_PI_4);

		//light = new GpuPointLight();
		//light->SetPosition(0.0f, 3.0f, 0.0f);

#ifdef DIR_LIGHT
		light = new Gpu::DirectionalLight();

		lightCamera.isOrthoCamera = true;
		lightCamera.fovOrHeight = 120.0f;
#else
		Gpu::SpotLight * spotLight = new Gpu::SpotLight();
		spotLight->power = 32.0f;
		light = spotLight;

		lightCamera.isOrthoCamera = false;
		lightCamera.fovOrHeight = float(M_PI) / 4.0f;

		//lightCamera.isOrthoCamera = true;
		//lightCamera.fovOrHeight = 120.0f;
#endif

		lightCamera.nearClip = 1.0f;
		lightCamera.farClip = 200.0f;

		lightRadius = 100.0f;

		shadowSurface = gpu->CreateDrawSurface(2048, 2048, Gpu::DrawSurface::Format_Typeless);

		spriteCamera.isOrthoCamera = true;

		LocalMesh * spriteGeom = builder.BuildGrid(1.0f, 1.0f, 2, 2, &Gpu::Rect(0.0f, 1.0f, 1.0f, 0.0f));
		spriteGeom->vertexBuffer->Transform(glm::eulerAngleX(float(M_PI_2)));
		Gpu::Mesh * spriteMesh = spriteGeom->GpuOnly(gpu);

		sprite = new Gpu::Model();
		sprite->mesh = spriteMesh;
		sprite->position.z = 2.0f;
		sprite->texture = shadowSurface->GetTexture();
		sprite->scale = glm::vec4(256.0f,256.0f,1.0f,1.0f);
	}

	virtual void End() override
	{
		delete shadowSurface;
		delete shadowEffect;
		delete floor->mesh;
		delete floor;
		delete cylinder->mesh;
		delete cylinder;
		delete sphere->mesh;
		delete sphere;
		delete sprite->mesh;
		delete sprite;
		delete light;
		delete font;
	}

	virtual void Update(float secs) override
	{
		if(ticket > -1 && assets->IsLoaded(ticket))
		{
			Gpu::Shader * shadowShader = assets->GetAsset<Gpu::Shader>("shadowShader");
			if(shadowShader)
			{
				shadowEffect = new Gpu::Effect(shadowShader);
			}
			
			floor->texture = assets->GetAsset<Gpu::Texture>("floorTexture");

			ticket = -1;
		}

		cameraAngle = cameraAngle + (secs * 0.2f);
		if(cameraAngle > fullCircle) cameraAngle -= fullCircle;

		viewCamera.position.x = sinf(cameraAngle)*cameraRadius;
		viewCamera.position.z = cosf(cameraAngle)*cameraRadius;

		float lightAngle = static_cast<float>(cameraAngle + M_PI_2);
		if(lightAngle > fullCircle) lightAngle -= fullCircle;

#ifdef DIR_LIGHT
		light->direction = glm::vec3(sin(lightAngle),0.5f,cos(lightAngle));

		// MEGA FIXME! Direction needs to be inverted here and in DX11Shaders.cpp, and in every usage!
		lightCamera.position = light->direction * lightRadius;

		shadowEffect->SetParam(2, 0.005f);
#else
		light->position = glm::vec3(sin(lightAngle),0.5f,cos(lightAngle)) * lightRadius;
		light->direction = glm::normalize(-light->position);

		lightCamera.position = light->position;

		shadowEffect->SetParam(2, 0.0001f);
#endif

		// Point Light
		//light->z = sinf(cameraAngle) * 25;

		unsigned screenWidth, screenHeight;
		gpu->GetBackbufferSize(screenWidth, screenHeight);
		sprite->position.x = float(screenWidth) - (sprite->scale.x / 2.0f);
		sprite->position.y = float(screenHeight) - (sprite->scale.y / 2.0f);

		spriteCamera.fovOrHeight = -float(screenHeight); // Y downward
		spriteCamera.position.x = spriteCamera.target.x = float(screenWidth) / 2.0f;
		spriteCamera.position.y = spriteCamera.target.y = float(screenHeight) / 2.0f;
	}

	void DrawScene(Gpu::Camera * camera, Gpu::DrawSurface * surface, Gpu::Effect * overrideEffect)
	{
		// Transform NDC space [-1,+1]^2 to texture space [0,1]^2
		static const glm::mat4 texCoordTransform(
			0.5f, 0.0f, 0.0f, 0.0f,
			0.0f, -0.5f, 0.0f, 0.0f,
			0.0f, 0.0f, 1.0f, 0.0f,
			0.5f, 0.5f, 0.0f, 1.0f);

		glm::mat4 shadowMatrix = texCoordTransform * lightCamera.GetProjMatrix(1.0f) * lightCamera.GetViewMatrix();
		Gpu::FloatArray shadowFloatArray((float*)&shadowMatrix, 16);
		shadowEffect->SetParam(0, &shadowFloatArray);
		shadowEffect->SetParam(1, shadowSurface->GetTexture());

		for(unsigned i = 0; i < 7; i++)
		{
			cylinder->position.x = sphere->position.x = -10.0f;
			cylinder->position.z = sphere->position.z = (i - 3.0f) * 10.0f;

			gpu->DrawGpuModel(sphere, camera, (Gpu::Light**) &light, 1, surface, 0, overrideEffect);
			gpu->DrawGpuModel(cylinder, camera, (Gpu::Light**) &light, 1, surface, 0, overrideEffect);

			cylinder->position.x = sphere->position.x = 10.0f;

			gpu->DrawGpuModel(sphere, camera, (Gpu::Light**) &light, 1, surface, 0, overrideEffect);
			gpu->DrawGpuModel(cylinder, camera, (Gpu::Light**) &light, 1, surface, 0, overrideEffect);
		}

		gpu->DrawGpuModel(floor, camera, (Gpu::Light**) &light, 1, surface, 0, overrideEffect);
	}

	virtual void Draw() override
	{
		shadowSurface->Clear();

		// First, draw to the shadow map
		DrawScene(&lightCamera, shadowSurface, 0);

		// Then, draw to the backbuffer
		DrawScene(&viewCamera, 0, shadowEffect);

		gpu->DrawGpuModel(sprite, &spriteCamera, 0, 0);

		gpu->DrawGpuText(font,L"Ingenuity", 0.0f, 0.0f, false);
	}

};

MAIN_WITH(ShadowDemo)