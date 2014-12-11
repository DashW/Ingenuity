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

using namespace Ingenuity;

const float fullCircle = (float)(M_PI * 2);

class ShadowDemo : public RealtimeApp
{
	Gpu::Font * font;
	Gpu::Model * floor;
	Gpu::Model * cylinder;
	Gpu::Model * sphere;
	Gpu::Model * sprite;
	//GpuDirectionalLight* light;
	//GpuPointLight* light;
	Gpu::DirectionalLight * light;
	Gpu::Camera camera, lightCamera, spriteCamera;
	Gpu::DrawSurface * shadowSurface;
	Gpu::Effect * shadowEffect;
	//std::vector<Gpu::Light*> lights;
	float cameraRadius, cameraAngle, lightRadius;
	int ticket;

public:
	virtual void Begin() override
	{
		gpu->SetClearColor(1.0f, 1.0f, 1.0f, 1.0f);

		font = gpu->CreateGpuFont(40,L"Arial");

		Files::Directory* directory = files->GetKnownDirectory(Files::FrameworkDir);
		ticket = assets->Load(directory, L"ShadowLit.xml", ShaderAsset, "shadowShader");

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

		cameraRadius = 80.0f;
		camera.position.y = 15.0f;
		cameraAngle = 0.0f;
		camera.fovOrHeight = (float)(M_PI_4);

		light = new Gpu::DirectionalLight();

		lightCamera.isOrthoCamera = true;
		lightCamera.nearClip = 1.0f;
		lightCamera.farClip = 200.0f;
		lightCamera.fovOrHeight = 120.0f;

		lightRadius = 100.0f;

		//light = new GpuPointLight();
		//light->SetPosition(0.0f, 3.0f, 0.0f);

		//light = new Gpu::SpotLight();
		//light->power = 32.0f;
		//light->color = glm::vec3(0.5f, 0.5f, 0.5f);
		//lights.push_back(light);

		//Gpu::PointLight * cornerLight1 = new Gpu::PointLight();
		//cornerLight1->position = glm::vec3(40.0f, 10.0f, 40.0f);
		//cornerLight1->color = glm::vec3(0.5f, 0.5f, 1.0f);
		//lights.push_back(cornerLight1);

		//Gpu::PointLight * cornerLight2 = new Gpu::PointLight();
		//cornerLight2->position = glm::vec3(-40.0f, 10.0f, -40.0f);
		//cornerLight2->color = glm::vec3(1.0f, 0.5f, 0.5f);
		//lights.push_back(cornerLight2);

		shadowSurface = gpu->CreateDrawSurface(2048, 2048, Gpu::DrawSurface::Format_Typeless);

		spriteCamera.isOrthoCamera = true;

		LocalMesh * spriteGeom = GeoBuilder().BuildGrid(1.0f, 1.0f, 2, 2, &Gpu::Rect(0.0f, 1.0f, 1.0f, 0.0f));
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
			ticket = -1;
		}

		cameraAngle = cameraAngle + (secs * 0.2f);
		if(cameraAngle > fullCircle) cameraAngle -= fullCircle;

		camera.position.x = sinf(cameraAngle)*cameraRadius;
		camera.position.z = cosf(cameraAngle)*cameraRadius;

		// Directional Light
		float lightAngle = static_cast<float>(cameraAngle + M_PI_2);
		if(lightAngle > fullCircle) lightAngle -= fullCircle;
		light->direction = glm::vec3(sin(lightAngle),0.5f,cos(lightAngle));

		// MEGA FIXME! Direction needs to be inverted here and in DX11Shaders.cpp, and in every usage!
		lightCamera.position = light->direction * lightRadius;
		
		// Point Light
		//light->z = sinf(cameraAngle) * 25;

		// Spot Light
		//light->position = glm::vec3(camera->position.x,camera->position.y,camera->position.z);
		//light->direction = glm::vec3(-light->position.x,-light->position.y,-light->position.z);

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

		glm::mat4 lightMatrix = texCoordTransform * lightCamera.GetProjMatrix(1.0f) * lightCamera.GetViewMatrix();
		glm::mat4 shadowMatrix;
		Gpu::FloatArray shadowFloatArray((float*)&shadowMatrix, 16);
		shadowEffect->SetParam(0, &shadowFloatArray);
		shadowEffect->SetParam(1, shadowSurface->GetTexture());

		for(unsigned i = 0; i < 7; i++)
		{
			cylinder->position.x = sphere->position.x = -10.0f;
			cylinder->position.z = sphere->position.z = (i - 3.0f) * 10.0f;

			shadowMatrix = lightMatrix * sphere->GetMatrix();
			gpu->DrawGpuModel(sphere, camera, (Gpu::Light**) &light, 1, surface, 0, overrideEffect);

			shadowMatrix = lightMatrix * cylinder->GetMatrix();
			gpu->DrawGpuModel(cylinder, camera, (Gpu::Light**) &light, 1, surface, 0, overrideEffect);

			cylinder->position.x = sphere->position.x = 10.0f;

			shadowMatrix = lightMatrix * sphere->GetMatrix();
			gpu->DrawGpuModel(sphere, camera, (Gpu::Light**) &light, 1, surface, 0, overrideEffect);

			shadowMatrix = lightMatrix * cylinder->GetMatrix();
			gpu->DrawGpuModel(cylinder, camera, (Gpu::Light**) &light, 1, surface, 0, overrideEffect);
		}

		shadowMatrix = lightMatrix * floor->GetMatrix();
		gpu->DrawGpuModel(floor, camera, (Gpu::Light**) &light, 1, surface, 0, overrideEffect);
	}

	virtual void Draw() override
	{
		shadowSurface->Clear();

		// First, draw to the shadow map
		DrawScene(&lightCamera, shadowSurface, 0);

		// Then, draw to the backbuffer
		DrawScene(&camera, 0, shadowEffect);

		gpu->DrawGpuModel(sprite, &spriteCamera, 0, 0);

		//sphere->positionX = light->x; sphere->positionZ = light->z;
		//sphere->positionY = light->y;
		//sphere->setColor(1.0f,1.0f,1.0f,1.0f);
		//gpu->DrawGpuIndexedMesh(sphere, camera, 0, 0);
		//sphere->positionY = 7.5f;
		//sphere->setColor(1.0f,0.0f,0.0f,1.0f);

		gpu->DrawGpuText(font,L"Ingenuity", 0.0f, 0.0f, false);
	}

};

MAIN_WITH(ShadowDemo)