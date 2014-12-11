// Mesh-loading demo - Richard Copperwaite

#include "pch.h"
#include <RealtimeApp.h>
#include <GeoBuilder.h>
#include <WavefrontLoader.h>
#include "SpriteLoadingBar.h"
#define _X86_
#define _USE_MATH_DEFINES
#include <math.h>

using namespace Ingenuity;

const float fullCircle = (float)(M_PI * 2);

class MeshDemo : public RealtimeApp
{
	Gpu::Font * font;
	Gpu::ComplexModel * model;
	Gpu::DirectionalLight * light;
	Gpu::Camera camera, spriteCamera;
	Gpu::Texture * pixTest;
	Gpu::VolumeTexture * dotsTex;
	Gpu::DrawSurface * drawSurface, * drawSurface2;
	Gpu::Model * pixTestSprite, * drawSurfaceSprite;
	Gpu::Effect * halfDotsEffect;
	Gpu::Mesh * spriteMesh;

	SpriteLoadingBar * loadingBar;

	float modelScale, modelPos;
	float cameraRadius, cameraAngle;

	int quickTicket, slowTicket;

public:
	virtual void Begin() override
	{
		font = 0; model = 0; light = 0;
		pixTest = 0; dotsTex = 0; 
		drawSurface = 0; drawSurface2 = 0; drawSurfaceSprite = 0; pixTestSprite = 0;
		halfDotsEffect = 0; loadingBar = 0; quickTicket = -1; slowTicket = -1; spriteMesh = 0;

		modelScale = 1.0f; modelPos = 0.0f;

		font = gpu->CreateGpuFont(40,L"Arial");
		if(font)
		{
			font->color = glm::vec4(1.0f);
		}

		Files::Directory * appDir = files->GetKnownDirectory(Files::AppDir);
		Files::Directory * frameworkDir = files->GetKnownDirectory(Files::FrameworkDir);
		AssetBatch quickAssets;
		quickAssets.emplace_back(appDir, L"RenderTextureDemo/PixTest.dds", TextureAsset, "pixTest");
		quickAssets.emplace_back(appDir, L"RenderTextureDemo/HalftoneDots16x16.dds", VolumeTexAsset, "dotsTex");
		quickAssets.emplace_back(appDir, L"RenderTextureDemo/LoadingRect.png", TextureAsset, "loadingRect");
		quickAssets.emplace_back(frameworkDir, L"HalftoneDots.xml", ShaderAsset, "dotsShader");
		quickTicket = assets->Load(quickAssets);

		slowTicket = assets->Load(appDir, L"RenderTextureDemo/skull3.obj", WavefrontModelAsset, "modelAsset");

		cameraRadius = 15.0f;
		camera.position.y = 12.0f;
		cameraAngle = 0.0f;
		camera.fovOrHeight = (float)(M_PI_4);
		camera.nearClip = 1.f;
		camera.farClip = 5000.f;
		camera.position.x = 0.0f;
		camera.position.z = cameraRadius;

		spriteCamera.isOrthoCamera = true;
		//spriteCamera.position.z = 1.0f;

		light = new Gpu::DirectionalLight();
		light->direction = glm::vec3(sinf(-0.5f),0.25f,cosf(-0.5f));

		loadingBar = new SpriteLoadingBar(this);

		drawSurface = gpu->CreateScreenDrawSurface();
		drawSurface2 = gpu->CreateScreenDrawSurface();

		LocalMesh * spriteGeom = GeoBuilder().BuildGrid(1.0f, 1.0f, 2, 2, &Gpu::Rect(0.0f, 1.0f, 1.0f, 0.0f));
		spriteGeom->vertexBuffer->Transform(glm::eulerAngleX(float(M_PI_2)));
		spriteMesh = spriteGeom->GpuOnly(gpu);
		
		pixTestSprite = new Gpu::Model();
		pixTestSprite->mesh = spriteMesh;
		pixTestSprite->position.z = 1000.0f;

		drawSurfaceSprite = new Gpu::Model();
		drawSurfaceSprite->mesh = spriteMesh;
		drawSurfaceSprite->position.z = 1000.0f;
	}

	virtual void End() override
	{
		if(pixTestSprite) delete pixTestSprite;
		if(drawSurface) delete drawSurface;
		if(drawSurface2) delete drawSurface2;
		if(drawSurfaceSprite) delete drawSurfaceSprite;
		if(spriteMesh) delete spriteMesh;
		if(halfDotsEffect) delete halfDotsEffect;
		if(model) delete model;	
		if(loadingBar) delete loadingBar;
		if(light) delete light;
		if(font) delete font;
	}

	virtual void Update(float secs) override
	{
		if(assets->IsLoaded(quickTicket))
		{
			pixTestSprite->texture = pixTest = assets->GetAsset<Gpu::Texture>("pixTest");
			pixTestSprite->scale = glm::vec4(pixTest->GetWidth(), pixTest->GetHeight(), 1.0f, 1.0f);

			dotsTex = assets->GetAsset<Gpu::VolumeTexture>("dotsTex");

			Gpu::Shader * halfDotsShader = assets->GetAsset<Gpu::Shader>("dotsShader");
			if(halfDotsShader)
			{
				halfDotsEffect = new Gpu::Effect(halfDotsShader);
				halfDotsEffect->SetParam(0, dotsTex);
				halfDotsEffect->SetParam(1, 0.02f);//0.017f);
			}

			loadingBar->SetTexture(assets->GetAsset<Gpu::Texture>("loadingRect"));

			quickTicket = -1;
		}

		// Check whether the model has finished loading
		if(assets->IsLoaded(slowTicket))
		{
			model = assets->GetAsset<Gpu::ComplexModel>("modelAsset");
			if(model)
			{
				model->models[0].color.r = 0.8f;
				model->models[0].color.g = 0.8f;
				model->models[0].color.b = 0.8f;
				model->scale = glm::vec4(modelScale);
				model->position.y = modelPos;
				//model->wireframe = true;
				//model->models[0].backFaceCull = false;
				//model->models[0].normalMap = modelNormal;
				//model->models[0].cubeMap = cubeMap;
			}

			slowTicket = -1;
		}

		loadingBar->Update(assets->GetProgress(slowTicket));

		cameraAngle = cameraAngle + (secs/2.0f);
		if(cameraAngle > fullCircle) cameraAngle -= fullCircle;

		if(model)
		{
			model->rotation.y = cameraAngle + float(M_PI);
		}

		unsigned screenWidth, screenHeight;
		gpu->GetBackbufferSize(screenWidth, screenHeight);
		pixTestSprite->position.x = float(screenWidth) - 20.0f;
		pixTestSprite->position.y = float(screenHeight) - 20.0f;

		spriteCamera.fovOrHeight = -float(screenHeight); // Y downward
		spriteCamera.position.x = spriteCamera.target.x = float(screenWidth) / 2.0f;
		spriteCamera.position.y = spriteCamera.target.y = float(screenHeight) / 2.0f;

		drawSurfaceSprite->scale.x = float(screenWidth);
		drawSurfaceSprite->scale.y = float(screenHeight);
		drawSurfaceSprite->position.x = float(screenWidth) / 2.0f;
		drawSurfaceSprite->position.y = float(screenHeight) / 2.0f;
	}

	virtual void Draw() override
	{
		drawSurface->Clear();
		drawSurface2->Clear();

		if(model)
		{
			model->BeDrawn(gpu, &camera, (Gpu::Light**) &light, 1, drawSurface);
		}

		gpu->DrawGpuSurface(drawSurface,halfDotsEffect,drawSurface2);

		// This is the next step!
		gpu->DrawGpuModel(pixTestSprite, &spriteCamera, 0, 0, drawSurface2);

		drawSurfaceSprite->texture = drawSurface2->GetTexture();
		gpu->DrawGpuModel(drawSurfaceSprite, &spriteCamera, 0, 0);

		if(slowTicket > -1) loadingBar->BeDrawn(gpu);

		gpu->DrawGpuText(font,L"Direct3D", 0.0f, 0.0f, false);
	}

};

MAIN_WITH(MeshDemo)