#define _X86_
#define _USE_MATH_DEFINES

#include <RealtimeApp.h>
#include <GeoBuilder.h>
#include <GpuApi.h>
#include <InputState.h>
#include <math.h>
#include <sstream>

#include "GlareDefD3D.h"

using namespace Ingenuity;

const float fullCircle = (float) M_PI * 2.0;

class FlyCamera : public Gpu::Camera
{
	float yAngle;
	float upAngle;
	float sensitivity;
	float flySpeed;

public:
	FlyCamera() : yAngle(0.0f), upAngle(0.0f), sensitivity(0.01f), flySpeed(10.0f) {}
	virtual ~FlyCamera() {}
	void Update(float secs, InputState * input)
	{
		MouseState & mouseState = input->GetMouseState();
		KeyState & keyState = input->GetKeyState();

		if(mouseState.left)
		{
			yAngle += (mouseState.dX * sensitivity);
			if(yAngle > fullCircle) yAngle -= fullCircle;
			if(yAngle < 0.0f) yAngle += fullCircle;

			upAngle -= (mouseState.dY * sensitivity);
			if(upAngle >= float(M_PI_2))
				upAngle = float(M_PI_2) - 0.01f;
			if(upAngle <= -float(M_PI_2))
				upAngle = -float(M_PI_2) + 0.01f;
		}

		glm::vec3 direction = target - position;
		if(keyState.keys[0x11]) // W
		{
			position += (direction * secs * flySpeed);
		}
		if(keyState.keys[0x1f]) // S
		{
			position -= (direction * secs * flySpeed);
		}
		if(keyState.keys[0x1e]) // A
		{
			position += (glm::normalize(glm::cross(direction, up)) * secs * flySpeed);
		}
		if(keyState.keys[0x20]) // D
		{
			position -= (glm::normalize(glm::cross(direction, up)) * secs * flySpeed);
		}

		target.x = position.x + (sinf(yAngle) * cosf(upAngle));
		target.y = position.y + (sinf(upAngle));
		target.z = position.z + (cosf(yAngle) * cosf(upAngle));
	}
};

class HDRLightDemo : public RealtimeApp
{
	static const unsigned NUM_LUM_DOWNSAMPLES = 5;
	static const unsigned NUM_STAR_SURFACES = 12;

	Gpu::Model wall1;
	Gpu::Model wall2;
	Gpu::Model wall3;
	Gpu::Model wall4;
	Gpu::Model floor;
	Gpu::Model ceil;
	Gpu::Model column1;
	Gpu::Model column2;
	Gpu::Model column3;
	Gpu::Model column4;
	Gpu::Model painting1;
	Gpu::Model painting2;
	Gpu::Model sphere;
	Gpu::Model sphere2;

	Gpu::Sprite surfaceSprite; // fixme!!
	Gpu::DrawSurface * surface1;
	Gpu::DrawSurface * surface2;
	Gpu::DrawSurface * lumDownsamples[NUM_LUM_DOWNSAMPLES];
	Gpu::DrawSurface * eyeAdaptationPrevious;
	Gpu::DrawSurface * eyeAdaptationTarget;
	Gpu::DrawSurface * toneMappingTarget;
	Gpu::DrawSurface * brightPassTarget;
	Gpu::DrawSurface * blurTarget;
	Gpu::DrawSurface * bloomSource;
	Gpu::DrawSurface * starSurfaces[NUM_STAR_SURFACES];
	Gpu::DrawSurface * bloomSourceBlurred;
	Gpu::DrawSurface * bloomTempTarget;
	Gpu::DrawSurface * bloomTarget;

	Gpu::Effect * downSampleEffect = 0;
	Gpu::Effect * logLuminanceEffect = 0;
	Gpu::Effect * eyeAdaptationEffect = 0;
	Gpu::Effect * toneMappingEffect = 0;
	Gpu::Effect * brightPassEffect = 0;
	Gpu::Effect * blurEffect = 0;
	Gpu::Effect * bloomEffect = 0;
	Gpu::Effect * multiplyEffect = 0;

	Gpu::PointLight * lights[2];

	Gpu::Font * debugFont = 0;

	FlyCamera camera;

	CGlareDef glareDef;

	float deltaTime;

	unsigned texTicket;

	wchar_t frameTimeText[100];

public:
	virtual void Begin() override
	{
		GeoBuilder geoBuilder;
		Gpu::Rect texRect = Gpu::Rect(0.0f, 0.0f, 7.0f, 2.0f);

		AssetBatch texBatch;
		Files::Directory * appDir = files->GetKnownDirectory(Files::AppDir);
		Files::Directory * frameworkDir = files->GetKnownDirectory(Files::FrameworkDir);
		texBatch.emplace_back(appDir, L"env2.bmp", TextureAsset);
		texBatch.emplace_back(appDir, L"env3.bmp", TextureAsset);
		texBatch.emplace_back(appDir, L"ground2.bmp", TextureAsset);
		texBatch.emplace_back(appDir, L"seafloor.bmp", TextureAsset);
		texBatch.emplace_back(frameworkDir, L"DownSample.xml", ShaderAsset);
		texBatch.emplace_back(frameworkDir, L"LogLuminance.xml", ShaderAsset);
		texBatch.emplace_back(frameworkDir, L"EyeAdaptation.xml", ShaderAsset);
		texBatch.emplace_back(frameworkDir, L"ToneMapping.xml", ShaderAsset);
		texBatch.emplace_back(frameworkDir, L"BrightPassFilter.xml", ShaderAsset);
		texBatch.emplace_back(frameworkDir, L"BlurShader.xml", ShaderAsset);
		texBatch.emplace_back(frameworkDir, L"BloomShader.xml", ShaderAsset);
		texBatch.emplace_back(frameworkDir, L"TextureMultiply.xml", ShaderAsset);
		texTicket = assets->Load(texBatch);

		wall1.mesh = geoBuilder.BuildGrid(15.0f, 3.0f, 2, 2, &texRect)->GpuOnly(gpu);
		wall1.rotation.x = static_cast<float>(M_PI_2);
		wall1.position.z = -10.0f;
		wall1.specPower = 5.0f;
		wall1.diffuseFactor = 0.5f;

		wall2.mesh = geoBuilder.BuildGrid(20.0f, 3.0f, 2, 2, &texRect)->GpuOnly(gpu);
		wall2.rotation.y = static_cast<float>(M_PI_2);
		wall2.rotation.x = static_cast<float>(-M_PI_2);
		wall2.position.x = 7.50f;
		wall2.specPower = 5.0f;
		wall2.diffuseFactor = 0.5f;

		wall3.mesh = geoBuilder.BuildGrid(15.0f, 3.0f, 2, 2, &texRect)->GpuOnly(gpu);
		wall3.rotation.x = static_cast<float>(-M_PI_2);
		wall3.position.z = 10.0f;
		wall3.specPower = 5.0f;
		wall3.diffuseFactor = 0.5f;

		wall4.mesh = geoBuilder.BuildGrid(20.0f, 3.0f, 2, 2, &texRect)->GpuOnly(gpu);
		wall4.rotation.y = static_cast<float>(M_PI_2);
		wall4.rotation.x = static_cast<float>(M_PI_2);
		wall4.position.x = -7.50f;
		wall4.specPower = 5.0f;
		wall4.diffuseFactor = 0.5f;

		floor.mesh = geoBuilder.BuildGrid(15.0f, 20.0f, 2, 2, &Gpu::Rect(0.0f, 0.0f, 8.0f, 8.0f))->GpuOnly(gpu);
		floor.position.y = -1.50f;
		floor.specPower = 50.0f;
		floor.specFactor = 3.0f;

		ceil.mesh = geoBuilder.BuildGrid(15.0f, 20.0f, 2, 2, &Gpu::Rect(0.0f, 0.0f, 6.0f, 6.0f))->GpuOnly(gpu);
		ceil.position.y = 1.50f;
		ceil.rotation.x = static_cast<float>(M_PI);
		ceil.specPower = 5.0f;
		ceil.specFactor = 0.3f;
		ceil.diffuseFactor = 0.3f;

		column1.mesh = geoBuilder.BuildCube()->GpuOnly(gpu);
		column1.scale.x = 0.325f;
		column1.scale.y = 1.5f;
		column1.scale.z = 0.325f;
		column1.position = glm::vec3(3.5f, 0.0f, 3.0f);
		column1.specPower = 5.0f;
		column1.diffuseFactor = 0.5f;

		column2 = column1;
		column2.position = glm::vec3(-3.5f, 0.0f, 3.0f);

		column3 = column1;
		column3.position = glm::vec3(3.5f, 0.0f, -3.0f);

		column4 = column1;
		column4.position = glm::vec3(-3.5f, 0.0f, -3.0f);

		painting1.mesh = geoBuilder.BuildGrid(3.5f, 2.5f, 2, 2, &Gpu::Rect(0.0f, 1.0f, 1.0f, 0.0f))->GpuOnly(gpu);
		painting1.rotation.x = static_cast<float>(-M_PI_2);
		painting1.position.x = -3.5f;
		painting1.position.z = 9.8f;
		painting1.specPower = 5.0f;
		painting1.specFactor = 0.3f;

		painting2 = painting1;
		painting2.position.x = 3.5f;

		camera.position = glm::vec3(0.0f, 0.4f, -9.0f);
		camera.target = glm::vec3(0.0f, 0.0f, 0.0f);
		camera.nearClip = 0.2f;
		camera.farClip = 30.0f;
		camera.fovOrHeight = float(M_PI) / 4.0f;

		lights[0] = new Gpu::PointLight();
		lights[0]->color = glm::vec3(80.0f, 80.0f, 80.0f);
		lights[0]->position = glm::vec3(-3.5f, 0.5f, 8.0f);
		lights[0]->atten = 0.0f;

		lights[1] = new Gpu::PointLight();
		lights[1]->color = glm::vec3(8.0f, 8.0f, 8.0f);
		lights[1]->position = glm::vec3(3.5f, 0.5f, 8.0f);
		lights[1]->atten = 0.0f;

		sphere.mesh = geoBuilder.BuildSphere(0.1f, 20, 20)->GpuOnly(gpu);
		sphere.position = lights[0]->position;
		sphere.color = glm::vec4(lights[0]->color * 40.0f, 1.0f);

		sphere2 = sphere;
		sphere2.position = lights[1]->position;
		sphere2.color = glm::vec4(lights[1]->color * 40.0f, 1.0f);

		surface1 = gpu->CreateScreenDrawSurface(1.0f, 1.0f, Gpu::DrawSurface::Format_4x16float);
		surfaceSprite.pixelSpace = true;

		surface2 = gpu->CreateScreenDrawSurface(0.5f, 0.5f, Gpu::DrawSurface::Format_4x16float);

		// WHY DOES CREATING TOO MANY TEXTURES CAUSE THE APP TO CRASH UNDER INTEL DRIVERS???

		for(unsigned i = 0; i < NUM_LUM_DOWNSAMPLES; ++i)
		{
			unsigned sampleTexWidth = 1 << (2 * i);

			lumDownsamples[i] = gpu->CreateDrawSurface(sampleTexWidth, sampleTexWidth, Gpu::DrawSurface::Format_1x16float);
		}

		eyeAdaptationPrevious = gpu->CreateDrawSurface(1, 1, Gpu::DrawSurface::Format_1x16float);
		eyeAdaptationTarget = gpu->CreateDrawSurface(1, 1, Gpu::DrawSurface::Format_1x16float);

		toneMappingTarget = gpu->CreateScreenDrawSurface(1.0f, 1.0f, Gpu::DrawSurface::Format_4x16float);

		brightPassTarget = gpu->CreateScreenDrawSurface(1.0f/2.0f, 1.0f/2.0f, Gpu::DrawSurface::Format_4x16float);
		blurTarget = gpu->CreateScreenDrawSurface(1.0f/2.0f, 1.0f/2.0f, Gpu::DrawSurface::Format_4x16float);

		for(unsigned i = 0; i < NUM_STAR_SURFACES; ++i)
		{
			starSurfaces[i] = gpu->CreateScreenDrawSurface(1.0f/4.0f, 1.0f/4.0f, Gpu::DrawSurface::Format_4x16float);
		}

		bloomSource = gpu->CreateScreenDrawSurface(1.0f / 8.0f, 1.0f / 8.0f, Gpu::DrawSurface::Format_4x16float);
		bloomSourceBlurred = gpu->CreateScreenDrawSurface(1.0f / 8.0f, 1.0f / 8.0f, Gpu::DrawSurface::Format_4x16float);
		bloomTempTarget = gpu->CreateScreenDrawSurface(1.0f / 8.0f, 1.0f / 8.0f, Gpu::DrawSurface::Format_4x16float);
		bloomTarget = gpu->CreateScreenDrawSurface(1.0f / 8.0f, 1.0f / 8.0f, Gpu::DrawSurface::Format_4x16float);

		glareDef.Initialize(GLT_FILTER_CROSSSCREEN);

		debugFont = gpu->CreateGpuFont(30, L"Arial");
	}
	virtual void End() override
	{
		delete wall1.mesh;
		delete wall2.mesh;
		delete wall3.mesh;
		delete wall4.mesh;
		delete floor.mesh;
		delete ceil.mesh;
		delete sphere.mesh;
		delete column1.mesh;
		delete painting1.mesh;
		delete lights[0];
		delete lights[1];

		delete debugFont;

		delete surface1;
		delete surface2;
		for(unsigned i = 0; i < NUM_LUM_DOWNSAMPLES; ++i)
		{
			delete lumDownsamples[i];
		}
		delete eyeAdaptationPrevious;
		delete eyeAdaptationTarget;
		delete toneMappingTarget;
		delete brightPassTarget;
		delete blurTarget;
		for(unsigned i = 0; i < NUM_STAR_SURFACES; ++i)
		{
			delete starSurfaces[i];
		}
		delete bloomSource;
		delete bloomSourceBlurred;
		delete bloomTempTarget;
		delete bloomTarget;

		if(downSampleEffect) delete downSampleEffect;
		if(logLuminanceEffect) delete logLuminanceEffect;
		if(eyeAdaptationEffect) delete eyeAdaptationEffect;
		if(toneMappingEffect) delete toneMappingEffect;
		if(brightPassEffect) delete brightPassEffect;
		if(blurEffect) delete blurEffect;
		if(bloomEffect) delete bloomEffect;
		if(multiplyEffect) delete multiplyEffect;
	}
	virtual void Update(float secs) override
	{
		deltaTime = secs;

		if(assets->IsLoaded(texTicket))
		{
			Files::Directory * appDir = files->GetKnownDirectory(Files::AppDir);
			Files::Directory * frameworkDir = files->GetKnownDirectory(Files::FrameworkDir);
			Gpu::Texture * wallTex = assets->GetAsset<Gpu::Texture>(appDir, L"env2.bmp");
			Gpu::Texture * paintingTex = assets->GetAsset<Gpu::Texture>(appDir, L"env3.bmp");
			Gpu::Texture * floorTex = assets->GetAsset<Gpu::Texture>(appDir, L"ground2.bmp");
			Gpu::Texture * ceilTex = assets->GetAsset<Gpu::Texture>(appDir, L"seafloor.bmp");

			wall1.texture = wallTex;
			wall2.texture = wallTex;
			wall3.texture = wallTex;
			wall4.texture = wallTex;
			column1.texture = wallTex;
			column2.texture = wallTex;
			column3.texture = wallTex;
			column4.texture = wallTex;
			floor.texture = floorTex;
			ceil.texture = ceilTex;
			painting1.texture = paintingTex;
			painting2.texture = paintingTex;

			Gpu::Shader * downSample = assets->GetAsset<Gpu::Shader>(frameworkDir, L"DownSample.xml");
			Gpu::Shader * logLuminance = assets->GetAsset<Gpu::Shader>(frameworkDir, L"LogLuminance.xml");
			Gpu::Shader * eyeAdaptation = assets->GetAsset<Gpu::Shader>(frameworkDir, L"EyeAdaptation.xml");
			Gpu::Shader * toneMapping = assets->GetAsset<Gpu::Shader>(frameworkDir, L"ToneMapping.xml");
			Gpu::Shader * brightPass = assets->GetAsset<Gpu::Shader>(frameworkDir, L"BrightPassFilter.xml");
			Gpu::Shader * blurShader = assets->GetAsset<Gpu::Shader>(frameworkDir, L"BlurShader.xml");
			Gpu::Shader * bloomShader = assets->GetAsset<Gpu::Shader>(frameworkDir, L"BloomShader.xml");
			Gpu::Shader * multiplyShader = assets->GetAsset<Gpu::Shader>(frameworkDir, L"TextureMultiply.xml");

			if(downSample) downSampleEffect = new Gpu::Effect(downSample);
			if(logLuminance) logLuminanceEffect = new Gpu::Effect(logLuminance);
			if(eyeAdaptation) eyeAdaptationEffect = new Gpu::Effect(eyeAdaptation);
			if(toneMapping) toneMappingEffect = new Gpu::Effect(toneMapping);
			if(brightPass) brightPassEffect = new Gpu::Effect(brightPass);
			if(blurShader) blurEffect = new Gpu::Effect(blurShader);
			if(bloomShader) bloomEffect = new Gpu::Effect(bloomShader);
			if(multiplyShader) multiplyEffect = new Gpu::Effect(multiplyShader);

			texTicket = -1;

			gpu->SetAnisotropy(16);
		}

		camera.Update(secs, input);

		_snwprintf_s(frameTimeText, 100, L"Frame Time: %3.1fms", secs * 1000.0f);
	}

	//-----------------------------------------------------------------------------
	// Name: GaussianDistribution
	// Desc: Helper function for GetSampleOffsets function to compute the 
	//       2 parameter Gaussian distrubution using the given standard deviation
	//       rho
	//-----------------------------------------------------------------------------
	float GaussianDistribution(float x, float y, float rho)
	{
		float g = 1.0f / sqrtf(2.0f * float(M_PI) * rho * rho);
		g *= expf(-(x * x + y * y) / (2 * rho * rho));

		return g;
	}

	void GetBloomParameters(float texSize, glm::vec4 * sampleOffsets, glm::vec4 * sampleWeights, float deviation, float multiplier)
	{
		int i = 0;
		float tu = 1.0f / texSize;

		// Fill the center texel
		float weight = 2.0f * GaussianDistribution(0, 0, 3.0f);
		sampleOffsets[0] = glm::vec4(0.0f);
		sampleWeights[0] = glm::vec4(weight, weight, weight, 1.0f);

		// Fill the first half
		for(i = 1; i < 8; i++)
		{
			// Get the Gaussian intensity for this offset
			weight = 2.0f * GaussianDistribution((float)i, 0, 3.0f);
			sampleOffsets[i] = glm::vec4(i * tu);
			sampleWeights[i] = glm::vec4(weight, weight, weight, 1.0f);
		}

		// Mirror to the second half
		for(i = 8; i < 15; i++)
		{
			sampleWeights[i] = sampleWeights[i - 7];
			sampleOffsets[i] = -sampleOffsets[i - 7];
		}
	}

	void DrawTimestamp(const wchar_t * name, float y)
	{
		Gpu::TimestampData data = gpu->GetTimestampData(name);
		std::wstringstream stream;
		stream << name << L": ";
		stream << unsigned(data.data[Gpu::TimestampData::DrawCalls]) << " calls, ";
		stream << (data.data[Gpu::TimestampData::Time] * 1000.0f) << "ms";
		gpu->DrawGpuText(debugFont, stream.str().c_str(), 0.0f, y, false);
	}

	virtual void Draw() override
	{
		surface1->Clear();
		surface2->Clear();
		toneMappingTarget->Clear();
		brightPassTarget->Clear();
		blurTarget->Clear();
		starSurfaces[0]->Clear();

		gpu->SetBlendMode(Gpu::BlendMode_Alpha);

#pragma region Scene

		// Draw Scene to HDR render target

		gpu->BeginTimestamp(L"scene");

		gpu->DrawGpuModel(&wall1, &camera, (Gpu::Light**)lights, 2, surface1);
		gpu->DrawGpuModel(&wall2, &camera, (Gpu::Light**)lights, 2, surface1);
		gpu->DrawGpuModel(&wall3, &camera, (Gpu::Light**)lights, 2, surface1);
		gpu->DrawGpuModel(&wall4, &camera, (Gpu::Light**)lights, 2, surface1);
		gpu->DrawGpuModel(&floor, &camera, (Gpu::Light**)lights, 2, surface1);
		gpu->DrawGpuModel(&ceil, &camera, (Gpu::Light**)lights, 2, surface1);
		gpu->DrawGpuModel(&column1, &camera, (Gpu::Light**)lights, 2, surface1);
		gpu->DrawGpuModel(&column2, &camera, (Gpu::Light**)lights, 2, surface1);
		gpu->DrawGpuModel(&column3, &camera, (Gpu::Light**)lights, 2, surface1);
		gpu->DrawGpuModel(&column4, &camera, (Gpu::Light**)lights, 2, surface1);
		gpu->DrawGpuModel(&painting1, &camera, (Gpu::Light**)lights, 2, surface1);
		gpu->DrawGpuModel(&painting2, &camera, (Gpu::Light**)lights, 2, surface1);
		gpu->DrawGpuModel(&sphere, &camera, 0, 0, surface1);
		gpu->DrawGpuModel(&sphere2, &camera, 0, 0, surface1);

		gpu->EndTimestamp(L"scene");

#pragma endregion // Draw Scene to HDR render target

#pragma region Tonemapping

		// 1. Measure Luminance.

		gpu->BeginTimestamp(L"tonemapping");

		if(downSampleEffect && logLuminanceEffect)
		{
			unsigned curLumSampleLevel = NUM_LUM_DOWNSAMPLES - 1;

			lumDownsamples[curLumSampleLevel]->Clear();
			gpu->DrawGpuSurface(surface1, logLuminanceEffect, lumDownsamples[curLumSampleLevel]);

			while(curLumSampleLevel > 0)
			{
				lumDownsamples[curLumSampleLevel - 1]->Clear();
				gpu->DrawGpuSurface(lumDownsamples[curLumSampleLevel], downSampleEffect, lumDownsamples[curLumSampleLevel - 1]);

				curLumSampleLevel--;
			}
		}

		// 2. Calculate Eye Adaptation

		if(eyeAdaptationEffect)
		{
			Gpu::DrawSurface * tempSurface = eyeAdaptationPrevious;
			eyeAdaptationPrevious = eyeAdaptationTarget;
			eyeAdaptationTarget = tempSurface;
			eyeAdaptationTarget->Clear();

			eyeAdaptationEffect->SetParam(0, eyeAdaptationPrevious->GetTexture());
			eyeAdaptationEffect->SetParam(1, deltaTime);

			gpu->DrawGpuSurface(lumDownsamples[0], eyeAdaptationEffect, eyeAdaptationTarget);
		}

		// 3. Tone-Map

		// Render the scene to a new surface, with the tone mapping parameters

		if(toneMappingEffect)
		{
			toneMappingEffect->SetParam(0, eyeAdaptationTarget->GetTexture());

			gpu->DrawGpuSurface(surface1, toneMappingEffect, toneMappingTarget);
		}

		gpu->EndTimestamp(L"tonemapping");

#pragma endregion // Perform Tone Mapping

#pragma region Bright Pass

		// 4. Down Sample

		gpu->BeginTimestamp(L"brightpass");

		if(downSampleEffect)
		{
			// NEED TO CLAMP THIS TEXTURE SAMPLER
			gpu->DrawGpuSurface(toneMappingTarget, downSampleEffect, surface2);
		}

		// 5. Bright-Pass Filter.

		if(brightPassEffect)
		{
			//brightPassEffect->SetParam(0, 20.0f);
			//brightPassEffect->SetParam(1, 30.0f);

			gpu->DrawGpuSurface(surface2, brightPassEffect, brightPassTarget);
		}

		// 6. Gaussian Blur.

		if(blurEffect)
		{
			gpu->DrawGpuSurface(brightPassTarget, blurEffect, blurTarget);
		}

		gpu->EndTimestamp(L"brightpass");

#pragma endregion // Perform Bright Pass Filtering

#pragma region Star

		// 7. Star.

		gpu->BeginTimestamp(L"star");

		if(bloomEffect && multiplyEffect)
		{
			const CStarDef& starDef = glareDef.m_starDef;
			const float tanFOV = atanf(float(M_PI) / 8.0f); // tangent field of view ???
			static const int s_maxPasses = 3;
			static const int nSamples = 8;
			static glm::vec4 s_aaColor[s_maxPasses][8];
			static const glm::vec4 s_colorWhite(0.63f, 0.63f, 0.63f, 0.0f);

			glm::vec4 avSampleWeights[16];
			glm::vec4 avSampleOffsets[16];

			float srcW = float(blurTarget->GetTexture()->GetWidth());
			float srcH = float(blurTarget->GetTexture()->GetHeight());

			for(int p = 0; p < s_maxPasses; p++)
			{
				float ratio;
				ratio = (float)(p + 1) / (float)s_maxPasses;

				for(int s = 0; s < nSamples; s++)
				{
					glm::vec4 chromaticAberrColor;

					// interpolate the chromatic aberration colour to white with the pass ratio

					glm::vec4 sampleAberrColor = CStarDef::GetChromaticAberrationColor(s);
					chromaticAberrColor = sampleAberrColor + ((s_colorWhite - sampleAberrColor) * ratio);

					// interpolate the aaColor(?) from white to the chromatic aberration colour with the glare definition

					s_aaColor[p][s] = s_colorWhite + ((chromaticAberrColor - s_colorWhite) * glareDef.m_fChromaticAberration);
				}
			}

			float radOffset;
			radOffset = glareDef.m_fStarInclination + starDef.m_fInclination;

			Gpu::DrawSurface * pTexSource;
			Gpu::DrawSurface * pSurfDest;

			// Direction loop
			for(int d = 0; d < starDef.m_nStarLines; d++)
			{
				CONST STARLINE& starLine = starDef.m_pStarLine[d];

				pTexSource = blurTarget;

				float rad = radOffset + starLine.fInclination;
				float sn = sinf(rad);
				float cs = cosf(rad);
				glm::vec2 vtStepUV;
				vtStepUV.x = sn / srcW * starLine.fSampleLength;
				vtStepUV.y = cs / srcH * starLine.fSampleLength;

				float attnPowScale = (tanFOV + 0.1f) * 1.0f *
					(160.0f + 120.0f) / (srcW + srcH) * 1.2f;

				// 1 direction expansion loop
				// disable alpha blending!
				//g_pd3dDevice->SetRenderState(D3DRS_ALPHABLENDENABLE, FALSE);

				int iWorkTexture = 1;
				for(int p = 0; p < starLine.nPasses; p++)
				{
					if(p == starLine.nPasses - 1)
					{
						// Last pass move to other work buffer
						pSurfDest = starSurfaces[d + 4];
					}
					else
					{
						pSurfDest = starSurfaces[iWorkTexture];
					}

					// Sampling configration for each stage
					for(int i = 0; i < nSamples; i++)
					{
						float lum;
						lum = powf(starLine.fAttenuation, attnPowScale * i);

						avSampleWeights[i] = s_aaColor[starLine.nPasses - 1 - p][i] *
							lum * (p + 1.0f) * 0.5f;

						// Offset of sampling coordinate
						avSampleOffsets[i].x = vtStepUV.x * i;
						avSampleOffsets[i].y = vtStepUV.y * i;
						if(fabs(avSampleOffsets[i].x) >= 0.9f ||
							fabs(avSampleOffsets[i].y) >= 0.9f)
						{
							avSampleOffsets[i].x = 0.0f;
							avSampleOffsets[i].y = 0.0f;
							avSampleWeights[i] *= 0.0f;
						}

					}

					// This is the remaining piece of the puzzle - 
					// to set the sample offsets and weights to the effect!

					Gpu::FloatArray gpuSampleOffsets((float*)&avSampleOffsets, 16 * 4);
					Gpu::FloatArray gpuSampleWeights((float*)&avSampleWeights, 16 * 4);

					bloomEffect->SetParam(0, &gpuSampleOffsets);
					bloomEffect->SetParam(1, &gpuSampleWeights);
					bloomEffect->SetParam(2, 8.0f);
					//bloomEffect->SetSamplerParam(Gpu::SamplerParam::Filter, Gpu::SamplerParam::FilterPoint);
					//bloomEffect->SetSamplerParam(Gpu::SamplerParam::Anisotropy, 0);

					gpu->DrawGpuSurface(pTexSource, bloomEffect, pSurfDest);

					// Setup next expansion
					vtStepUV *= nSamples;
					attnPowScale *= nSamples;

					// Set the work drawn just before to next texture source.
					pTexSource = starSurfaces[iWorkTexture];

					iWorkTexture += 1;
					if(iWorkTexture > 2)
					{
						iWorkTexture = 1;
					}

				}
			}

			// Now merge all the textures together
			
			// Note: Instead of using multiple draw calls, 
			// a much faster way would be to draw the scene to an atlas
			// and merge the atlassed texture.

			// Alternatively, since you already know the number of source textures, in the star effect
			// you could draw directly to the target texture with the weighting as an alpha value?

			multiplyEffect->SetParam(0, 1.0f);
			multiplyEffect->SetParam(1, 1.0f);
			multiplyEffect->SetParam(2, 1.0f);
			multiplyEffect->SetParam(3, 1.0f / float(starDef.m_nStarLines));

			for(int i = 0; i < starDef.m_nStarLines; ++i)
			{
				gpu->DrawGpuSurface(starSurfaces[i + 4], multiplyEffect, starSurfaces[0]);
			}
		}

		gpu->EndTimestamp(L"star");

#pragma endregion // Generate Star Effect

#pragma region Bloom

		// 8. Bloom.

		gpu->BeginTimestamp(L"bloom");

		if(downSampleEffect && bloomEffect)
		{
			gpu->DrawGpuSurface(blurTarget, downSampleEffect, bloomSource);

			// Clear the bloom texture
			bloomTarget->Clear();

			if(glareDef.m_fGlareLuminance > 0.0f &&
				(glareDef.m_fBloomLuminance > 0.0f))
			{
				gpu->DrawGpuSurface(bloomSource, blurEffect, bloomSourceBlurred);

				//g_pd3dDevice->SetRenderState(D3DRS_SCISSORTESTENABLE, FALSE);

				glm::vec4 sampleOffsets[16];
				glm::vec4 sampleWeights[16];

				Gpu::FloatArray gpuSampleOffsets((float*)&sampleOffsets, 16 * 4);
				Gpu::FloatArray gpuSampleWeights((float*)&sampleWeights, 16 * 4);

				bloomEffect->SetParam(0, &gpuSampleOffsets);
				bloomEffect->SetParam(1, &gpuSampleWeights);
				bloomEffect->SetParam(2, 16.0f);

				GetBloomParameters(float(bloomSourceBlurred->GetTexture()->GetWidth()), sampleOffsets, sampleWeights, 3.0f, 2.0f);

				for(int i = 0; i < 16; i++)
				{
					sampleOffsets[i] = glm::vec4(sampleOffsets[i].x, 0.0f, 0.0f, 0.0f);
				}

				gpu->DrawGpuSurface(bloomSourceBlurred, bloomEffect, bloomTempTarget);

				//g_pd3dDevice->SetRenderState(D3DRS_SCISSORTESTENABLE, FALSE);

				GetBloomParameters(float(bloomSourceBlurred->GetTexture()->GetHeight()), sampleOffsets, sampleWeights, 3.0f, 2.0f);

				for(int i = 0; i < 16; i++)
				{
					sampleOffsets[i] = glm::vec4(0.0f, sampleOffsets[i].y, 0.0f, 0.0f);
				}

				gpu->DrawGpuSurface(bloomTempTarget, bloomEffect, bloomTarget);
			}
		}

		gpu->EndTimestamp(L"bloom");

#pragma endregion // Generate Bloom Effect

		// Now to plop the star and bloom textures onto the tone map target.
		// Texture copy with an additive blend??? Need to apply a multiplier first!
		// bloomtex = 1.0f
		// startex = 0.5f
		// could this be done with alpha values? Might be a bit cheeky...

		gpu->SetBlendMode(Gpu::BlendMode_Additive);

		if(multiplyEffect)
		{
			multiplyEffect->SetParam(0, 0.5f);
			multiplyEffect->SetParam(1, 0.5f);
			multiplyEffect->SetParam(2, 0.5f);
			multiplyEffect->SetParam(3, 1.0f);

			gpu->DrawGpuSurface(bloomTarget, multiplyEffect, toneMappingTarget);

			multiplyEffect->SetParam(0, 1.0f);
			multiplyEffect->SetParam(1, 1.0f);
			multiplyEffect->SetParam(2, 1.0f);
			multiplyEffect->SetParam(3, 1.0f);

			gpu->DrawGpuSurface(starSurfaces[0], multiplyEffect, toneMappingTarget);
		}

		surfaceSprite.texture = toneMappingTarget->GetTexture(); // FIXME!!
		surfaceSprite.position.x = 0.0f;
		surfaceSprite.position.z = 1.0f;
		surfaceSprite.scale.x = 1.0f;
		surfaceSprite.scale.y = 1.0f;
		gpu->DrawGpuSprite(&surfaceSprite);

		gpu->DrawGpuText(debugFont, frameTimeText, 0.0f, 0.0f, false);
		
		DrawTimestamp(L"scene", 40);
		DrawTimestamp(L"tonemapping", 80);
		DrawTimestamp(L"brightpass", 120);
		DrawTimestamp(L"star", 160);
		DrawTimestamp(L"bloom", 200);
	}
};

MAIN_WITH(HDRLightDemo)