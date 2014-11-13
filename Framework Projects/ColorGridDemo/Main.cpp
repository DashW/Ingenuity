// Cube Demo - 3d rendering - Richard Copperwaite

#include "pch.h"
#define _X86_
#define _USE_MATH_DEFINES

#include <RealtimeApp.h>
#include <GeoBuilder.h>
#include <GpuApi.h>
#include <InputState.h>
#include <math.h>
#include <sstream>

using namespace Ingenuity;

class ColorGridDemo : public RealtimeApp
{
	Gpu::Model* cell;
	Gpu::Font* font;
	Gpu::Camera camera;
	float cameraY, cameraRadius, cameraAngle, fullCircle;
	Gpu::Rect textPos;
	GeoBuilder * builder;
	unsigned mouseX, mouseY;
	Gpu::InstanceBuffer * instanceBuffer;
	wchar_t drawCallText[100];
	static const unsigned NUM_INSTANCES = 200000;
	Instance_PosCol localInstances[NUM_INSTANCES];

	int assetTicket;

public:
	virtual void Begin() override
	{
		font = gpu->CreateGpuFont(40,L"Arial");

 		builder = new GeoBuilder();

		Gpu::Mesh * mesh = builder->BuildRect(0,0,100,100)->GpuOnly(gpu,true);

		cell = new Gpu::Model();
		cell->mesh = mesh;
		cell->backFaceCull = false;
		cell->color = glm::vec4(1.0f,0.0f,0.0f,1.0f);

		Files::Directory * frameworkDir = files->GetKnownDirectory(Files::FrameworkDir);
		assetTicket = assets->Load(frameworkDir, L"PathShader.xml", ShaderAsset, "PathShader");

		camera.isOrthoCamera = true;
		cameraRadius = 8.0f;
		cameraAngle = 0.0f;

		textPos.top = textPos.left = 0.0f;
		textPos.right = 200.0f;
		textPos.bottom = 40.0f;

		fullCircle = (float)(M_PI * 2);

		for(unsigned i = 0; i < NUM_INSTANCES; ++i)
		{
			localInstances[i].position = glm::vec3(0.0f,0.0f,0.0f);
			localInstances[i].color = glm::vec4(0.0f,0.0f,0.0f,1.0f);
		}

		instanceBuffer = gpu->CreateInstanceBuffer(NUM_INSTANCES, localInstances, InstanceType_PosCol);
	}

	virtual void End() override
	{
		if(cell)
		{
			if(cell->mesh) delete cell->mesh;
			if(cell->effect) delete cell->effect;
			delete cell;
		}
		delete instanceBuffer;
		delete font;
		delete builder;
	}

	virtual void Update(float secs) override
	{
		if(assets->IsLoaded(assetTicket))
		{
			Gpu::Shader * shader = assets->GetAsset<Gpu::Shader>("PathShader");
			if(shader)
			{
				cell->effect = new Gpu::Effect(shader);
			}
			assetTicket = -1;
		}

		unsigned screenWidth, screenHeight;
		gpu->GetBackbufferSize(screenWidth, screenHeight);

		//cameraAngle = cameraAngle + secs;
		//if(cameraAngle > fullCircle) cameraAngle -= fullCircle;
		camera.nearClip = 1.0f;
		camera.farClip = 500.0f;
		camera.fovOrHeight = float(screenHeight);
		camera.position.x = float(screenWidth) / 2.0f;
		camera.position.y = -float(screenHeight) / 2.0f;
		camera.position.z = -100;

		camera.target.x = float(screenWidth) / 2.0f;
		camera.target.y = -float(screenHeight) / 2.0f;

		MouseState mouse = input->GetMouseState();

		unsigned numDrawCalls = unsigned(ceilf(float(screenWidth)/float(mouse.x)) * ceilf(float(screenHeight)/float(mouse.y)));

		if(numDrawCalls < NUM_INSTANCES && mouse.x > 0 && mouse.y > 0)
		{
			mouseX = mouse.x;
			mouseY = mouse.y;
		}

		LocalMesh * updatedCell = builder->BuildRect(0.0f,0.0f,float(mouseX),float(mouseY));
		gpu->UpdateDynamicMesh(cell->mesh,updatedCell->vertexBuffer);
		gpu->UpdateDynamicMesh(cell->mesh, updatedCell->numTriangles, updatedCell->indexBuffer);
		delete updatedCell;

		_snwprintf_s(drawCallText, 100, L"Frame Time: %3.1fms  Num Draw Calls: %d", secs * 1000.0f, numDrawCalls);

		//camera.x = sin(cameraAngle)*cameraRadius;
		//camera.z = cos(cameraAngle)*cameraRadius;
	}

	virtual void Draw() override
	{	
		unsigned screenWidth, screenHeight;
		gpu->GetBackbufferSize(screenWidth, screenHeight);

		unsigned columns = unsigned(ceilf(float(screenWidth) / float(mouseX)));

		for(unsigned i = 0; i < NUM_INSTANCES; ++i)
		{
			Instance_PosCol & instance = localInstances[i];
			instance.position.x = float((i % columns) * mouseX);
			instance.position.y = float((i / columns) * mouseY);
			instance.color.r = 1.0f - (instance.position.x / float(screenWidth));
			instance.color.g = (instance.position.y / float(screenHeight));
		}

		gpu->UpdateInstanceBuffer(instanceBuffer,NUM_INSTANCES,localInstances);

		gpu->DrawGpuModel(cell, &camera, 0, 0, 0, instanceBuffer);
		gpu->DrawGpuText(font,L"Direct3D", 0.0f, 0.0f, false);
		gpu->DrawGpuText(font,drawCallText,0.0f,40.0f,false);
	}
};

MAIN_WITH(ColorGridDemo)