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
	Gpu::Camera * camera;
	Gpu::Texture * pixTest;
	Gpu::VolumeTexture * dotsTex;
	Gpu::DrawSurface * drawSurface, * drawSurface2;
	Gpu::Sprite * drawSurfaceSprite, * pixTestSprite;
	Gpu::Effect * halfDotsEffect;

	SpriteLoadingBar * loadingBar;

	float modelScale, modelPos;
	float cameraRadius, cameraAngle;

	int quickTicket, slowTicket;

public:
	virtual void Begin() override
	{
		font = 0; model = 0; light = 0; camera = 0;
		pixTest = 0; dotsTex = 0; 
		drawSurface = 0; drawSurface2 = 0; drawSurfaceSprite = 0; pixTestSprite = 0;
		halfDotsEffect = 0; loadingBar = 0; quickTicket = -1; slowTicket = -1;

		modelScale = 1.0f; modelPos = 0.0f;

		font = gpu->CreateGpuFont(40,L"Arial");
		font->color = glm::vec4(1.0f);

		Files::Directory * appDir = files->GetKnownDirectory(Files::AppDir);
		Files::Directory * frameworkDir = files->GetKnownDirectory(Files::FrameworkDir);
		AssetBatch quickAssets;
		quickAssets.emplace_back(appDir, L"RenderTextureDemo/PixTest.dds", TextureAsset, "pixTest");
		quickAssets.emplace_back(appDir, L"RenderTextureDemo/HalftoneDots16x16.dds", VolumeTexAsset, "dotsTex");
		quickAssets.emplace_back(appDir, L"RenderTextureDemo/LoadingRect.png", TextureAsset, "loadingRect");
		quickAssets.emplace_back(frameworkDir, L"HalftoneDots.xml", ShaderAsset, "dotsShader");
		quickTicket = assets->Load(quickAssets);

		slowTicket = assets->Load(appDir, L"RenderTextureDemo/skull3.obj", WavefrontModelAsset, "modelAsset");

		camera = new Gpu::Camera();
		cameraRadius = 15.0f;
		camera->position.y = 12.0f;
		cameraAngle = 0.0f;
		camera->fovOrHeight = (float)(M_PI_4);
		camera->nearClip = 1.f;
		camera->farClip = 5000.f;
		camera->position.x = 0.0f;
		camera->position.z = cameraRadius;

		light = new Gpu::DirectionalLight();
		light->direction = glm::vec3(sinf(-0.5f),0.25f,cosf(-0.5f));

		loadingBar = new SpriteLoadingBar(this);

		drawSurface = gpu->CreateScreenDrawSurface();
		drawSurface2 = gpu->CreateScreenDrawSurface();
		drawSurfaceSprite = new Gpu::Sprite();
		drawSurfaceSprite->pixelSpace = true;

		pixTestSprite = new Gpu::Sprite();
		pixTestSprite->pixelSpace = true;
		pixTestSprite->texture = pixTest;
	}

	virtual void End() override
	{
		if(pixTestSprite) delete pixTestSprite;
		if(drawSurface) delete drawSurface;
		if(drawSurface2) delete drawSurface2;
		if(drawSurfaceSprite) delete drawSurfaceSprite;
		if(halfDotsEffect) delete halfDotsEffect;
		if(model) delete model;	
		if(loadingBar) delete loadingBar;
		if(light) delete light;
		if(camera) delete camera;
		if(font) delete font;
	}

	virtual void Update(float secs) override
	{
		if(assets->IsLoaded(quickTicket))
		{
			pixTestSprite->texture = pixTest = assets->GetAsset<Gpu::Texture>("pixTest");
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
				model->scale = glm::vec3(modelScale);
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
		pixTestSprite->position.x = screenWidth - 40.0f;
		pixTestSprite->position.y = screenHeight - 40.0f;
	}

	virtual void Draw() override
	{
		drawSurface->Clear();
		drawSurface2->Clear();

		if(model)
		{
			model->BeDrawn(gpu, camera, (Gpu::Light**) &light, 1, drawSurface);
		}

		gpu->DrawGpuSurface(drawSurface,halfDotsEffect,drawSurface2);

		gpu->DrawGpuSprite(pixTestSprite,drawSurface2);

		drawSurfaceSprite->texture = drawSurface2->GetTexture();
		gpu->DrawGpuSprite(drawSurfaceSprite);

		if(slowTicket > -1) loadingBar->BeDrawn(gpu);

		gpu->DrawGpuText(font,L"Direct3D", 0.0f, 0.0f, false);
	}

};

MAIN_WITH(MeshDemo)