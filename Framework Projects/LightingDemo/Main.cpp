// Lighting Demo - phong shading - Richard Copperwaite

#include "pch.h"

#define _X86_
#define _USE_MATH_DEFINES

#include <RealtimeApp.h>
#include <GeoBuilder.h>
#include <math.h>

using namespace Ingenuity;

const float fullCircle = (float)(M_PI * 2);

class LightingDemo : public RealtimeApp
{
	Gpu::Font* font;
	Gpu::Model* teapot;
	Gpu::DirectionalLight* light;
	Gpu::Camera* camera;
	float cameraRadius, cameraAngle;
	Gpu::Rect textPos;

public:
	virtual void Begin() override
	{
		gpu->SetClearColor(1.0f, 1.0f, 1.0f, 1.0f);

		font = 0; teapot = new Gpu::Model();

		font = gpu->CreateGpuFont(40,L"Arial");
		//teapot = gpu->CreateTeapot();
		//teapot->mesh = gpu->CreateCylinder(0.5f, 3.0f, 20, 20);
		teapot->mesh = GeoBuilder().BuildCapsule(0.5f, 3.0f, 20, 20)->GpuOnly(gpu);
		teapot->color = glm::vec4(1.0f, 0.0f, 0.0f, 1.0f);
		//teapot->wireframe = true;
		//teapot->rotationX = static_cast<float>(M_PI_2);
		//teapot->positionX = 1.0f;
		//teapot = gpu->CreateSphere(1.0f,20,20);
		//teapot = gpu->CreateGrid(2.0f,2.0f,2,2);

		camera = new Gpu::Camera();
		cameraRadius = 5.0f;
		camera->position.y = 3.0f;
		cameraAngle = 0.0f;

		light = new Gpu::DirectionalLight();
		light->color = glm::vec3(1.0f, 1.0f, 1.0f);

		textPos.top = textPos.left = 0.0f;
		textPos.right = 200.0f;
		textPos.bottom = 40.0f;
	}

	virtual void End() override
	{
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

		camera->position.x = sin(cameraAngle)*cameraRadius;
		camera->position.z = cos(cameraAngle)*cameraRadius;
		
		float lightAngle = static_cast<float>(cameraAngle-M_PI_4);
		if(lightAngle < 0.0f) lightAngle += fullCircle;
		light->direction = glm::vec3(sinf(lightAngle),-0.5f,cosf(lightAngle));
	}

	virtual void Draw() override
	{
		gpu->DrawGpuModel(teapot, camera, (Gpu::Light**) &light, 1);

		gpu->DrawGpuText(font,L"Direct3D", 0.0f, 0.0f, false);
	}

};

MAIN_WITH(LightingDemo);