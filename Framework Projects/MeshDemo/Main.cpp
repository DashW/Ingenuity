// Mesh-loading demo - Richard Copperwaite

#include "pch.h"
#include <RealtimeApp.h>
#include <GeoBuilder.h>
#include <InputState.h>
#include <WavefrontLoader.h>
//#include "SpriteLoadingBar.h"
#define _X86_
#define _USE_MATH_DEFINES
#include <math.h>

using namespace Ingenuity;

const float fullCircle = (float)(M_PI * 2);

class MeshDemo : public RealtimeApp
{
	Gpu::Font * font;
	Gpu::ComplexModel * model;
	Gpu::Model * floor, * debugModel;
	Gpu::DirectionalLight * light;
	Gpu::Camera * camera;
	Gpu::Texture * floorTex, *floorNormal; // , *modelNormal;
	Gpu::CubeMap * cubeMap;
	//SpriteLoadingBar * loadingBar;
	float modelScale, modelPos;
	float cameraRadius, cameraAngle;
	unsigned assetTicket;

public:
	virtual void Begin() override
	{
		font = 0; model = 0; floor = 0; debugModel = 0; light = 0; camera = 0; assetTicket = -1;
		floorTex = 0; floorNormal = 0; /*modelNormal = 0;*/ cubeMap = 0; /*loadingBar = 0;*/

		gpu->SetClearColor(1.0f, 1.0f, 1.0f, 1.0f);

		font = gpu->CreateGpuFont(40,L"Arial");

		modelScale = 8.0f; modelPos = 7.0f;

		Files::Directory * appDir = files->GetKnownDirectory(Files::AppDir);
		AssetBatch batch;
		batch.emplace_back(appDir, L"MeshDemo/Paramedic_Bag.obj", WavefrontModelAsset); // CONSOLIDATE VERTICES!!!
		batch.emplace_back(appDir, L"MeshDemo/grassCubeMap.dds", CubeMapAsset);
		batch.emplace_back(appDir, L"MeshDemo/face.jpg", TextureAsset);
		batch.emplace_back(appDir, L"MeshDemo/face_nmap.jpg", TextureAsset);
		//batch.emplace_back(appDir, L"MeshDemo/LoadingRect.png", TextureAsset);
		assetTicket = assets->Load(batch);

		//skullLoader = new WavefrontLoader(steppables,gpu,assets,L"MeshDemo/Paramedic_Bag.obj"); // skull3.obj
		//skullLoader->SetConsolidate(true);
		//skullLoader->SetGenerateDebugMesh(false);
		//steppables->Add(skullLoader);

		floor = new Gpu::Model();
		//floor->mesh = gpu->CreateGrid(20.0f,20.0f,2,2,&GpuRect(0.0f,0.0f,5.0f,5.0f),true);
		floor->mesh = GeoBuilder().BuildGrid(20.0f,20.0f,2,2,&Gpu::Rect(0.0f,0.0f,5.0f,5.0f),true)->GpuOnly(gpu);
		floor->texture = floorTex;
		floor->normalMap = floorNormal;
		floor->color.r = 0.5f;
		floor->color.g = 0.5f;
		floor->color.b = 0.5f;
		//floor->wireframe = true;

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

		//loadingBar = new SpriteLoadingBar(this);
	}

	virtual void End() override
	{
		//delete cubeMap;
		if(model)
		{
			//for(unsigned i = 0; i < model->numModels; i++)
			//	delete model->models[i].mesh;
			delete model;
		}
		//if(debugModel)
		//{
		//	delete debugModel->mesh;
		//	delete debugModel;
		//}
		//delete modelNormal;
		//delete loadingBar;
		//delete floorTex;
		//delete floorNormal;
		delete floor->mesh;
		delete floor;
		delete light;
		delete camera;
		delete font;
	}

	virtual void Update(float secs) override
	{
		KeyState& keys = input->GetKeyState();

		if(assets->IsLoaded(assetTicket))
		{
			Files::Directory * appDir = files->GetKnownDirectory(Files::AppDir);

			cubeMap = assets->GetAsset<Gpu::CubeMap>(appDir, L"MeshDemo/grassCubeMap.dds");
			floor->texture = floorTex = assets->GetAsset<Gpu::Texture>(appDir, L"MeshDemo/face.jpg");
			floor->normalMap = floorNormal = assets->GetAsset<Gpu::Texture>(appDir, L"MeshDemo/face_nmap.jpg");

			model = assets->GetAsset<Gpu::ComplexModel>(appDir, L"MeshDemo/Paramedic_Bag.obj");
			if(model)
			{
				model->scale = glm::vec3(modelScale);
				model->position.y = modelPos;
				//model->wireframe = true;
				//model->models[0].backFaceCull = false;
				model->models[0].cubeMap = cubeMap;
			}

			//Gpu::Mesh * debugMesh = skullLoader->GetDebugMesh();
			//if(debugMesh)
			//{
			//	debugModel = new Gpu::Model();
			//	debugModel->mesh = debugMesh;
			//	debugModel->wireframe = true;
			//	debugModel->scale = glm::vec3(modelScale);
			//	debugModel->position.y = modelPos;
			//	debugModel->backFaceCull = false;
			//}

			assetTicket = -1;
		}

		//loadingBar->Update();

		cameraAngle = cameraAngle + (secs/2.0f);
		if(cameraAngle > fullCircle) cameraAngle -= fullCircle;

		if(keys.downKeys[0x39]) // SPACE
		{
			floor->wireframe = !floor->wireframe;
			model->wireframe = !model->wireframe;
		}

		//camera->x = sin(cameraAngle)*cameraRadius;
		//camera->z = cos(cameraAngle)*cameraRadius;
		
		//float lightAngle = static_cast<float>(/*cameraAngle*/-0.5f);
		//if(lightAngle < 0.0f) lightAngle += fullCircle;
		
		//light->SetDirection(sin(lightAngle),0.25f,cos(lightAngle));

		if(model)
		{
			model->rotation.y = cameraAngle + float(M_PI);
		}
		if(debugModel)
		{
			debugModel->rotation.y = cameraAngle + float(M_PI);
		}
		floor->rotation.y = cameraAngle;
	}

	virtual void Draw() override
	{
		if(model)
		{
			model->BeDrawn(gpu, camera, (Gpu::Light**) &light, 1);
		}
		if(debugModel)
		{
			gpu->DrawGpuModel(debugModel, camera, 0, 0);
		}

		gpu->DrawGpuModel(floor, camera, (Gpu::Light**) &light, 1);

		//loadingBar->BeDrawn(gpu);

		gpu->DrawGpuText(font,L"Direct3D", 0.0f, 0.0f, false);
	}

};

MAIN_WITH(MeshDemo)