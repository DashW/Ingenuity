#include "ScriptCallbacks.h"
#include "ScriptInterpreter.h"
#include "AssetMgr.h"
#include "GeoBuilder.h"
#include "GpuScene.h"
#include "SvgParser.h"
#include "ImageApi.h"
#include "InputState.h"
#include "AudioApi.h"
#include "IngenuityHelper.h"
#include <HeightParser.h>
#include <sstream>
#include <vector>

namespace Ingenuity {

std::wstring ScriptCallbacks::subPathString;

Files::Directory * ScriptCallbacks::GetDirectory(Files::Api * files, const char * name)
{
	if(!name) return 0;

	Files::Directory * directoryPtr = 0;
	if(strcmp(name, "FrameworkDir") == 0)
	{
		directoryPtr = files->GetKnownDirectory(Files::FrameworkDir);
	}
	if(strcmp(name, "InstallDir") == 0)
	{
		directoryPtr = files->GetKnownDirectory(Files::AppDir);
	}
	if(strcmp(name, "TempDir") == 0)
	{
		directoryPtr = files->GetKnownDirectory(Files::TempDir);
	}
	if(strcmp(name, "ProjectDir") == 0)
	{
		directoryPtr = IngenuityHelper::GetDataDirectory();
		directoryPtr->isProjectDir = true;
	}

	return directoryPtr;
}

void ScriptCallbacks::LoadAsset(
	AssetMgr * assets,
	AssetBatch & batch,
	const char * directory, 
	const char * file, 
	const char * type, 
	const char * name)
{
	Files::Directory * directoryPtr = GetDirectory(assets->GetFileApi(), directory);

	AssetType assetType = TextureAsset;
	if(type)
	{
		if(strcmp(type, "CubeMap") == 0)
		{
			assetType = CubeMapAsset;
		}
		if(strcmp(type, "VolumeTex") == 0)
		{
			assetType = VolumeTexAsset;
		}
		if(strcmp(type, "WavefrontModel") == 0)
		{
			assetType = WavefrontModelAsset;
		}
		if(strcmp(type, "RawHeightMap") == 0)
		{
			assetType = RawHeightMapAsset;
		}
		if(strcmp(type, "SVGModel") == 0)
		{
			assetType = SvgModelAsset;
		}
		if(strcmp(type, "Shader") == 0)
		{
			assetType = ShaderAsset;
		}
		if(strcmp(type, "WavAudio") == 0)
		{
			assetType = AudioAsset;
		}
	}

	std::string shortFile(file);
	std::wstring wideFile(shortFile.begin(), shortFile.end());

	if(directoryPtr && directoryPtr->isProjectDir)
	{
		wideFile = subPathString + wideFile;
	}

	batch.emplace_back(directoryPtr, wideFile.c_str(), assetType, name);
}

void ScriptCallbacks::ClearConsole(ScriptInterpreter * interpreter)
{
	interpreter->GetLogger().Clear();
}

void ScriptCallbacks::Help(ScriptInterpreter * interpreter)
{
	for(unsigned i = 0; i < interpreter->callbacks.size(); i++)
	{
		interpreter->GetLogger().Log("%s - %s\n",
			interpreter->callbacks[i].name,
			interpreter->callbacks[i].help);
	}
}

void ScriptCallbacks::Require(ScriptInterpreter * interpreter)
{
	struct RequireResponse : public Files::Response
	{
		ScriptInterpreter * interpreter;
		std::string filename;

		RequireResponse(ScriptInterpreter * i, std::wstring filename) : interpreter(i)
		{
			this->filename.assign(filename.begin(), filename.end());
		}

		virtual void Respond() override
		{
			closeOnComplete = true; deleteOnComplete = true;

			if(buffer)
			{
				interpreter->LoadScript(buffer, bufferLength, filename.c_str());
			}
			else
			{
				interpreter->ThrowError("Could not load required package, error reading file");
			}

			interpreter->decDependencies();
		}
	};

	POP_PARAM(1, directory, STRING);
	POP_PARAM(2, path, STRING);

	interpreter->incDependencies();

	Files::Api * files = interpreter->GetApp()->files;

	Files::Directory * dir = GetDirectory(files, directory.svalue);
	std::string spath(path.svalue);
	std::wstring wpath(spath.begin(), spath.end());

	if(dir->isProjectDir)
	{
		wpath = subPathString + wpath;
	}
	
	files->OpenAndRead(dir, wpath.c_str(), new RequireResponse(interpreter, wpath));
}

void ScriptCallbacks::DrawSprite(ScriptInterpreter * interpreter)
{
	POP_PTRPARAM(1,texture,GpuTexture);
	POP_NUMPARAM(2,x);
	POP_NUMPARAM(3,y);
	ScriptParam size = interpreter->PopParam();

	Gpu::Sprite sprite;
	sprite.position.x = (float) x.nvalue;
	sprite.position.y = (float) y.nvalue;
	sprite.texture = (Gpu::Texture*) texture.pvalue->ptr;

	if(size.type == ScriptParam::DOUBLE || size.type == ScriptParam::FLOAT) 
	{
		sprite.size = float(size.nvalue);
	}

	interpreter->GetApp()->gpu->DrawGpuSprite(&sprite);
}

void ScriptCallbacks::CreateCamera(ScriptInterpreter * interpreter)
{
	ScriptParam orthographic = interpreter->PopParam();

	Gpu::Camera * camera = new Gpu::Camera();

	if(orthographic.type == ScriptParam::BOOL)
	{
		camera->isOrthoCamera = orthographic.nvalue > 0.0;
	}

	if(camera) interpreter->PushParam(ScriptParam(camera,ScriptPtrType::GpuCamera));
}

void ScriptCallbacks::SetCameraPosition(ScriptInterpreter * interpreter)
{
	POP_PTRPARAM(1,camera,GpuCamera);
	POP_NUMPARAM(2,x);
	POP_NUMPARAM(3,y);
	POP_NUMPARAM(4,z);

	Gpu::Camera * gpuCamera = static_cast<Gpu::Camera*>(camera.pvalue->ptr);

	if(!gpuCamera) return;

	gpuCamera->position.x = (float) x.nvalue;
	gpuCamera->position.y = (float) y.nvalue;
	gpuCamera->position.z = (float) z.nvalue;
}

void ScriptCallbacks::SetCameraTarget(ScriptInterpreter * interpreter)
{
	POP_PTRPARAM(1,camera,GpuCamera);
	POP_NUMPARAM(2,x);
	POP_NUMPARAM(3,y);
	POP_NUMPARAM(4,z);

	Gpu::Camera * gpuCamera = static_cast<Gpu::Camera*>(camera.pvalue->ptr);

	if(!gpuCamera) return;

	gpuCamera->target.x = (float) x.nvalue;
	gpuCamera->target.y = (float) y.nvalue;
	gpuCamera->target.z = (float) z.nvalue;
}

void ScriptCallbacks::SetCameraClipFovOrHeight(ScriptInterpreter * interpreter)
{
	POP_PTRPARAM(1,camera,GpuCamera);
	POP_NUMPARAM(2,nearclip);
	POP_NUMPARAM(3,farclip);
	POP_NUMPARAM(4,fovOrHeight);

	Gpu::Camera * gpuCamera = static_cast<Gpu::Camera*>(camera.pvalue->ptr);
	if(!gpuCamera) return;

	gpuCamera->nearClip = (float) nearclip.nvalue;
	gpuCamera->farClip = (float) farclip.nvalue;
	gpuCamera->fovOrHeight = (float) fovOrHeight.nvalue;
}

void ScriptCallbacks::CreateSphere(ScriptInterpreter * interpreter)
{
	LocalMesh * localSphere = GeoBuilder().BuildSphere(0.5f,50,50);
	Gpu::Mesh * gpuSphere = localSphere->GpuOnly(interpreter->GetApp()->gpu);
	Gpu::ComplexModel * gpuModel = new Gpu::ComplexModel(1);
	gpuModel->models[0].mesh = gpuSphere;
	gpuModel->models[0].destructMesh = true;

	interpreter->PushParam(ScriptParam(gpuModel,ScriptPtrType::GpuComplexModel));
}

void ScriptCallbacks::DrawModel(ScriptInterpreter * interpreter)
{
	POP_PTRPARAM(1,model,GpuComplexModel);
	POP_PTRPARAM(2,camera,GpuCamera);
	ScriptParam lights = interpreter->PopParam();
	ScriptParam surface = interpreter->PopParam();

	Gpu::ComplexModel * gpuModel = (Gpu::ComplexModel*) model.pvalue->ptr;
	Gpu::Camera * gpuCamera = (Gpu::Camera*) camera.pvalue->ptr;
	std::vector<Gpu::Light*> gpuLights;
	Gpu::DrawSurface * gpuSurface = 0;

	if(lights.type == ScriptParam::MAPREF)
	{
		ScriptParam index; // NONE
		ScriptParam result;

		while(interpreter->GetMapNext(index,index,result))
		{
			if(result.type != ScriptParam::POINTER) break;
			gpuLights.push_back((Gpu::Light*) result.pvalue->ptr);
		}
	}
	if(lights.CheckPointer(ScriptPtrType::GpuLight))
	{
		gpuLights.push_back((Gpu::Light*) lights.pvalue->ptr);
	}

	if(surface.CheckPointer(ScriptPtrType::GpuDrawSurface))
	{
		gpuSurface = static_cast<Gpu::DrawSurface*>(surface.pvalue->ptr);
	}

	if(gpuModel)
	{
		Gpu::Api * gpu = interpreter->GetApp()->gpu;
		gpuModel->BeDrawn(gpu,gpuCamera,gpuLights.data(),gpuLights.size(),gpuSurface);
	}
}

void ScriptCallbacks::SetModelPosition(ScriptInterpreter * interpreter)
{
	POP_PTRPARAM(1,model,GpuComplexModel);
	POP_NUMPARAM(2,x);
	POP_NUMPARAM(3,y);
	POP_NUMPARAM(4,z);

	Gpu::ComplexModel * gpuModel = static_cast<Gpu::ComplexModel*>(model.pvalue->ptr);

	if(!gpuModel) return;

	gpuModel->position.x = (float) x.nvalue;
	gpuModel->position.y = (float) y.nvalue;
	gpuModel->position.z = (float) z.nvalue;
}

void ScriptCallbacks::SetModelRotation(ScriptInterpreter * interpreter)
{
	POP_PTRPARAM(1,model,GpuComplexModel);
	POP_NUMPARAM(2,x);
	POP_NUMPARAM(3,y);
	POP_NUMPARAM(4,z);

	Gpu::ComplexModel * gpuModel = static_cast<Gpu::ComplexModel*>(model.pvalue->ptr);

	if(!gpuModel) return;

	gpuModel->rotation.x = (float) x.nvalue;
	gpuModel->rotation.y = (float) y.nvalue;
	gpuModel->rotation.z = (float) z.nvalue;
}

void ScriptCallbacks::SetModelScale(ScriptInterpreter * interpreter)
{
	POP_PTRPARAM(1,model,GpuComplexModel);
	POP_NUMPARAM(2,scale);
	ScriptParam scaleY = interpreter->PopParam();
	ScriptParam scaleZ = interpreter->PopParam();

	Gpu::ComplexModel * gpuModel = static_cast<Gpu::ComplexModel*>(model.pvalue->ptr);

	if((scaleY.type == ScriptParam::FLOAT || scaleY.type == ScriptParam::DOUBLE)
		&& (scaleZ.type == ScriptParam::FLOAT || scaleZ.type == ScriptParam::DOUBLE))
	{
		gpuModel->scale.x = (float)scale.nvalue;
		gpuModel->scale.y = (float)scaleY.nvalue;
		gpuModel->scale.z = (float)scaleZ.nvalue;
	}
	else
	{
		gpuModel->scale = glm::vec3((float)scale.nvalue);
	}
}

void ScriptCallbacks::GetFont(ScriptInterpreter * interpreter)
{
	POP_NUMPARAM(1,size);
	POP_PARAM(2,family,STRING);
	ScriptParam style = interpreter->PopParam();
	ScriptParam isPixelSpace = interpreter->PopParam();

	Gpu::FontStyle styleValue = Gpu::FontStyle_Regular;
	if(style.type == ScriptParam::STRING)
	{
		if(_stricmp(style.svalue,"italic") == 0)
			styleValue = Gpu::FontStyle_Italic;
		if(_stricmp(style.svalue,"bold") == 0)
			styleValue = Gpu::FontStyle_Bold;
	}

	std::string shortFamily(family.svalue);
	std::wstring wideFamily(shortFamily.begin(),shortFamily.end());

	Gpu::Api * gpu = interpreter->GetApp()->gpu;
	Gpu::Font * font = gpu->CreateGpuFont((int)size.nvalue,wideFamily.c_str(),styleValue);

	if(isPixelSpace.type == ScriptParam::BOOL || isPixelSpace.type == ScriptParam::INT)
	{
		font->pixelSpace = isPixelSpace.nvalue > 0;
	}

	if(font) interpreter->PushParam(ScriptParam(font,ScriptPtrType::GpuFont));
}

void ScriptCallbacks::DrawText(ScriptInterpreter * interpreter)
{
	POP_PTRPARAM(1,font,GpuFont);
	POP_PARAM(2,text,STRING);
	POP_NUMPARAM(3,x);
	POP_NUMPARAM(4,y);
	ScriptParam center = interpreter->PopParam();
	bool centered = false;

	if(center.type != ScriptParam::NONE) centered = center.nvalue > 0.0;

	Gpu::Font * gpuFont = static_cast<Gpu::Font*>(font.pvalue->ptr);

	std::string shortText(text.svalue);
	std::wstring wideText(shortText.begin(),shortText.end());

	Gpu::Api * gpu = interpreter->GetApp()->gpu;
	gpu->DrawGpuText(gpuFont,wideText.c_str(),(float)x.nvalue,(float)y.nvalue,centered);
}

void ScriptCallbacks::SetFontColor(ScriptInterpreter * interpreter)
{
	POP_PTRPARAM(1,font,GpuFont);
	POP_NUMPARAM(2,r);
	POP_NUMPARAM(3,g);
	POP_NUMPARAM(4,b);
	ScriptParam a = interpreter->PopParam();

	Gpu::Font * gpuFont = static_cast<Gpu::Font*>(font.pvalue->ptr);

	gpuFont->color.r = float(r.nvalue);
	gpuFont->color.g = float(g.nvalue);
	gpuFont->color.b = float(b.nvalue);
	if(a.type == ScriptParam::DOUBLE) gpuFont->color.a = float(a.nvalue);
}

void ScriptCallbacks::CreateLight(ScriptInterpreter * interpreter)
{
	POP_PARAM(1,type,STRING);

	Gpu::LightType gpuType = Gpu::LightType_Directional;
	Gpu::Light * result;

	if(_stricmp(type.svalue,"point") == 0) gpuType = Gpu::LightType_Point;
	if(_stricmp(type.svalue,"spot") == 0)  gpuType = Gpu::LightType_Spot;
	
	switch(gpuType)
	{
	case Gpu::LightType_Point:
		result = new Gpu::PointLight();
		break;
	case Gpu::LightType_Spot:
		result = new Gpu::SpotLight();
		break;
	default:
		result = new Gpu::DirectionalLight();
	}

	interpreter->PushParam(ScriptParam(result,ScriptPtrType::GpuLight));
}

void ScriptCallbacks::SetLightColor(ScriptInterpreter * interpreter)
{
	POP_PTRPARAM(1,light,GpuLight);
	POP_NUMPARAM(2,r);
	POP_NUMPARAM(3,g);
	POP_NUMPARAM(4,b);

	Gpu::Light * gpuLight = static_cast<Gpu::Light*>(light.pvalue->ptr);

	if(!gpuLight) return;

	gpuLight->color.r = (float)r.nvalue;
	gpuLight->color.g = (float)g.nvalue;
	gpuLight->color.b = (float)b.nvalue;
}

void ScriptCallbacks::SetLightPosition(ScriptInterpreter * interpreter)
{
	POP_PTRPARAM(1,light,GpuLight);
	POP_NUMPARAM(2,x);
	POP_NUMPARAM(3,y);
	POP_NUMPARAM(4,z);
	
	Gpu::Light * gpuLight = static_cast<Gpu::Light*>(light.pvalue->ptr);

	switch(gpuLight->GetType())
	{
	case Gpu::LightType_Point:
		{
			Gpu::PointLight * pointLight = static_cast<Gpu::PointLight*>(gpuLight);
			pointLight->position.x = (float)x.nvalue;
			pointLight->position.y = (float)y.nvalue;
			pointLight->position.z = (float)z.nvalue;
		}
		break;
	case Gpu::LightType_Spot:
		{
			Gpu::SpotLight * spotLight = static_cast<Gpu::SpotLight*>(gpuLight);
			spotLight->position.x = (float)x.nvalue;
			spotLight->position.y = (float)y.nvalue;
			spotLight->position.z = (float)z.nvalue;
		}
		break;
	}
}

void ScriptCallbacks::SetLightDirection(ScriptInterpreter * interpreter)
{
	POP_PTRPARAM(1,light,GpuLight);
	POP_NUMPARAM(2,x);
	POP_NUMPARAM(3,y);
	POP_NUMPARAM(4,z);

	Gpu::Light * gpuLight = static_cast<Gpu::Light*>(light.pvalue->ptr);

	switch(gpuLight->GetType())
	{
	case Gpu::LightType_Directional:
		{
			Gpu::DirectionalLight * dirLight = static_cast<Gpu::DirectionalLight*>(gpuLight);
			dirLight->direction.x = (float)x.nvalue;
			dirLight->direction.y = (float)y.nvalue;
			dirLight->direction.z = (float)z.nvalue;
		}
		break;
	case Gpu::LightType_Spot:
		{
			Gpu::SpotLight * spotLight = static_cast<Gpu::SpotLight*>(gpuLight);
			spotLight->direction.x = (float)x.nvalue;
			spotLight->direction.y = (float)y.nvalue;
			spotLight->direction.z = (float)z.nvalue;
		}
		break;
	}
}

void ScriptCallbacks::SetLightAttenuation(ScriptInterpreter * interpreter)
{
	POP_PTRPARAM(1,light,GpuLight);
	POP_NUMPARAM(2,atten);

	Gpu::Light * gpuLight = static_cast<Gpu::Light*>(light.pvalue->ptr);

	switch(gpuLight->GetType())
	{
	case Gpu::LightType_Point:
		{
			Gpu::PointLight * pointLight = static_cast<Gpu::PointLight*>(gpuLight);
			pointLight->atten = (float) atten.nvalue;
		}
		break;
	case Gpu::LightType_Spot:
		{
			Gpu::SpotLight * spotLight = static_cast<Gpu::SpotLight*>(gpuLight);
			spotLight->atten = (float) atten.nvalue;
		}
		break;
	}
}

void ScriptCallbacks::SetMeshPosition(ScriptInterpreter * interpreter)
{
	POP_PTRPARAM(1,model,GpuComplexModel);
	POP_NUMPARAM(2,meshnum);
	POP_NUMPARAM(3,x);
	POP_NUMPARAM(4,y);
	POP_NUMPARAM(5,z);

	Gpu::ComplexModel * gpuModel = static_cast<Gpu::ComplexModel*>(model.pvalue->ptr);
	unsigned meshnumber = (unsigned) meshnum.nvalue;
	gpuModel->models[meshnumber].position = glm::vec3(float(x.nvalue),float(y.nvalue),float(z.nvalue));
}

void ScriptCallbacks::SetMeshEffect(ScriptInterpreter * interpreter)
{
	POP_PTRPARAM(1,model,GpuComplexModel);
	POP_NUMPARAM(2,meshnum);
	ScriptParam effect = interpreter->PopParam();

	Gpu::ComplexModel * gpuModel = static_cast<Gpu::ComplexModel*>(model.pvalue->ptr);
	unsigned meshnumber = (unsigned) meshnum.nvalue;
	Gpu::Effect * gpuEffect = 0;

	if(meshnumber < gpuModel->numModels)
	{
		if(effect.CheckPointer(ScriptPtrType::GpuEffect))
		{
			gpuEffect = static_cast<Gpu::Effect*>(effect.pvalue->ptr);
		}
		gpuModel->models[meshnumber].effect = gpuEffect;
	}
}

void ScriptCallbacks::CreateEffect(ScriptInterpreter * interpreter)
{
	ScriptParam nameOrShader = interpreter->PopParam();
	interpreter->ClearParams();
	Gpu::Shader * shader = 0;

	if(nameOrShader.type == ScriptParam::STRING)
	{
		shader = interpreter->GetApp()->assets->GetAsset<Gpu::Shader>(nameOrShader.svalue);
	}
	if(nameOrShader.type == ScriptParam::POINTER)
	{
		if(nameOrShader.pvalue->type != ScriptPtrType::GpuShader)
		{
			interpreter->ThrowError("CreateEffect: Pointer parameter is not of type GpuShader");
			return;
		}
		shader = static_cast<Gpu::Shader*>(nameOrShader.pvalue->ptr);
	}

	if(shader)
	{
		Gpu::Effect * effect = new Gpu::Effect(shader);

		interpreter->PushParam(ScriptParam(effect,ScriptPtrType::GpuEffect));
	}
}

void ScriptCallbacks::SetMeshTexture(ScriptInterpreter * interpreter)
{
	POP_PTRPARAM(1,model,GpuComplexModel);
	POP_NUMPARAM(2,meshnum);
	POP_PTRPARAM(3,texture,GpuTexture);

	Gpu::ComplexModel * gpuModel = static_cast<Gpu::ComplexModel*>(model.pvalue->ptr);
	Gpu::Texture * gpuTexture = static_cast<Gpu::Texture*>(texture.pvalue->ptr);
	unsigned meshnumber = (unsigned) meshnum.nvalue;

	if(meshnumber < gpuModel->numModels)
	{
		gpuModel->models[meshnumber].texture = gpuTexture;
	}
}

void ScriptCallbacks::SetMeshNormal(ScriptInterpreter * interpreter)
{
	POP_PTRPARAM(1,model,GpuComplexModel);
	POP_NUMPARAM(2,meshnum);
	POP_PTRPARAM(3,normal,GpuTexture);

	Gpu::ComplexModel * gpuModel = static_cast<Gpu::ComplexModel*>(model.pvalue->ptr);
	Gpu::Texture * gpuNormal = static_cast<Gpu::Texture*>(normal.pvalue->ptr);
	unsigned meshnumber = (unsigned) meshnum.nvalue;

	if(meshnumber < gpuModel->numModels)
	{
		gpuModel->models[meshnumber].normalMap = gpuNormal;
	}
}

void ScriptCallbacks::SetMeshCubeMap(ScriptInterpreter * interpreter)
{
	POP_PTRPARAM(1,model,GpuComplexModel);
	POP_NUMPARAM(2,meshnum);
	POP_PTRPARAM(3,cubemap,GpuCubeMap);

	Gpu::ComplexModel * gpuModel = static_cast<Gpu::ComplexModel*>(model.pvalue->ptr);
	Gpu::CubeMap * gpuCubeMap = static_cast<Gpu::CubeMap*>(cubemap.pvalue->ptr);
	unsigned meshnumber = (unsigned) meshnum.nvalue;

	if(meshnumber < gpuModel->numModels)
	{
		gpuModel->models[meshnumber].cubeMap = gpuCubeMap;
	}
}

void ScriptCallbacks::SetMeshColor(ScriptInterpreter * interpreter)
{
	POP_PTRPARAM(1,model,GpuComplexModel);
	POP_NUMPARAM(2,meshnum);
	POP_NUMPARAM(3,r);
	POP_NUMPARAM(4,g);
	POP_NUMPARAM(5,b);
	ScriptParam a = interpreter->PopParam();

	Gpu::ComplexModel * gpuModel = static_cast<Gpu::ComplexModel*>(model.pvalue->ptr);
	unsigned meshnumber = (unsigned) meshnum.nvalue;

	if(meshnumber < gpuModel->numModels)
	{
		gpuModel->models[meshnumber].color.r = (float) r.nvalue;
		gpuModel->models[meshnumber].color.g = (float) g.nvalue;
		gpuModel->models[meshnumber].color.b = (float) b.nvalue;
		if(a.type == ScriptParam::FLOAT || a.type == ScriptParam::DOUBLE)
		{
			gpuModel->models[meshnumber].color.a = (float) a.nvalue;
		}
	}
}

void ScriptCallbacks::SetMeshSpecular(ScriptInterpreter * interpreter)
{
	POP_PTRPARAM(1,model,GpuComplexModel);
	POP_NUMPARAM(2,meshnum);
	POP_NUMPARAM(3,specular);

	Gpu::ComplexModel * gpuModel = static_cast<Gpu::ComplexModel*>(model.pvalue->ptr);
	unsigned meshnumber = (unsigned) meshnum.nvalue;

	if(meshnumber < gpuModel->numModels)
	{
		gpuModel->models[meshnumber].specPower = (float) specular.nvalue;
	}
}

void ScriptCallbacks::GetMeshBounds(ScriptInterpreter * interpreter)
{
	POP_PTRPARAM(1, model, GpuComplexModel);
	POP_NUMPARAM(2, meshnum);
	interpreter->ClearParams();

	Gpu::ComplexModel * gpuModel = static_cast<Gpu::ComplexModel*>(model.pvalue->ptr);
	unsigned meshnumber = (unsigned)meshnum.nvalue;

	if(meshnumber < gpuModel->numModels)
	{
		Gpu::BoundingSphere& sphere = gpuModel->models[meshnumber].boundingSphere;
		interpreter->PushParam(ScriptParam(ScriptParam::DOUBLE, (double)sphere.origin.x));
		interpreter->PushParam(ScriptParam(ScriptParam::DOUBLE, (double)sphere.origin.y));
		interpreter->PushParam(ScriptParam(ScriptParam::DOUBLE, (double)sphere.origin.z));
		interpreter->PushParam(ScriptParam(ScriptParam::DOUBLE, (double)sphere.radius));
	}
}

void ScriptCallbacks::GetNumMeshes(ScriptInterpreter * interpreter)
{
	POP_PTRPARAM(1,model,GpuComplexModel);

	Gpu::ComplexModel * gpuModel = static_cast<Gpu::ComplexModel*>(model.pvalue->ptr);
	unsigned numMeshes = gpuModel->numModels;

	interpreter->PushParam(ScriptParam(ScriptParam::INT,numMeshes));
}

void ScriptCallbacks::SetEffectParam(ScriptInterpreter * interpreter)
{
	POP_PTRPARAM(1,effect,GpuEffect);
	POP_NUMPARAM(2,paramnum);
	ScriptParam value = interpreter->PopParam();

	Gpu::Effect * gpuEffect = static_cast<Gpu::Effect*>(effect.pvalue->ptr);

	if(!gpuEffect) return;

	unsigned paramIndex = (unsigned) paramnum.nvalue;

	if(value.type == ScriptParam::DOUBLE)
	{
		gpuEffect->SetParam(paramIndex,(float)value.nvalue);
	}
	if(value.type == ScriptParam::POINTER)
	{
		switch(value.pvalue->type)
		{
		case ScriptPtrType::GpuTexture:
			gpuEffect->SetParam(paramIndex,(Gpu::Texture*)value.pvalue->ptr);
			break;
		case ScriptPtrType::GpuCubeMap:
			gpuEffect->SetParam(paramIndex,(Gpu::CubeMap*)value.pvalue->ptr);
			break;
		case ScriptPtrType::GpuVolumeTexture:
			gpuEffect->SetParam(paramIndex,(Gpu::VolumeTexture*)value.pvalue->ptr);
			break;
		default:
			// Warn ?
			std::stringstream error;
			error << "Could not identify type of effect parameter " << paramIndex;
			interpreter->ThrowError(error.str().c_str());
		}
	}
}

void ScriptCallbacks::CreateSurface(ScriptInterpreter * interpreter)
{
	ScriptParam width = interpreter->PopParam();
	ScriptParam height = interpreter->PopParam();

	Gpu::DrawSurface * surface;

	if(width.type == ScriptParam::DOUBLE && height.type == ScriptParam::DOUBLE)
	{
		surface = interpreter->GetApp()->gpu->CreateDrawSurface(
			(unsigned)width.nvalue,
			(unsigned)height.nvalue);
	}
	else
	{
		surface = interpreter->GetApp()->gpu->CreateScreenDrawSurface();
	}

	interpreter->PushParam(ScriptParam(surface,ScriptPtrType::GpuDrawSurface));
};

void ScriptCallbacks::ShadeSurface(ScriptInterpreter * interpreter)
{
	POP_PTRPARAM(1,inSurface,GpuDrawSurface);
	POP_PTRPARAM(2,effect,GpuEffect);
	POP_PTRPARAM(3,outSurface,GpuDrawSurface);

	Gpu::DrawSurface * gpuInSurface = static_cast<Gpu::DrawSurface*>(inSurface.pvalue->ptr);
	Gpu::Effect * gpuEffect = static_cast<Gpu::Effect*>(effect.pvalue->ptr);
	Gpu::DrawSurface * gpuOutSurface = static_cast<Gpu::DrawSurface*>(outSurface.pvalue->ptr);

	Gpu::Api * gpu = interpreter->GetApp()->gpu;

	gpu->DrawGpuSurface(gpuInSurface,gpuEffect,gpuOutSurface);
}

void ScriptCallbacks::DrawSurface(ScriptInterpreter * interpreter) 
{
	POP_PTRPARAM(1,surface,GpuDrawSurface);

	Gpu::DrawSurface * gpuSurface = static_cast<Gpu::DrawSurface*>(surface.pvalue->ptr);

	Gpu::Sprite surfaceSprite;
	surfaceSprite.pixelSpace = true;
	surfaceSprite.texture = gpuSurface->GetTexture();
	
	interpreter->GetApp()->gpu->DrawGpuSprite(&surfaceSprite);
}

void ScriptCallbacks::ClearSurface(ScriptInterpreter * interpreter)
{
	POP_PTRPARAM(1,surface,GpuDrawSurface);
	ScriptParam r = interpreter->PopParam();
	ScriptParam g = interpreter->PopParam();
	ScriptParam b = interpreter->PopParam();
	ScriptParam a = interpreter->PopParam();

	Gpu::DrawSurface * gpuSurface = static_cast<Gpu::DrawSurface*>(surface.pvalue->ptr);

	if(r.type == ScriptParam::DOUBLE && g.type == ScriptParam::DOUBLE && b.type == ScriptParam::DOUBLE)
	{
		glm::vec4 clearColor = glm::vec4(0.0f, 0.0f, 0.0f, 1.0f);
		clearColor.r = float(r.nvalue);
		clearColor.g = float(g.nvalue);
		clearColor.b = float(b.nvalue);
		if(a.type == ScriptParam::DOUBLE)
		{
			clearColor.a = float(a.nvalue);
		}
		gpuSurface->Clear(clearColor);
	}
	else
	{
		gpuSurface->Clear();
	}
}

void ScriptCallbacks::GetSurfaceTexture(ScriptInterpreter * interpreter)
{
	POP_PTRPARAM(1,surface,GpuDrawSurface);

	Gpu::DrawSurface * gpuSurface = static_cast<Gpu::DrawSurface*>(surface.pvalue->ptr);

	Gpu::Texture * texture = gpuSurface->GetTexture();

	interpreter->ClearParams();
	interpreter->PushParam(ScriptParam(new NonDeletingPtr(texture,ScriptPtrType::GpuTexture)));
}

void ScriptCallbacks::LoadAssets(ScriptInterpreter * interpreter)
{
	AssetMgr * assets = interpreter->GetApp()->assets;
	AssetBatch assetBatch;

	ScriptParam item = interpreter->PopParam();

	bool failed = false;
	while(!failed && item.type != ScriptParam::NONE)
	{
		const char * properties[4];

		if(item.type == ScriptParam::MAPREF)
		{
			ScriptParam index; // NONE
			ScriptParam result;

			for(unsigned i = 0; i < 4; ++i)
			{
				if(interpreter->GetMapNext(item, index, result) 
					&& result.type == ScriptParam::STRING)
				{
					properties[i] = result.svalue;
				}
				else
				{
					failed = true;
					break;
				}
			}
			item = interpreter->PopParam();
		}
		else
		{
			for(unsigned i = 0; i < 4; ++i)
			{
				if(item.type == ScriptParam::STRING)
				{
					properties[i] = item.svalue;
				}
				else
				{
					failed = true;
					break;
				}
				item = interpreter->PopParam();
			}
		}

		if(!failed) LoadAsset(assets, assetBatch, properties[0], properties[1], properties[2], properties[3]);
	}

	unsigned ticketNum = assets->Load(assetBatch);

	interpreter->PushParam(ScriptParam(ScriptParam::INT,ticketNum));
}

void ScriptCallbacks::GetLoadProgress(ScriptInterpreter * interpreter)
{
	POP_NUMPARAM(1,ticket);
	interpreter->ClearParams();

	unsigned ticketNum = (unsigned) ticket.nvalue;

	float progress = interpreter->GetApp()->assets->GetProgress(ticketNum);

	interpreter->PushParam(ScriptParam(ScriptParam::FLOAT,progress));
}

void ScriptCallbacks::IsLoaded(ScriptInterpreter * interpreter)
{
	POP_NUMPARAM(1,ticket);
	interpreter->ClearParams();
	unsigned ticketNum = (unsigned) ticket.nvalue;

	bool loaded = interpreter->GetApp()->assets->IsLoaded(ticketNum);

	interpreter->PushParam(ScriptParam(ScriptParam::BOOL,loaded));
}

void ScriptCallbacks::UnloadAssets(ScriptInterpreter * interpreter)
{
	POP_NUMPARAM(1,ticket);
	interpreter->ClearParams();

	unsigned ticketNum = (unsigned) ticket.nvalue;

	interpreter->GetApp()->assets->Unload(ticketNum);
}

void ScriptCallbacks::GetAsset(ScriptInterpreter * interpreter)
{
	POP_PARAM(1, directoryOrName, STRING);
	ScriptParam subPath = interpreter->PopParam();
	IAsset * asset = 0;

	if(subPath.type == ScriptParam::STRING)
	{
		Files::Directory * directory = GetDirectory(interpreter->GetApp()->files, directoryOrName.svalue);

		std::string subPathShort(subPath.svalue);
		std::wstring subPathWide(subPathShort.begin(), subPathShort.end());

		asset = interpreter->GetApp()->assets->GetAsset<IAsset>(directory, subPathWide.c_str());
	}
	else
	{
		asset = interpreter->GetApp()->assets->GetAsset<IAsset>(directoryOrName.svalue);
	}

	if(asset)
	{
		switch(asset->GetType())
		{
			case TextureAsset:
			{
				Gpu::Texture * texAsset = dynamic_cast<Gpu::Texture*>(asset);
				interpreter->PushParam(ScriptParam(new NonDeletingPtr(texAsset, ScriptPtrType::GpuTexture)));
				break;
			}
			case CubeMapAsset:
			{
				Gpu::CubeMap * cubeAsset = dynamic_cast<Gpu::CubeMap*>(asset);
				interpreter->PushParam(ScriptParam(new NonDeletingPtr(cubeAsset, ScriptPtrType::GpuCubeMap)));
				break;
			}
			case VolumeTexAsset:
			{
				Gpu::VolumeTexture * vTexAsset = dynamic_cast<Gpu::VolumeTexture*>(asset);
				interpreter->PushParam(ScriptParam(new NonDeletingPtr(vTexAsset, ScriptPtrType::GpuVolumeTexture)));
				break;
			}
			case ShaderAsset:
			{
				Gpu::Shader * shdrAsset = dynamic_cast<Gpu::Shader*>(asset);
				interpreter->PushParam(ScriptParam(new NonDeletingPtr(shdrAsset, ScriptPtrType::GpuShader)));
				break;
			}
			case ModelAsset:
			{
				Gpu::ComplexModel * mdlAsset = dynamic_cast<Gpu::ComplexModel*>(asset);
				interpreter->PushParam(ScriptParam(mdlAsset, ScriptPtrType::GpuComplexModel));
				break;
			}
			case RawHeightMapAsset:
			{
				HeightParser * heightParser = dynamic_cast<HeightParser*>(asset);
				interpreter->PushParam(ScriptParam(new NonDeletingPtr(heightParser, ScriptPtrType::HeightParser)));
				break;
			}
			case ImageAsset:
			{
				Image::Buffer * imgAsset = dynamic_cast<Image::Buffer*>(asset);
				interpreter->PushParam(ScriptParam(new NonDeletingPtr(imgAsset, ScriptPtrType::ImageBuffer)));
				break;
			}
			case AudioAsset:
			{
				Audio::Item * audioItem = dynamic_cast<Audio::Item*>(asset);
				interpreter->PushParam(ScriptParam(new NonDeletingPtr(audioItem, ScriptPtrType::AudioItem)));
				break;
			}
		}
	}
}

void ScriptCallbacks::EnumerateDirectory(ScriptInterpreter * interpreter)
{
	POP_PARAM(1, directory, STRING);

	Files::Api * files = interpreter->GetApp()->files;
	Files::Directory * directoryObject = GetDirectory(files, directory.svalue);
	files->Enumerate(directoryObject, false);
}

void ScriptCallbacks::GetDirectoryFiles(ScriptInterpreter * interpreter)
{
	POP_PARAM(1, directory, STRING);
	interpreter->ClearParams();

	Files::Api * files = interpreter->GetApp()->files;
	Files::Directory * directoryObject = GetDirectory(files, directory.svalue);

	if(!directoryObject || directoryObject->numFiles < 0)
	{
		// Not yet enumerated, return None
		interpreter->PushParam(ScriptParam());
	}
	else
	{
		ScriptParam mapref = interpreter->CreateMap();
		for(int i = 0; i < directoryObject->numFiles; ++i)
		{
			// GAH!!! I can't reference the wide-strings in the directory object!!! 
			// No matter what I do, short-string versions would get lost when the function returns :(

			// Are you talking rubbish? If these strings are getting added to a table, won't the table store copies???
			// Let's find out...

			std::wstring wideName(directoryObject->fileNames[i]);
			std::string shortName(wideName.begin(), wideName.end());

			interpreter->SetMapNext(mapref,
				ScriptParam(ScriptParam::INT, double(i + 1)),
				ScriptParam(ScriptParam::STRING, shortName.c_str()));
		}

		interpreter->PushParam(mapref);
	}
}

void ScriptCallbacks::CreateModel(ScriptInterpreter * interpreter)
{
	POP_PARAM(1,type,STRING);
	POP_PARAM(2,vertices,MAPREF);
	POP_NUMPARAM(3,numvertices);
	POP_PARAM(3,indices,MAPREF);
	POP_NUMPARAM(4,numindices);

	ScriptParam key;
	ScriptParam item;

	std::vector<float> values;

	IVertexBuffer * vb = 0;
	unsigned vnum = 0;

	if(strcmp(type.svalue,"Pos") == 0)
		vb = new VertexBuffer<Vertex_Pos>((unsigned)numvertices.nvalue);
	else if(strcmp(type.svalue,"PosCol") == 0)
		vb = new VertexBuffer<Vertex_PosCol>((unsigned)numvertices.nvalue);
	else if(strcmp(type.svalue,"PosNor") == 0)
		vb = new VertexBuffer<Vertex_PosNor>((unsigned)numvertices.nvalue);
	else if(strcmp(type.svalue, "PosTex") == 0)
		vb = new VertexBuffer<Vertex_PosTex>((unsigned)numvertices.nvalue);
	else if(strcmp(type.svalue,"PosNorTex") == 0)
		vb = new VertexBuffer<Vertex_PosNorTex>((unsigned)numvertices.nvalue);
	else if(strcmp(type.svalue,"PosNorTanTex") == 0)
		vb = new VertexBuffer<Vertex_PosNorTanTex>((unsigned)numvertices.nvalue);

	if(!vb)
	{
		interpreter->ThrowError("Vertex Type not recognised");
	}

	while(interpreter->GetMapNext(vertices,key,item))
	{
		if(vnum >= vb->GetLength())
		{
			interpreter->ThrowError("Could not create model, vertex buffer overflow");
			return;
		}
		if(item.type != ScriptParam::MAPREF)
		{
			interpreter->ThrowError("Could not create model, vertex is not a data structure");
			return;
		}

		values.clear();

		ScriptParam component;
		ScriptParam value;

		while(interpreter->GetMapNext(item,component,value))
		{
			if(value.type != ScriptParam::DOUBLE 
				&& value.type != ScriptParam::FLOAT)
			{
				interpreter->ThrowError("Could not create model, vertex component is not a number");
				return;
			}

			values.push_back((float) value.nvalue);
		}

		bool malsized = false;

		switch(vb->GetVertexType())
		{
		case VertexType_Pos:
			if(values.size() != 3) {malsized = true; break;}
			((VertexBuffer<Vertex_Pos>*)vb)->Set(vnum,Vertex_Pos(
				values[0],values[1],values[2]));
			break;
		case VertexType_PosCol:
			if(values.size() != 6) {malsized = true; break;}
			((VertexBuffer<Vertex_PosCol>*)vb)->Set(vnum,Vertex_PosCol(
				values[0],values[1],values[2],
				values[3],values[4],values[5]));
			break;
		case VertexType_PosNor:
			if(values.size() != 6) {malsized = true; break;}
			((VertexBuffer<Vertex_PosNor>*)vb)->Set(vnum,Vertex_PosNor(
				values[0],values[1],values[2],
				values[3],values[4],values[5]));
			break;
		case VertexType_PosTex:
			if(values.size() != 5) { malsized = true; break; }
			((VertexBuffer<Vertex_PosTex>*)vb)->Set(vnum, Vertex_PosTex(
				values[0], values[1], values[2],
				values[3], values[4]));
			break;
		case VertexType_PosNorTex:
			if(values.size() != 8) {malsized = true; break;}
			((VertexBuffer<Vertex_PosNorTex>*)vb)->Set(vnum,Vertex_PosNorTex(
				values[0],values[1],values[2],
				values[3],values[4],values[5],
				values[6],values[7]));
			break;
		case VertexType_PosNorTanTex:
			if(values.size() != 12) {malsized = true; break;}
			((VertexBuffer<Vertex_PosNorTanTex>*)vb)->Set(vnum,Vertex_PosNorTanTex(
				values[0],values[1],values[2],
				values[3],values[4],values[5],
				values[6],values[7],values[8],values[9],
				values[10],values[11]));
			break;
		default:
			interpreter->ThrowError("Could not create model, could not identify vertex type");
			return;
		}

		if(malsized)
		{
			interpreter->ThrowError("Could not create model, incorrect number of vertex components");
			return;
		}

		++vnum;
	}

	key = ScriptParam();
	item = ScriptParam();

	unsigned iblength = (unsigned)numindices.nvalue;
	unsigned * ib = new unsigned[iblength];
	unsigned inum = 0;

	while(interpreter->GetMapNext(indices,key,item))
	{
		if(inum >= iblength)
		{
			interpreter->ThrowError("Could not create model, index buffer overflow");
		}
		if(item.type != ScriptParam::DOUBLE 
			&& item.type != ScriptParam::FLOAT 
			&& item.type != ScriptParam::INT)
		{
			interpreter->ThrowError("Could not create model, index is not a number");
			return;
		}

		ib[inum++] = (unsigned) item.nvalue;
	}

	Gpu::Mesh * mesh = interpreter->GetApp()->gpu->CreateGpuMesh(vb, iblength / 3, ib);

	if(mesh)
	{
		Gpu::ComplexModel * complexModel = new Gpu::ComplexModel(1);
		complexModel->models[0].mesh = mesh;
		complexModel->models[0].destructMesh = true;

		interpreter->PushParam(ScriptParam(complexModel,ScriptPtrType::GpuComplexModel));
	}

	delete vb;
	delete[] ib;
}

void ScriptCallbacks::SetHeightmapScale(ScriptInterpreter * interpreter)
{
	POP_PTRPARAM(1, heightmap, HeightParser);
	POP_NUMPARAM(2, x);
	POP_NUMPARAM(3, y);
	POP_NUMPARAM(4, z);
	interpreter->ClearParams();

	HeightParser * heightParser = static_cast<HeightParser*>(heightmap.pvalue->ptr);
	if(heightParser)
	{
		heightParser->SetScale(float(x.nvalue), float(y.nvalue), float(z.nvalue));
	}
}

void ScriptCallbacks::GetHeightmapHeight(ScriptInterpreter * interpreter)
{
	POP_PTRPARAM(1,heightmap,HeightParser);
	POP_NUMPARAM(2,x);
	POP_NUMPARAM(3,z);

	HeightParser * heightParser = static_cast<HeightParser*>(heightmap.pvalue->ptr);
	float y = 0.0f;
	
	if(heightParser)
	{
		y = heightParser->GetHeight((float)x.nvalue,(float)z.nvalue);
	}

	interpreter->PushParam(ScriptParam(ScriptParam::FLOAT,y));
}

void ScriptCallbacks::GetHeightmapModel(ScriptInterpreter * interpreter)
{
	POP_PTRPARAM(1, heightmap, HeightParser);
	interpreter->ClearParams();

	HeightParser * heightParser = static_cast<HeightParser*>(heightmap.pvalue->ptr);

	float y = 0.0f;

	if(heightParser)
	{
		Gpu::Api * gpu = interpreter->GetApp()->gpu;
		Gpu::Mesh * mesh = heightParser->GetMesh(&Gpu::Rect(0.0f, 0.0f, 1.0f, 1.0f))->GpuOnly(gpu);
		Gpu::ComplexModel * model = new Gpu::ComplexModel(1);
		model->models[0].mesh = mesh;
		model->models[0].destructMesh = true;
		
		interpreter->PushParam(ScriptParam(model,ScriptPtrType::GpuComplexModel));
	}
}

void ScriptCallbacks::SetClearColor(ScriptInterpreter * interpreter)
{
	POP_NUMPARAM(1,r);
	POP_NUMPARAM(2,g);
	POP_NUMPARAM(3,b);
	ScriptParam a = interpreter->PopParam();

	float alpha = 1.0f;
	if(a.type == ScriptParam::FLOAT || a.type == ScriptParam::DOUBLE)
	{
		alpha = (float) a.nvalue;
	}

	interpreter->GetApp()->gpu->SetClearColor(
		(float) r.nvalue,
		(float) g.nvalue,
		(float) b.nvalue,
		alpha);
}

void ScriptCallbacks::SetBlendMode(ScriptInterpreter * interpreter)
{
	POP_PARAM(1,mode,STRING);

	const char * modeChars = mode.svalue;

	if(_stricmp(modeChars,"add") == 0)
	{

	}
}

void ScriptCallbacks::SetWireframe(ScriptInterpreter * interpreter)
{
	POP_NUMPARAM(1,wireframe);

	interpreter->GetApp()->gpu->SetDrawWireframe(wireframe.nvalue > 0.0f);
}

void ScriptCallbacks::SetMultisampling(ScriptInterpreter * interpreter)
{
	POP_NUMPARAM(1,level);

	interpreter->GetApp()->gpu->SetMultisampling(unsigned(level.nvalue));
}

void ScriptCallbacks::GetScreenSize(ScriptInterpreter * interpreter)
{
	unsigned width, height;
	interpreter->GetApp()->gpu->GetBackbufferSize(width, height);

	interpreter->ClearParams();

	interpreter->PushParam(ScriptParam(ScriptParam::INT, width));
	interpreter->PushParam(ScriptParam(ScriptParam::INT, height));
}

void ScriptCallbacks::GetMousePosition(ScriptInterpreter * interpreter)
{
	InputState * input = interpreter->GetApp()->input;
	MouseState & mouse = input->GetMouseState();

	interpreter->PushParam(ScriptParam(ScriptParam::FLOAT, float(mouse.x)));
	interpreter->PushParam(ScriptParam(ScriptParam::FLOAT, float(mouse.y)));
}

void ScriptCallbacks::GetMouseLeft(ScriptInterpreter * interpreter)
{
	InputState * input = interpreter->GetApp()->input;
	MouseState & mouse = input->GetMouseState();

	interpreter->PushParam(ScriptParam(ScriptParam::BOOL, mouse.left));
	interpreter->PushParam(ScriptParam(ScriptParam::BOOL, mouse.leftDown));
	interpreter->PushParam(ScriptParam(ScriptParam::BOOL, mouse.leftUp));
}

void ScriptCallbacks::GetMouseRight(ScriptInterpreter * interpreter)
{
	InputState * input = interpreter->GetApp()->input;
	MouseState & mouse = input->GetMouseState();

	interpreter->PushParam(ScriptParam(ScriptParam::BOOL, mouse.right));
	interpreter->PushParam(ScriptParam(ScriptParam::BOOL, mouse.rightDown));
	interpreter->PushParam(ScriptParam(ScriptParam::BOOL, mouse.rightUp));
}

void ScriptCallbacks::GetKeyState(ScriptInterpreter * interpreter)
{
	ScriptParam key = interpreter->PopParam();
	InputState * input = interpreter->GetApp()->input;
	char keyChar = 0;
	
	if(key.type == ScriptParam::STRING)
	{
		keyChar = input->CharToScanCode(key.svalue[0]);
	}
	else if(key.type == ScriptParam::DOUBLE)
	{
		keyChar = static_cast<char>(key.nvalue);
	}
	else
	{
		interpreter->ThrowError("GetKeyState requires string or integer parameter");
		return;
	}
	KeyState & keyboard = input->GetKeyState();

	if(keyChar)
	{
		interpreter->PushParam(ScriptParam(ScriptParam::BOOL, keyboard.keys[keyChar]));
		interpreter->PushParam(ScriptParam(ScriptParam::BOOL, keyboard.downKeys[keyChar]));
		interpreter->PushParam(ScriptParam(ScriptParam::BOOL, keyboard.upKeys[keyChar]));
	}
}

void ScriptCallbacks::GetTypedText(ScriptInterpreter * interpreter)
{
	InputState * input = interpreter->GetApp()->input;
	KeyState & keyboard = input->GetKeyState();

	interpreter->PushParam(ScriptParam(ScriptParam::STRING, keyboard.text.data()));
}

void ScriptCallbacks::CreateRect(ScriptInterpreter * interpreter)
{
	POP_NUMPARAM(1,x);
	POP_NUMPARAM(2,y);
	POP_NUMPARAM(3,w);
	POP_NUMPARAM(4,h);
	ScriptParam updateRect = interpreter->PopParam();

	GeoBuilder builder;
	LocalMesh * localMesh = builder.BuildRect(
		float(x.nvalue),
		float(y.nvalue),
		float(w.nvalue),
		float(h.nvalue));

	Gpu::ComplexModel * gpuModel = 0;
	Gpu::Mesh * gpuMesh = 0;

	if(updateRect.CheckPointer(ScriptPtrType::GpuComplexModel))
	{
		gpuModel = static_cast<Gpu::ComplexModel*>(updateRect.pvalue->ptr);
		gpuMesh = gpuModel->models[0].mesh;
		interpreter->GetApp()->gpu->UpdateDynamicMesh(gpuMesh, localMesh->vertexBuffer);
		interpreter->GetApp()->gpu->UpdateDynamicMesh(gpuMesh, localMesh->numTriangles, localMesh->indexBuffer);
	}
	else
	{
		gpuMesh = localMesh->ToGpuMesh(interpreter->GetApp()->gpu,true);
		gpuModel = new Gpu::ComplexModel(1);
		gpuModel->models[0].mesh = gpuMesh;
		gpuModel->models[0].backFaceCull = false;
		gpuModel->models[0].destructMesh = true;

		interpreter->PushParam(ScriptParam(gpuModel,ScriptPtrType::GpuComplexModel));
	}

	delete localMesh;
}

void ScriptCallbacks::CreateRectStroke(ScriptInterpreter * interpreter)
{
	POP_NUMPARAM(1,x);
	POP_NUMPARAM(2,y);
	POP_NUMPARAM(3,w);
	POP_NUMPARAM(4,h);
	POP_NUMPARAM(5,weight);
	ScriptParam updateRect = interpreter->PopParam();

	GeoBuilder builder;
	LocalMesh * localMesh = builder.BuildRectStroke(
		float(x.nvalue),
		float(y.nvalue),
		float(w.nvalue),
		float(h.nvalue),
		float(weight.nvalue));

	Gpu::ComplexModel * gpuModel = 0;

	if(updateRect.CheckPointer(ScriptPtrType::GpuComplexModel))
	{
		gpuModel = static_cast<Gpu::ComplexModel*>(updateRect.pvalue->ptr);
		Gpu::Mesh * gpuMesh = gpuModel->models[0].mesh;
		interpreter->GetApp()->gpu->UpdateDynamicMesh(gpuMesh, localMesh->vertexBuffer);
		interpreter->GetApp()->gpu->UpdateDynamicMesh(gpuMesh, localMesh->numTriangles, localMesh->indexBuffer);
	}
	else
	{
		Gpu::Mesh * gpuMesh = localMesh->ToGpuMesh(interpreter->GetApp()->gpu,true);
		gpuModel = new Gpu::ComplexModel(1);
		gpuModel->models[0].mesh = gpuMesh;
		gpuModel->models[0].backFaceCull = false;
		gpuModel->models[0].destructMesh = true;

		interpreter->PushParam(ScriptParam(gpuModel,ScriptPtrType::GpuComplexModel));
	}

	delete localMesh;
}

void ScriptCallbacks::CreateEllipse(ScriptInterpreter * interpreter)
{
	POP_NUMPARAM(1,x);
	POP_NUMPARAM(2,y);
	POP_NUMPARAM(3,rx);
	POP_NUMPARAM(4,ry);
	ScriptParam updateEllipse = interpreter->PopParam();

	GeoBuilder builder;
	LocalMesh * localMesh = builder.BuildEllipse(
		float(x.nvalue),
		float(y.nvalue),
		float(rx.nvalue),
		float(ry.nvalue));

	if(localMesh)
	{
		Gpu::ComplexModel * gpuModel = 0;

		if(updateEllipse.CheckPointer(ScriptPtrType::GpuComplexModel))
		{
			gpuModel = static_cast<Gpu::ComplexModel*>(updateEllipse.pvalue->ptr);
			Gpu::Mesh * gpuMesh = gpuModel->models[0].mesh;
			interpreter->GetApp()->gpu->UpdateDynamicMesh(gpuMesh, localMesh->vertexBuffer);
			interpreter->GetApp()->gpu->UpdateDynamicMesh(gpuMesh, localMesh->numTriangles, localMesh->indexBuffer);
		}
		else
		{
			Gpu::Mesh * gpuMesh = localMesh->ToGpuMesh(interpreter->GetApp()->gpu,true);
			gpuModel = new Gpu::ComplexModel(1);
			gpuModel->models[0].mesh = gpuMesh;
			gpuModel->models[0].backFaceCull = false;
			gpuModel->models[0].destructMesh = true;

			interpreter->PushParam(ScriptParam(gpuModel,ScriptPtrType::GpuComplexModel));
		}

		delete localMesh;
	}
}

void ScriptCallbacks::CreateEllipseStroke(ScriptInterpreter * interpreter)
{
	POP_NUMPARAM(1,x);
	POP_NUMPARAM(2,y);
	POP_NUMPARAM(3,rx);
	POP_NUMPARAM(4,ry);
	POP_NUMPARAM(5,weight);
	ScriptParam updateEllipse = interpreter->PopParam();

	GeoBuilder builder;
	LocalMesh * localMesh = builder.BuildEllipseStroke(
		float(x.nvalue),
		float(y.nvalue),
		float(rx.nvalue),
		float(ry.nvalue),
		float(weight.nvalue));

	Gpu::ComplexModel * gpuModel = 0;

	if(updateEllipse.CheckPointer(ScriptPtrType::GpuComplexModel))
	{
		gpuModel = static_cast<Gpu::ComplexModel*>(updateEllipse.pvalue->ptr);
		Gpu::Mesh * gpuMesh = gpuModel->models[0].mesh;
		interpreter->GetApp()->gpu->UpdateDynamicMesh(gpuMesh, localMesh->vertexBuffer);
		interpreter->GetApp()->gpu->UpdateDynamicMesh(gpuMesh, localMesh->numTriangles, localMesh->indexBuffer);
	}
	else
	{
		Gpu::Mesh * gpuMesh = localMesh->ToGpuMesh(interpreter->GetApp()->gpu,true);
		gpuModel = new Gpu::ComplexModel(1);
		gpuModel->models[0].mesh = gpuMesh;
		gpuModel->models[0].backFaceCull = false;
		gpuModel->models[0].destructMesh = true;

		interpreter->PushParam(ScriptParam(gpuModel,ScriptPtrType::GpuComplexModel));
	}

	delete localMesh;
}

void ScriptCallbacks::CreateScene(ScriptInterpreter * interpreter)
{
	ScriptParam type = interpreter->PopParam();
	
	Gpu::Scene * scene = 0;

	if(type.type == ScriptParam::STRING)
	{
		if(strcmp(type.svalue,"instanced") == 0)
		{
			// FIXME - Should be able to define the instance type, maybe?
			scene = new Gpu::InstancedScene<Instance_PosCol>(interpreter->GetApp()->gpu); 
		}
	}

	if(!scene) scene = new Gpu::LinearScene();

	interpreter->ClearParams();
	interpreter->PushParam(ScriptParam(scene,ScriptPtrType::GpuScene));
}

void ScriptCallbacks::AddToScene(ScriptInterpreter * interpreter)
{
	POP_PTRPARAM(1,scene,GpuScene);
	ScriptParam object = interpreter->PopParam();

	Gpu::Scene * gpuScene = static_cast<Gpu::Scene*>(scene.pvalue->ptr);

	if(object.type == ScriptParam::POINTER)
	{
		if(object.pvalue->type == ScriptPtrType::GpuComplexModel)
		{
			Gpu::ComplexModel * model = static_cast<Gpu::ComplexModel*>(object.pvalue->ptr);
			for(unsigned i = 0; i < model->numModels; ++i)
			{
				gpuScene->Add(&model->models[i]);
			}
		}
		if(object.pvalue->type == ScriptPtrType::GpuTexture)
		{
			// FIXME - if I create a new sprite here, who's going to delete it when the scene is destroyed?
		}
		if(object.pvalue->type == ScriptPtrType::GpuLight)
		{
			Gpu::Light * light = static_cast<Gpu::Light*>(object.pvalue->ptr);
			gpuScene->Add(light);
		}
	}
	
}

void ScriptCallbacks::ClearScene(ScriptInterpreter * interpreter)
{
	POP_PTRPARAM(1,scene,GpuScene);

	Gpu::Scene * gpuScene = static_cast<Gpu::Scene*>(scene.pvalue->ptr);

	gpuScene->ClearModels();
	gpuScene->ClearSprites();
	gpuScene->ClearLights();
}

void ScriptCallbacks::DrawScene(ScriptInterpreter * interpreter)
{
	POP_PTRPARAM(1,scene,GpuScene);
	POP_PTRPARAM(2,camera,GpuCamera);

	Gpu::Scene * gpuScene = static_cast<Gpu::Scene*>(scene.pvalue->ptr);
	Gpu::Camera * gpuCamera = static_cast<Gpu::Camera*>(camera.pvalue->ptr);

	gpuScene->SetCamera(gpuCamera);

	interpreter->GetApp()->gpu->Draw(gpuScene);
}

void ScriptCallbacks::GetImageSize(ScriptInterpreter * interpreter)
{
	POP_PTRPARAM(1,image,ImageBuffer);

	Image::Buffer * imageBuffer = static_cast<Image::Buffer*>(image.pvalue->ptr);
	Image::Api * imaging = interpreter->GetApp()->imaging;

	unsigned w, h;
	imaging->GetImageSize(imageBuffer,w,h);

	interpreter->ClearParams();
	interpreter->PushParam(ScriptParam(ScriptParam::INT, double(w)));
	interpreter->PushParam(ScriptParam(ScriptParam::INT, double(h)));
}

void ScriptCallbacks::GetImagePixel(ScriptInterpreter * interpreter)
{
	POP_PTRPARAM(1,image,ImageBuffer);
	POP_NUMPARAM(2,u);
	POP_NUMPARAM(3,v);

	Image::Buffer * imageBuffer = static_cast<Image::Buffer*>(image.pvalue->ptr);
	Image::Api * imaging = interpreter->GetApp()->imaging;
	
	if(u.nvalue < 0.0f || v.nvalue < 0.0f) return;

	unsigned uVal = unsigned(u.nvalue);
	unsigned vVal = unsigned(v.nvalue);
	Image::Color color = imaging->GetPixelColor(imageBuffer,uVal,vVal);

	interpreter->ClearParams();
	interpreter->PushParam(ScriptParam(ScriptParam::DOUBLE, double(color.r)));
	interpreter->PushParam(ScriptParam(ScriptParam::DOUBLE, double(color.g)));
	interpreter->PushParam(ScriptParam(ScriptParam::DOUBLE, double(color.b)));
	interpreter->PushParam(ScriptParam(ScriptParam::DOUBLE, double(color.a)));
}

struct ScriptCallbackResponse : public Files::Response
{
	ScriptInterpreter * interpreter;
	ScriptParam callback;

	ScriptCallbackResponse(ScriptInterpreter * interpreter, ScriptParam callback) :
		interpreter(interpreter), callback(callback) {}

	virtual void Respond() override
	{
		closeOnComplete = true; deleteOnComplete = true;

		const char * chars = buffer ? buffer : "";
		interpreter->PushParam(ScriptParam(ScriptParam::STRING, chars));
		interpreter->CallFunction(callback);
	}
};

void ScriptCallbacks::LoadFile(ScriptInterpreter * interpreter)
{
	POP_PARAM(1, directory, STRING);
	POP_PARAM(2, path, STRING);
	POP_PARAM(3, callback, FUNCTION);

	Files::Directory * fileDir = GetDirectory(interpreter->GetApp()->files, directory.svalue);
	std::string spath(path.svalue);
	std::wstring wpath(spath.begin(), spath.end());

	if(fileDir->isProjectDir)
	{
		wpath = subPathString + wpath;
	}

	interpreter->GetApp()->files->OpenAndRead(fileDir, wpath.c_str(), new ScriptCallbackResponse(interpreter, callback));
}

void ScriptCallbacks::PickFile(ScriptInterpreter * interpreter)
{
	POP_PARAM(1, directory, STRING);
	POP_PARAM(2, extension, STRING);
	POP_PARAM(3, callback, FUNCTION);

	Files::Directory * fileDir = GetDirectory(interpreter->GetApp()->files, directory.svalue);
	std::string shortExt(extension.svalue);
	std::wstring wideExt(shortExt.begin(), shortExt.end());

	if(fileDir->isProjectDir)
	{

	}

	interpreter->GetApp()->files->PickFile(fileDir, wideExt.c_str(), new ScriptCallbackResponse(interpreter, callback));
}

void ScriptCallbacks::PlaySound(ScriptInterpreter * interpreter)
{
	POP_PTRPARAM(1, sound, AudioItem);
	ScriptParam loop = interpreter->PopParam();
	
	Audio::Item * audioItem = static_cast<Audio::Item*>(sound.pvalue->ptr);
	bool loopBool = false;

	if(loop.type == ScriptParam::BOOL || loop.type == ScriptParam::INT
		|| loop.type == ScriptParam::FLOAT || loop.type == ScriptParam::DOUBLE)
	{
		loopBool = loop.nvalue > 0;
	}

	interpreter->GetApp()->audio->Play(audioItem, loopBool);
}

void ScriptCallbacks::GetAmplitude(ScriptInterpreter * interpreter)
{
	ScriptParam sound = interpreter->PopParam();
	Audio::Item * item = 0;

	if(sound.type == ScriptParam::POINTER && sound.pvalue->type == ScriptPtrType::AudioItem)
	{
		Audio::Item * item = static_cast<Audio::Item*>(sound.pvalue->ptr);
	}

	float amplitude = interpreter->GetApp()->audio->GetAmplitude(item);

	interpreter->ClearParams();
	interpreter->PushParam(ScriptParam(ScriptParam::FLOAT, double(amplitude)));
}

} // namespace Ingenuity
