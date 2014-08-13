// Scene Demo - rendering with one draw call - Richard Copperwaite

#include "pch.h"

#include <RealtimeApp.h>
#include <WavefrontLoader.h>
#include <GpuScene.h>
#include <GeoBuilder.h>
#define _X86_
#define _USE_MATH_DEFINES
#include <math.h>

using namespace Ingenuity;

const float fullCircle = (float)(M_PI * 2);

class SceneDemo : public RealtimeApp
{
	Gpu::Font * font;

	Gpu::Model * lightModel;
	Gpu::Model * floorModel;

	Gpu::Model * sphereModels;
	Gpu::Model * vaseModels;

	Gpu::Mesh * vaseMesh;
	Gpu::Mesh * floorMesh;
	Gpu::Mesh * sphereMesh;

	//GpuDirectionalLight* light;
	Gpu::PointLight * light;
	//GpuSpotLight* light;

	Gpu::Camera * camera;
	float cameraRadius, cameraAngle;

	Gpu::Scene * scene;

	int vaseTicket;

	wchar_t drawCallText[100];

	static const unsigned NUM_VASES = 4096;

public:

	virtual void Begin() override
	{
		sphereModels = 0; vaseModels = 0; vaseMesh = 0; floorMesh = 0; sphereMesh = 0; vaseTicket = -1;

		scene = new Gpu::InstancedScene<Instance_Pos>(gpu);

		gpu->SetClearColor(0.5f,0.5f,0.5f,1.0f);

		font = gpu->CreateGpuFont(40,L"Arial");
		GeoBuilder geoBuilder;
		floorMesh = geoBuilder.BuildGrid(640.0f,640.0f,2,2)->GpuOnly(gpu); 
		sphereMesh = geoBuilder.BuildSphere(1.0f, 20, 20)->GpuOnly(gpu); // IMPROVE

		Files::Directory * directory = files->GetKnownDirectory(Files::AppDir);

		vaseTicket = assets->Load(directory, L"SceneDemo//vase.obj", WavefrontModelAsset, "vase");

		//sphereModels = new GpuModel[14];

		//for(unsigned i = 0; i < 14; i++)
		//{
		//	sphereModels[i].mesh = sphereMesh;
		//	sphereModels[i].position.x = (i%2 ? 10.0f : -10.0f);
		//	sphereModels[i].position.y = 9.0f;
		//	sphereModels[i].position.z = ((i/2)-3.0f) * 10.0f;
		//	scene->Add(&sphereModels[i]);
		//}

		floorModel = new Gpu::Model();
		floorModel->mesh = floorMesh;
		floorModel->specPower = FLT_MAX;
		scene->Add(floorModel);

		camera = new Gpu::Camera();
		cameraRadius = 140.0f;
		camera->position.y = 40.0f;
		cameraAngle = 0.0f;
		camera->fovOrHeight = (float)(M_PI_4);
		scene->SetCamera(camera);

		//light = new GpuDirectionalLight();

		light = new Gpu::PointLight();
		light->position = glm::vec3(0.0f, 3.0f, 0.0f);
		light->atten = 0.001f;

		//light = new GpuSpotLight();
		//light->power = 32.0f;

		scene->Add(light);

		lightModel = new Gpu::Model();
		lightModel->mesh = sphereMesh;
		lightModel->position = glm::vec4(light->position, 1.0f);

		scene->SetDirty();
	}

	virtual void End() override
	{
		if(vaseModels)
		{
			delete[] vaseModels;
		}
		//delete[] sphereModels;

		//if(vaseMesh) delete vaseMesh;
		delete floorMesh;
		delete sphereMesh;

		delete lightModel;
		delete floorModel;

		delete camera;
		delete light;
		delete font;

		delete scene;
	}

	virtual void Update(float secs) override
	{
		if(vaseTicket > -1 && assets->IsLoaded(vaseTicket))
		{
			Gpu::ComplexModel * vaseModel = assets->GetAsset<Gpu::ComplexModel>("vase");

			if(vaseModel->numModels > 0)
			{
				vaseMesh = vaseModel->models[0].mesh;
				Gpu::Texture * vaseTexture = vaseModel->models[0].texture;

				vaseModels = new Gpu::Model[NUM_VASES];

				unsigned numRows = 64;
				unsigned numCols = 64;

				float offset = (float(numRows) - 1.0f) / 2.0f;

				for(unsigned i = 0; i < NUM_VASES; i++)
				{
					vaseModels[i].mesh = vaseMesh;
					vaseModels[i].texture = vaseTexture;
					vaseModels[i].position.x = ((i % (numCols / 2)) + 1) * (i % numCols < numCols / 2 ? 10.0f : -10.0f);
					vaseModels[i].position.z = ((i / numCols) - offset) * 10.0f;
					vaseModels[i].scale = glm::vec4(0.08f);
					scene->Add(&vaseModels[i]);
				}
			}

			delete vaseModel;

			vaseTicket = -1;
		}

		cameraAngle = cameraAngle + (secs * 0.5f);
		if(cameraAngle > fullCircle) cameraAngle -= fullCircle;

		camera->position.x = sinf(cameraAngle)*cameraRadius;
		camera->position.z = cosf(cameraAngle)*cameraRadius;

		_snwprintf_s(drawCallText, 100, L"Frame Time: %3.1fms  Num Vases: %d", secs * 1000.0f, NUM_VASES);
	}

	virtual void Draw() override
	{
		gpu->Draw(scene);

		gpu->DrawGpuModel(lightModel,camera,0,0);

		gpu->DrawGpuText(font,L"Direct3D", 0.0f, 0.0f, false);
		gpu->DrawGpuText(font,drawCallText,0.0f,40.0f,false);
	}

};

MAIN_WITH(SceneDemo)