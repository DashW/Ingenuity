#include "pch.h"

#define _X86_
#define _USE_MATH_DEFINES

#include <RealtimeApp.h>
#include <math.h>

const float fullCircle = (float)(M_PI * 2);

class LightingDemo : public RealtimeApp
{
	GpuFont* font;
	GpuIndexedMesh *floor, *cylinder, *sphere;
	//GpuDirectionalLight* light;
	GpuPointLight* light;
	//GpuSpotLight* light;
	GpuCamera* camera;
	float cameraRadius, cameraAngle, textx, texty;
	GpuTexture *sphereTexture, *cylinderTexture, *floorTexture, *flareTexture, *nightSkyTexture;
	GpuSprite *flare, *nightSky;

public:

	virtual void Begin() override
	{
		gpu->SetClearColor(0.0f,0.0f,0.0f);

		font = gpu->CreateGpuFont(40,L"Arial");
		font->colorR = 1.0f; font->colorG = 1.0f; font->colorB = 1.0f; font->colorA = 0.5f;
		floor = gpu->CreateGrid(80.0f,80.0f,2,2,new GpuRect(0.0f,0.0f,1.0f,1.0f));
		floor->texture = floorTexture = gpu->CreateGpuTextureFromFile(L"stone2.dds");

		cylinder = gpu->CreateCylinder(1.0f,6.0f,20,20,false);
		cylinder->positionY = 3.0f;
		cylinder->rotationX = static_cast<float>(M_PI_2);
		//cylinder->texture = cylinderTexture = gpu->CreateGpuTextureFromFile(L"stone2.dds");
		
		sphere = gpu->CreateSphere(1.0f, 20, 20, false);
		sphere -> positionY = 7.5f;
		//sphere->texture = sphereTexture = gpu->CreateGpuTextureFromFile(L"marble.bmp");

		flare = new GpuSprite();
		flare->pixelSpace = false;
		flare->positionY = -0.2f; 
		flare->brightAsAlpha = true;
		flare->texture = flareTexture = gpu->CreateGpuTextureFromFile(L"lensflare.png");
		flare->transformCenterX = 128.0f; flare->transformCenterY = 128.0f;

		nightSky = new GpuSprite();
		nightSky->pixelSpace = false;
		nightSky->texture = nightSkyTexture = gpu->CreateGpuTextureFromFile(L"starrysky.png");
		nightSky->transformCenterX = 512.0f; nightSky->transformCenterY = 512.0f;

		camera = new GpuCamera();
		cameraRadius = 80.0f; //50
		camera->y = 20.0f; //15
		cameraAngle = 0.0f;

		light = new GpuPointLight();
		light->SetPosition(0.0f, 3.0f, 0.0f);
		
		light->setColor(1.0f,1.0f,1.0f);

		textx = texty = 0.0f;
	}

	virtual void End() override
	{
		delete nightSkyTexture;
		delete flareTexture;
		delete floorTexture;
		//delete sphereTexture;
		//delete cylinderTexture;
		delete floor;
		delete cylinder;
		delete sphere;
		delete font;
	}

	virtual void Update(float secs) override
	{
		cameraAngle = cameraAngle + (secs/2.0f);
		if(cameraAngle > fullCircle) cameraAngle -= fullCircle;

		camera->x = sinf(cameraAngle)*cameraRadius;
		camera->z = cosf(cameraAngle)*cameraRadius;
		
		//light->z = sinf(cameraAngle) * 25;
	}

	virtual void Draw() override
	{
		//gpu->DrawGpuSprite(nightSky);

		gpu->LookTransform(
			camera->x, camera->y, camera->z,
			0.f,0.f,0.f,
			0.f,1.f,0.f);

		gpu->PerspectiveTransform((float)(M_PI_4), 
			(windowRect.right)/(windowRect.bottom), 1.f, 5000.f);

		for(int i = 0; i < 7; i++)
		{
			cylinder->positionX = sphere->positionX = -10.0f;
			cylinder->positionZ = sphere->positionZ = (i-3) * 10.0f;
			gpu->DrawGpuIndexedMesh(sphere, camera, (GpuLight**) &light, 1);
			gpu->DrawGpuIndexedMesh(cylinder, camera, (GpuLight**) &light, 1);

			cylinder->positionX = sphere->positionX = 10.0f;
			gpu->DrawGpuIndexedMesh(sphere, camera, (GpuLight**) &light, 1);
			gpu->DrawGpuIndexedMesh(cylinder, camera, (GpuLight**) &light, 1);
		}

		//sphere->positionX = light->x; sphere->positionZ = light->z;
		//sphere->positionY = light->y;
		//sphere->setColor(1.0f,1.0f,1.0f,1.0f);
		//sphere->texture = 0;
		//gpu->DrawIndexedMesh(sphere, camera, 0, 0);
		//sphere->positionY = 7.5f;
		//sphere->setColor(1.0f,0.0f,0.0f,1.0f);
		//sphere->texture = sphereTexture;

		gpu->DrawGpuIndexedMesh(floor, camera,  (GpuLight**) &light, 1);

		//gpu->DrawGpuSprite(flare);

		gpu->DrawGpuText(font,L"Direct3D",textx,texty,false);
	}

};

MAIN_WITH(LightingDemo)