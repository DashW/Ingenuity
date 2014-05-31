// Mirror Demo - stencil buffering - Richard Copperwaite

#include "pch.h"
#include <RealtimeApp.h>
#include <GeoBuilder.h>

#define _USE_MATH_DEFINES

#include <math.h>

using namespace Ingenuity;

Gpu::Font* hellofont;
Gpu::Model *tiles, *wall, *mirror, *ball;
Gpu::Camera* camera;
Gpu::DirectionalLight* light;
float cameraAngle;
int loadTicket;

const float fullCircle = (float)(M_PI * 2);

void RealtimeApp::Begin()
{
	tiles = new Gpu::Model();
	wall = new Gpu::Model();
	mirror = new Gpu::Model();
	ball = new Gpu::Model();
	loadTicket = -1;

	gpu->SetClearColor(1.0f, 1.0f, 1.0f, 1.0f);

	hellofont = gpu->CreateGpuFont(40, L"Arial");

	Files::Directory * appDir = files->GetKnownDirectory(Files::AppDir);

	AssetBatch assetBatch;
	assetBatch.emplace_back(appDir, L"checkboard.dds", TextureAsset, "checkboard");
	assetBatch.emplace_back(appDir, L"brick1.dds", TextureAsset, "brick");
	assetBatch.emplace_back(appDir, L"ice.dds", TextureAsset, "ice");
	loadTicket = assets->Load(assetBatch);

	GeoBuilder geo;

	tiles->mesh = geo.BuildGrid(10.0f,10.0f,2,2,&Gpu::Rect(0.0f,0.0f,4.0f,4.0f))->GpuOnly(gpu);

	wall->mesh = geo.BuildGrid(10.0f,5.0f,2,2,&Gpu::Rect(0.0f,0.0f,1.0f,1.0f))->GpuOnly(gpu);
	wall->rotation.x = static_cast<float>(M_PI_2);
	wall->position.y = 2.5f;
	wall->position.z = -5.f;

	mirror->mesh = geo.BuildGrid(5.0f,5.0f,2,2,&Gpu::Rect(0.0f,0.0f,1.0f,1.0f))->GpuOnly(gpu);
	mirror->rotation.x = static_cast<float>(M_PI_2);
	mirror->position.y = 2.5f;
	mirror->position.z = -4.5f;

	light = new Gpu::DirectionalLight();
	light->direction = glm::vec3(-1.0f,1.0f,1.0f);
	
	ball->mesh = geo.BuildSphere(1.0f,30,30)->GpuOnly(gpu);
	ball->color.g = 0.0f;
	ball->color.b = 0.0f;
	ball->position.y = 2.0f;
	ball->backFaceCull = true;

	camera = new Gpu::Camera();
	camera->position.x = 0.0f;
	camera->position.y = 4.0f;
	camera->position.z = 10.0f;
	cameraAngle = 0.0f;
	camera->fovOrHeight = (float)(M_PI_4);
	camera->target.y = 1.0f;
}

void RealtimeApp::End()
{
	delete tiles->mesh;
	delete tiles;

	delete wall->mesh;
	delete wall;

	delete mirror->mesh;
	delete mirror;

	delete ball->mesh;
	delete ball;

	delete light;
	delete camera;
	delete hellofont;
}

void RealtimeApp::Update(float secs)
{
	if(loadTicket > -1 && assets->IsLoaded(loadTicket))
	{
		tiles->texture = assets->GetAsset<Gpu::Texture>("checkboard");
		wall->texture = assets->GetAsset<Gpu::Texture>("brick");
		mirror->texture = assets->GetAsset<Gpu::Texture>("ice");
		loadTicket = -1;
	}

	cameraAngle = cameraAngle + secs;
	if(cameraAngle > fullCircle) cameraAngle -= fullCircle;

	camera->position.x = sinf(cameraAngle)*10.0f;
}

void RealtimeApp::Draw()
{	
	gpu->DrawGpuModel(mirror, camera, 0, 0);

	gpu->DrawGpuModel(mirror, camera, 0, 0, gpu->GetStencilSurface());

	ball->position.z = -9.0f;
	tiles->position.z = -9.0f;
	light->direction.z = -light->direction.z;
	gpu->DrawGpuModel(tiles, camera, 0, 0, gpu->GetStencilClipSurface());
	gpu->DrawGpuModel(ball, camera, (Gpu::Light**) &light, 1, gpu->GetStencilClipSurface());
	light->direction.z = -light->direction.z;
	ball->position.z = 0.0f;
	tiles->position.z = -0.0f;

	gpu->DrawGpuModel(wall, camera, 0, 0);

	gpu->DrawGpuModel(tiles, camera, 0, 0);

	gpu->DrawGpuModel(ball, camera, (Gpu::Light**) &light, 1);

	gpu->DrawGpuText(hellofont, L"Direct3D", 0.0f, 0.0f, false);
}

MAIN()