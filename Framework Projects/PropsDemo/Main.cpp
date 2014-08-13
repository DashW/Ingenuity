// Crate Demo - texturing - Richard Copperwaite

#include "pch.h"

#include <RealtimeApp.h>
#include <GeoBuilder.h>
#include <GpuShaders.h>
#include <HeightParser.h>
#include <InputState.h>
#include <WavefrontLoader.h>
#include "SpriteLoadingBar.h"
#define _X86_
#define _USE_MATH_DEFINES
#include <math.h>

using namespace Ingenuity;

const float fullCircle = (float)(M_PI * 2);

// Camera distances
const int 
	CAMERA_CLOSE_UP = 0,
	CAMERA_WALLS = 1,
	CAMERA_CASTLE = 2,
	CAMERA_WORLD = 3,
	CAMERA_DISTANCES = 4;

class HeightmapDemo : public RealtimeApp
{
	HeightParser heightParser;
	Gpu::ComplexModel *castle, *skull;
	Gpu::Model *ground, *water;
	Gpu::Font *font;
	Gpu::DirectionalLight* light;
	Gpu::Camera *camera;
	Gpu::Shader *shader;
	Gpu::CubeMap *cubeMap;
	Gpu::Effect * waterEffect, *terrainEffect;
	SpriteLoadingBar * loadingBar;
	float cameraAngle, cameraRadius, cameraSpeed;
	int cameraDistance;
	int quickLoadTicket, slowLoadTicket;

	void setCameraDistance(int distance)
	{
		switch(distance)
		{
		case CAMERA_WALLS:
			camera->position.y = 0.26f;
			camera->target = glm::vec3(0.03f,0.17f,0.36f);
			cameraRadius = 0.2f;
			break;
		case CAMERA_CASTLE:
			camera->position.y = 0.5f;
			camera->target = glm::vec3(0.03f,0.17f,0.36f);
			cameraRadius = 0.5f;
			break;
		case CAMERA_WORLD:
			camera->position.y = 0.9f;
			camera->target = glm::vec3(0.0f,0.2f,0.0f);
			cameraRadius = 1.5f;
			break;
		default:
			camera->position.y = 0.2f;
			camera->target = glm::vec3(0.03f,0.17f,0.36f);
			cameraRadius = 0.1f;
		}
	}

public:
	virtual void Begin() override
	{
		castle = 0; skull = 0; font = 0; ground = new Gpu::Model(); cubeMap = 0; waterEffect = 0;
		quickLoadTicket = -1; slowLoadTicket = -1; 

		gpu->SetClearColor(0.8f,0.8f,1.0f,1.0f);

		Files::Directory * appDir = files->GetKnownDirectory(Files::AppDir);
		Files::Directory * frameworkDir = files->GetKnownDirectory(Files::FrameworkDir);

		struct HeightmapResponse : public Files::Response
		{
			HeightmapDemo * app;
			HeightmapResponse(HeightmapDemo * app) : app(app) {}
			virtual void Respond() override
			{
				closeOnComplete = true; deleteOnComplete = true;

				if(buffer)
				{
					app->heightParser.ParseHeightmap(buffer, 257);
					app->heightParser.SetScale(2.0f, 0.002f, 2.0f);
					app->ground->mesh = app->heightParser.GetMesh(&Gpu::Rect(0.0f, 0.0f, 1.0f, 1.0f))->GpuOnly(app->gpu);
				}
			}
		};
		files->OpenAndRead(appDir, L"PropsDemo/castlehm257.raw", new HeightmapResponse(this));

		AssetBatch quickAssetBatch;
		quickAssetBatch.emplace_back(appDir, L"PropsDemo/blend_castle.dds", TextureAsset, "blend_castle");
		quickAssetBatch.emplace_back(appDir, L"PropsDemo/grass.dds", TextureAsset, "grass");
		quickAssetBatch.emplace_back(appDir, L"PropsDemo/dirt.dds", TextureAsset, "dirt");
		quickAssetBatch.emplace_back(appDir, L"PropsDemo/rock.dds", TextureAsset, "rock");
		quickAssetBatch.emplace_back(appDir, L"PropsDemo/grassCubeMap.dds", CubeMapAsset, "skycube");
		quickAssetBatch.emplace_back(appDir, L"PropsDemo/wave0.dds", TextureAsset, "wave0");
		quickAssetBatch.emplace_back(appDir, L"PropsDemo/wave1.dds", TextureAsset, "wave1");
		quickAssetBatch.emplace_back(appDir, L"PropsDemo/LoadingRect.png", TextureAsset, "loadingRect");
		quickAssetBatch.emplace_back(frameworkDir, L"MultiTextureAnimY.xml", ShaderAsset, "multitexShader");
		quickAssetBatch.emplace_back(frameworkDir, L"WaterShader.xml", ShaderAsset, "waterShader");
		quickLoadTicket = assets->Load(quickAssetBatch);

		AssetBatch slowAssetBatch;
		slowAssetBatch.emplace_back(appDir, L"PropsDemo/castle.obj", WavefrontModelAsset, "castle");
		slowAssetBatch.emplace_back(appDir, L"PropsDemo/skull3.obj", WavefrontModelAsset, "skull");
		slowLoadTicket = assets->Load(slowAssetBatch);

		water = new Gpu::Model();
		water->mesh = GeoBuilder().BuildGrid(2.0f, 2.0f, 100, 100, &Gpu::Rect(0.0f, 0.0f, 16.0f, 16.0f), true)->GpuOnly(gpu);
		water->color = glm::vec4(0.25f,0.25f,0.5f,0.5f);
		water->position.y = 0.13f;

		font = gpu->CreateGpuFont(40,L"Arial");

		camera = new Gpu::Camera();
		camera->fovOrHeight = (float)(M_PI_4);
		camera->nearClip = 0.001f;
		camera->farClip = 10.0f;
		camera->position.x = 0.0f;
		camera->position.z = 0.5f;
		cameraSpeed = 0.5f;
		cameraAngle = 0.0f;
		cameraDistance = CAMERA_CLOSE_UP;
		setCameraDistance(cameraDistance);

		light = new Gpu::DirectionalLight();
		float lightAngle = static_cast<float>(M_PI-M_PI_4);
		if(lightAngle < 0.0f) lightAngle += fullCircle;
		light->direction = glm::vec3(sinf(lightAngle),1.0f,cosf(lightAngle));

		loadingBar = new SpriteLoadingBar(this);
	}
	virtual void End() override
	{
		delete loadingBar;
		delete ground->mesh;
		//delete ground->texture;
		delete ground->effect;
		delete ground;
		if(castle)
		{
			//for(unsigned i = 0; i < castle->numModels; i++)
			//{
			//	delete castle->models[i].mesh;
			//}
			delete castle; // How do we know we have to delete it?
		}
		//delete castleLoader;
		//if(cubeMap) delete cubeMap;
		if(skull)
		{
			//delete skull->models[0].mesh;
			delete skull;
		}
		//delete skullLoader;
		delete water->mesh;
		delete water->effect;
		delete water;
		delete camera;
		delete light;
		delete font;
	}
	virtual void Update(float secs) override
	{		
		if(quickLoadTicket > -1 && assets->IsLoaded(quickLoadTicket))
		{
			Gpu::Shader * groundShader = assets->GetAsset<Gpu::Shader>("multitexShader");
			if(groundShader)
			{
				terrainEffect = new Gpu::Effect(groundShader);
				terrainEffect->SetParam(0, assets->GetAsset<Gpu::Texture>("grass"));
				terrainEffect->SetParam(1, assets->GetAsset<Gpu::Texture>("dirt"));
				terrainEffect->SetParam(2, assets->GetAsset<Gpu::Texture>("rock"));
				ground->effect = terrainEffect;
			}
			ground->texture = assets->GetAsset<Gpu::Texture>("blend_castle");

			Gpu::Shader * waterShader = assets->GetAsset<Gpu::Shader>("waterShader");
			if(waterShader)
			{
				waterEffect = new Gpu::Effect(waterShader);
				waterEffect->SetParam(0, assets->GetAsset<Gpu::Texture>("wave1"));
				waterEffect->SetParam(1, 0.0f);
				waterEffect->SetParam(2, 0.0f);
				waterEffect->SetParam(3, 0.0f);
				waterEffect->SetParam(4, 0.0f);
				water->effect = waterEffect;
			}
			water->normalMap = assets->GetAsset<Gpu::Texture>("wave0");

			cubeMap = assets->GetAsset<Gpu::CubeMap>("skycube");
			water->cubeMap = cubeMap;

			loadingBar->SetTexture(assets->GetAsset<Gpu::Texture>("loadingRect"));

			quickLoadTicket = -1;
		}

		if(slowLoadTicket > -1 && assets->IsLoaded(slowLoadTicket))
		{
			castle = assets->GetAsset<Gpu::ComplexModel>("castle");
			if(castle)
			{
				castle->position = glm::vec4(0.03f, 0.15f, 0.325f, 1.0f);
				castle->scale = glm::vec4(0.0035f);
			}

			skull = assets->GetAsset<Gpu::ComplexModel>("skull");
			if(skull)
			{
				skull->position = glm::vec4(0.03f, 0.155f, 0.36f, 1.0f);
				skull->scale = glm::vec4(0.007f);
				skull->models[0].color.r = 0.0f;
				skull->models[0].color.g = 0.0f;
				skull->models[0].color.b = 0.0f;
				skull->models[0].specPower = 1024.f;
				skull->models[0].cubeMap = cubeMap;
			}

			slowLoadTicket = -1;
		}

		loadingBar->Update();

		if(waterEffect)
		{
			float flowspeed = secs * 0.05f;

			float & offsetX1 = waterEffect->params[1]->fvalue;
			float & offsetY1 = waterEffect->params[2]->fvalue;
			float & offsetX2 = waterEffect->params[3]->fvalue;
			float & offsetY2 = waterEffect->params[4]->fvalue;

			offsetX1 += flowspeed;
			if(offsetX1 > 1.0f) offsetX1 -= 1.0f;
			offsetX2 -= flowspeed;
			if(offsetX2 < 0.0f) offsetX2 += 1.0f;
			offsetY1 += flowspeed;
			if(offsetY1 > 1.0f) offsetY1 -= 1.0f;
			offsetY2 += flowspeed;
			if(offsetY2 > 1.0f) offsetY2 -= 1.0f;
		}

		//KeyState keyboard = input->PollKeyboard();
		MouseState & mouse = input->GetMouseState();

		//const float camSpeed = 0.1f;
		//const float camHeight = 0.02f;

		//if(keyboard.keys['W'])
		//{
		//	camera->x += (sinf(cameraAngle) * camSpeed * secs);
		//	camera->z += (-cosf(cameraAngle) * camSpeed * secs);
		//}
		//if(keyboard.keys['S'])
		//{
		//	camera->x -= (sinf(cameraAngle) * camSpeed * secs);
		//	camera->z -= (-cosf(cameraAngle) * camSpeed * secs);
		//}
		//if(keyboard.keys['A'])
		//{
		//	camera->x += (sinf(cameraAngle + float(M_PI_2)) * camSpeed * secs);
		//	camera->z += (-cosf(cameraAngle + float(M_PI_2)) * camSpeed * secs);
		//}
		//if(keyboard.keys['D'])
		//{
		//	camera->x -= (sinf(cameraAngle + float(M_PI_2)) * camSpeed * secs);
		//	camera->z -= (-cosf(cameraAngle + float(M_PI_2)) * camSpeed * secs);
		//}
		//if(keyboard.keys[KeyState::LEFT])
		//{
		//	cameraAngle += secs;
		//}
		//if(keyboard.keys[KeyState::RIGHT])
		//{
		//	cameraAngle -= secs;
		//}
		//camera->y = heightParser.GetHeight(camera->x,camera->z) + camHeight;

		//float dx = float(mouse.x) - float(mouse.prevX);
		//cameraAngle += (0.1f * dx * secs);
		//camera->targetX = camera->x + sinf(cameraAngle);
		//camera->targetZ = camera->z - cosf(cameraAngle);

		if(mouse.leftUp)
		{
			cameraDistance = cameraDistance + 1;
			if(cameraDistance >= CAMERA_DISTANCES) cameraDistance = 0;
			setCameraDistance(cameraDistance);
		}

		cameraAngle = cameraAngle + (secs * cameraSpeed);
		if(cameraAngle > fullCircle) cameraAngle -= fullCircle;

		camera->position.x = camera->target.x + sin(cameraAngle)*cameraRadius;
		camera->position.z = camera->target.z + cos(cameraAngle)*cameraRadius;

	}
	virtual void Draw() override
	{
		gpu->DrawGpuModel(ground, camera, (Gpu::Light**) &light, 1);

		if(castle)
		{
			castle->BeDrawn(gpu, camera, (Gpu::Light**) &light, 1);
		}

		if(skull)
		{
			skull->BeDrawn(gpu, camera, (Gpu::Light**) &light, 1);
		}

		gpu->DrawGpuModel(water, camera, (Gpu::Light**) &light, 1);

		loadingBar->BeDrawn(gpu);

		gpu->DrawGpuText(font,L"Direct3D", 0.0f, 0.0f, false);
	}
};

MAIN_WITH(HeightmapDemo)