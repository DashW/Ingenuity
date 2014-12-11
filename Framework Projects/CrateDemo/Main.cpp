// Crate Demo - texturing - Richard Copperwaite

#include "pch.h"

#define _X86_
#define _USE_MATH_DEFINES

#include <RealtimeApp.h>
#include <GeoBuilder.h>
#include <AssetMgr.h>
#include <math.h>

using namespace Ingenuity;

const float fullCircle = (float)(M_PI * 2);

class CrateDemo : public RealtimeApp
{
	Gpu::Model* cube;
	Gpu::Font* font;
	//GpuDirectionalLight* light;
	Gpu::Camera* camera;
	int textureTicket;
	float cameraRadius, cameraAngle;

public:
	virtual void Begin() override
	{
		font = 0; cube = new Gpu::Model();
		textureTicket = -1;

		gpu->SetClearColor(1.0f, 1.0f, 1.0f, 1.0f);

		font = gpu->CreateGpuFont(40,L"Arial");

		cube->mesh = GeoBuilder().BuildCube()->GpuOnly(gpu);
		//cube = gpu->CreateSphere(2.0f,10,10,true);
		//cube->setColor(1.0f,0.0f,0.0f,1.0f);
		//cube->rotationX = (float) M_PI_2;
		
		Files::Directory * appDir = files->GetKnownDirectory(Files::AppDir);
		textureTicket = assets->Load(appDir, L"crate.jpg", TextureAsset);

		camera = new Gpu::Camera();
		cameraRadius = 5.0f;
		camera->position.y = 3.0f;
		cameraAngle = 0.0f;
		camera->fovOrHeight = (float)(M_PI_4);

		//light = new GpuDirectionalLight();
		//light->setColor(1.0f,1.0f,1.0f);
	}
	virtual void End() override
	{
		if(cube)
		{
			if(cube->mesh) delete cube->mesh;
			//if(cube->texture) delete cube->texture; // asset managed now!
			delete cube;
		}
		if(camera) delete camera;
		if(font) delete font;
	}
	virtual void Update(float secs) override
	{
		if(assets->IsLoaded(textureTicket))
		{
			Files::Directory * appDir = files->GetKnownDirectory(Files::AppDir);
			cube->texture = assets->GetAsset<Gpu::Texture>(appDir, L"crate.jpg");
		}

		cameraAngle = cameraAngle + secs;
		if(cameraAngle > fullCircle) cameraAngle -= fullCircle;

		camera->position.x = sin(cameraAngle)*cameraRadius;
		camera->position.z = cos(cameraAngle)*cameraRadius;
		
		//float lightAngle = static_cast<float>(cameraAngle - M_PI_4);
		//if(lightAngle < 0.0f) lightAngle += fullCircle;
		//light->SetDirection(sin(lightAngle),1.0f,cos(lightAngle));
	}
	virtual void Draw() override
	{
		gpu->DrawGpuModel(cube, camera, 0, 0);

		gpu->DrawGpuText(font,L"Direct3D", 0.0f, 0.0f, false);
	}
};

MAIN_WITH(CrateDemo)