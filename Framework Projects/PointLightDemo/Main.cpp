// Spot Light Demo - lighting multiple objects - Richard Copperwaite

// Okay, this is the demo we're going to use to light multiple objects
// with multiple lights of different types.

#include "pch.h"

#define _X86_
#define _USE_MATH_DEFINES

#include <RealtimeApp.h>
#include <GeoBuilder.h>
#include <math.h>
#include <vector>

using namespace Ingenuity;

const float fullCircle = (float)(M_PI * 2);

class LightingDemo : public RealtimeApp
{
	Gpu::Font* font;
	Gpu::Model* floor;
	Gpu::Model* cylinder;
	Gpu::Model* sphere;
	//GpuDirectionalLight* light;
	//GpuPointLight* light;
	Gpu::SpotLight* light;
	Gpu::Camera* camera;
	std::vector<Gpu::Light*> lights;
	float cameraRadius, cameraAngle;

public:
	virtual void Begin() override
	{
		gpu->SetClearColor(1.0f, 1.0f, 1.0f, 1.0f);

		font = gpu->CreateGpuFont(40,L"Arial");

		GeoBuilder builder;
		
		floor = new Gpu::Model();
		//floor->mesh = gpu->CreateGrid(80.0f,80.0f,2,2); 
		floor->mesh = builder.BuildGrid(80.0f,80.0f,2,2)->GpuOnly(gpu);

		cylinder = new Gpu::Model();
		//cylinder->mesh = gpu->CreateCylinder(1.0f,6.0f,20,20); 
		cylinder->mesh = builder.BuildCylinder(1.0f,6.0f,20,20)->GpuOnly(gpu);
		cylinder->position.y = 3.0f;
		cylinder->rotation.x = static_cast<float>(M_PI_2); // FIXME

		sphere = new Gpu::Model();
		//sphere->mesh = gpu->CreateSphere(1.0f, 20, 20); // IMPROVE
		sphere->mesh = builder.BuildSphere(1.0f, 20, 20)->GpuOnly(gpu);
		sphere->position.y = 7.5f;

		camera = new Gpu::Camera();
		cameraRadius = 80.0f;
		camera->position.y = 15.0f;
		cameraAngle = 0.0f;
		camera->fovOrHeight = (float)(M_PI_4);

		//light = new GpuDirectionalLight();

		//light = new GpuPointLight();
		//light->SetPosition(0.0f, 3.0f, 0.0f);

		light = new Gpu::SpotLight();
		light->power = 32.0f;
		light->color = glm::vec3(0.5f, 0.5f, 0.5f);
		lights.push_back(light);

		Gpu::PointLight * cornerLight1 = new Gpu::PointLight();
		cornerLight1->position = glm::vec3(40.0f, 10.0f, 40.0f);
		cornerLight1->color = glm::vec3(0.5f, 0.5f, 1.0f);
		lights.push_back(cornerLight1);

		Gpu::PointLight * cornerLight2 = new Gpu::PointLight();
		cornerLight2->position = glm::vec3(-40.0f, 10.0f, -40.0f);
		cornerLight2->color = glm::vec3(1.0f, 0.5f, 0.5f);
		lights.push_back(cornerLight2);
	}

	virtual void End() override
	{
		delete floor->mesh;
		delete floor;
		delete cylinder->mesh;
		delete cylinder;
		delete sphere->mesh;
		delete sphere;
		delete camera;
		for(unsigned i = 0; i < lights.size(); ++i)
		{
			delete lights[i];
		}
		delete font;
	}

	virtual void Update(float secs) override
	{
		cameraAngle = cameraAngle + (secs * 0.2f);
		if(cameraAngle > fullCircle) cameraAngle -= fullCircle;

		camera->position.x = sinf(cameraAngle)*cameraRadius;
		camera->position.z = cosf(cameraAngle)*cameraRadius;

		// Directional Light
		//float lightAngle = static_cast<float>(cameraAngle - M_PI_2);
		//if(lightAngle < 0.0f) lightAngle += fullCircle;
		//light->SetDirection(sin(lightAngle),0.5f,cos(lightAngle));
		
		// Point Light
		//light->z = sinf(cameraAngle) * 25;

		// Spot Light
		light->position = glm::vec3(camera->position.x,camera->position.y,camera->position.z);
		light->direction = glm::vec3(-light->position.x,-light->position.y,-light->position.z);
	}

	virtual void Draw() override
	{
		for(unsigned i = 0; i < 7; i++)
		{
			cylinder->position.x = sphere->position.x = -10.0f;
			cylinder->position.z = sphere->position.z = (i-3.0f) * 10.0f;
			gpu->DrawGpuModel(sphere, camera, lights.data(), lights.size());
			gpu->DrawGpuModel(cylinder, camera, lights.data(), lights.size());

			cylinder->position.x = sphere->position.x = 10.0f;
			gpu->DrawGpuModel(sphere, camera, lights.data(), lights.size());
			gpu->DrawGpuModel(cylinder, camera, lights.data(), lights.size());
		}

		gpu->DrawGpuModel(floor, camera, lights.data(), lights.size());

		//sphere->positionX = light->x; sphere->positionZ = light->z;
		//sphere->positionY = light->y;
		//sphere->setColor(1.0f,1.0f,1.0f,1.0f);
		//gpu->DrawGpuIndexedMesh(sphere, camera, 0, 0);
		//sphere->positionY = 7.5f;
		//sphere->setColor(1.0f,0.0f,0.0f,1.0f);

		gpu->DrawGpuText(font,L"Direct3D", 0.0f, 0.0f, false);
	}

};

MAIN_WITH(LightingDemo)