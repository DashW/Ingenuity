// Complex Mesh Demo, for meshes with multiple materials - Richard Copperwaite

#include "pch.h"
#include <RealtimeApp.h>
#include <WavefrontLoader.h>
#define _X86_
#define _USE_MATH_DEFINES
#include <math.h>

const float fullCircle = (float)(M_PI * 2);

class MeshDemo : public RealtimeApp
{
	GpuFont* font;
	GpuModel* teapot;
	GpuDirectionalLight* light;
	GpuCamera* camera;
	float cameraRadius, cameraAngle;

public:
	virtual void Begin() override
	{
		font = 0; teapot = 0; light = 0; camera = 0;

		font = gpu->CreateGpuFont(40,L"Arial");

		WavefrontLoader loader;
		teapot = loader.LoadModel(gpu,files,L"vase.obj");
		// http://thefree3dmodels.com/stuff/furniture/vase/12-1-0-3937 //

		camera = new GpuCamera();
		cameraRadius = 200.0f;
		camera->y = 200.0f;
		cameraAngle = 0.0f;
		camera->fov = (float)(M_PI_4);
		camera->nearClip = 1.f;
		camera->farClip = 5000.f;

		light = new GpuDirectionalLight();
		light->setColor(1.0f,1.0f,1.0f);
	}

	virtual void End() override
	{
		delete teapot->texture;
		delete teapot->mesh;
		delete teapot;
		delete light;
		delete camera;
		delete font;
	}

	virtual void Update(float secs) override
	{
		cameraAngle = cameraAngle + secs;
		if(cameraAngle > fullCircle) cameraAngle -= fullCircle;

		camera->x = sin(cameraAngle)*cameraRadius;
		camera->z = cos(cameraAngle)*cameraRadius;
		
		float lightAngle = static_cast<float>(cameraAngle-0.5f);
		if(lightAngle < 0.0f) lightAngle += fullCircle;
		
		light->SetDirection(sin(lightAngle),0.25f,cos(lightAngle));
	}

	virtual void Draw() override
	{
		gpu->DrawGpuModel(teapot, camera, (GpuLight**) &light, 1);

		gpu->DrawGpuText(font,L"Direct3D", 0.0f, 0.0f, false);
	}

};

MAIN_WITH(MeshDemo)