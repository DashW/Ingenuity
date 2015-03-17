#include "GpuScriptCallbacks.h"
#include "ScriptInterpreter.h"
#include "ScriptTypes.h"
#include "AssetMgr.h"
#include "GeoBuilder.h"
#include "GpuScene.h"
#include "SvgParser.h"
#include "ImageApi.h"
#include "InputState.h"
#include "AudioApi.h"
#include "IngenuityHelper.h"
#include "PlatformApi.h"
#include "IsoSurface.h"
#include "PhysicsApi.h"
#include "WavefrontLoader.h"
#include "HeightParser.h"
#include "LeapMotionHelper.h"
#include "SpriteManager.h"
#include <sstream>
#include <vector>

// Generic Parameter
#define POP_PARAM(NUM,NAME,TYPE) ScriptParam NAME = interpreter->PopParam();\
	if(NAME.type != ScriptParam::TYPE) { \
	interpreter->ThrowError("parameter " #NUM " (" #NAME ") is not of type " #TYPE); \
	return; }

// Numeric Parameter
#define POP_NUMPARAM(NUM,NAME) ScriptParam NAME = interpreter->PopParam();\
	if(!NAME.IsNumber() && NAME.type != ScriptParam::BOOL) {\
	interpreter->ThrowError("parameter " #NUM " (" #NAME ") is not a number");\
	return; }

// Pointer Parameter
#define POP_PTRPARAM(NUM,NAME,PTRTYPE) ScriptParam NAME = interpreter->PopParam();\
	if(!ScriptTypes::CheckPtrType(NAME,PTRTYPE)) {\
	interpreter->ThrowError("parameter " #NUM " (" #NAME ") is not a pointer of type " #PTRTYPE);\
	return; }

// Special Pointer Parameter
#define POP_SPTRPARAM(NUM,NAME,PTRTYPE) ScriptParam NAME = interpreter->PopParam();\
	if(!NAME.CheckPointer(interpreter->GetSpecialPtrType(ScriptInterpreter::PTRTYPE))) {\
	interpreter->ThrowError("parameter " #NUM " (" #NAME ") is not a pointer of type " #PTRTYPE);\
	return; }

namespace Ingenuity {

ScriptParam GpuScriptCallbacks::VertexBufferToFloats(ScriptInterpreter * interpreter, IVertexBuffer * vertexBuffer)
{
	if(!interpreter || !vertexBuffer) return ScriptParam();

	const unsigned rawBufferSize = vertexBuffer->GetLength() * vertexBuffer->GetElementSize();
	const unsigned floatBufferSize = rawBufferSize / sizeof(float);

	ScriptFloatArray * scriptFloatArray = new ScriptFloatArray(floatBufferSize);

	memcpy(scriptFloatArray->floats, vertexBuffer->GetData(), rawBufferSize);

	return ScriptParam(scriptFloatArray, ScriptTypes::GetHandle(TypeFloatArray));
}

LocalMesh * GpuScriptCallbacks::FloatsToLocalMesh(ScriptInterpreter * interpreter, ScriptParam type, ScriptParam vertices, ScriptParam indices)
{
	LocalMesh * localMesh = 0;

	if(vertices.type == ScriptParam::MAPREF && indices.type == ScriptParam::MAPREF)
	{
		ScriptParam key;
		ScriptParam item;

		std::vector<float> values;

		IVertexBuffer * vb = 0;
		unsigned vnum = 0;
		unsigned numvertices = interpreter->GetMapLength(vertices);
		unsigned numindices = interpreter->GetMapLength(indices);

		if(numvertices == 0 || numindices == 0 || numindices % 3 != 0)
		{
			interpreter->ThrowError("Could not create model, no vertices or invalid number of indices");
			return 0;
		}

		if(strcmp(type.svalue, "Pos") == 0)
			vb = new VertexBuffer<Vertex_Pos>(numvertices);
		else if(strcmp(type.svalue, "PosCol") == 0)
			vb = new VertexBuffer<Vertex_PosCol>(numvertices);
		else if(strcmp(type.svalue, "PosNor") == 0)
			vb = new VertexBuffer<Vertex_PosNor>(numvertices);
		else if(strcmp(type.svalue, "PosTex") == 0)
			vb = new VertexBuffer<Vertex_PosTex>(numvertices);
		else if(strcmp(type.svalue, "PosNorTex") == 0)
			vb = new VertexBuffer<Vertex_PosNorTex>(numvertices);
		else if(strcmp(type.svalue, "PosNorTanTex") == 0)
			vb = new VertexBuffer<Vertex_PosNorTanTex>(numvertices);

		if(!vb)
		{
			interpreter->ThrowError("Vertex Type not recognised");
		}

		while(interpreter->GetMapNext(vertices, key, item))
		{
			if(vnum >= vb->GetLength())
			{
				interpreter->ThrowError("Could not create model, vertex buffer overflow");
				return 0;
			}
			if(item.type != ScriptParam::MAPREF)
			{
				interpreter->ThrowError("Could not create model, vertex is not a data structure");
				return 0;
			}

			values.clear();

			ScriptParam component;
			ScriptParam value;

			while(interpreter->GetMapNext(item, component, value))
			{
				if(value.type != ScriptParam::DOUBLE)
				{
					interpreter->ThrowError("Could not create model, vertex component is not a number");
					return 0;
				}

				values.push_back((float)value.nvalue);
			}

			bool malsized = false;

			switch(vb->GetVertexType())
			{
			case VertexType_Pos:
				if(values.size() != 3) { malsized = true; break; }
				((VertexBuffer<Vertex_Pos>*)vb)->Set(vnum, Vertex_Pos(
					values[0], values[1], values[2]));
				break;
			case VertexType_PosCol:
				if(values.size() != 6) { malsized = true; break; }
				((VertexBuffer<Vertex_PosCol>*)vb)->Set(vnum, Vertex_PosCol(
					values[0], values[1], values[2],
					values[3], values[4], values[5]));
				break;
			case VertexType_PosNor:
				if(values.size() != 6) { malsized = true; break; }
				((VertexBuffer<Vertex_PosNor>*)vb)->Set(vnum, Vertex_PosNor(
					values[0], values[1], values[2],
					values[3], values[4], values[5]));
				break;
			case VertexType_PosTex:
				if(values.size() != 5) { malsized = true; break; }
				((VertexBuffer<Vertex_PosTex>*)vb)->Set(vnum, Vertex_PosTex(
					values[0], values[1], values[2],
					values[3], values[4]));
				break;
			case VertexType_PosNorTex:
				if(values.size() != 8) { malsized = true; break; }
				((VertexBuffer<Vertex_PosNorTex>*)vb)->Set(vnum, Vertex_PosNorTex(
					values[0], values[1], values[2],
					values[3], values[4], values[5],
					values[6], values[7]));
				break;
			case VertexType_PosNorTanTex:
				if(values.size() != 12) { malsized = true; break; }
				((VertexBuffer<Vertex_PosNorTanTex>*)vb)->Set(vnum, Vertex_PosNorTanTex(
					values[0], values[1], values[2],
					values[3], values[4], values[5],
					values[6], values[7], values[8], values[9],
					values[10], values[11]));
				break;
			default:
				interpreter->ThrowError("Could not create model, could not identify vertex type");
				return 0;
			}

			if(malsized)
			{
				interpreter->ThrowError("Could not create model, incorrect number of vertex components");
				return 0;
			}

			++vnum;
		}

		key = ScriptParam();
		item = ScriptParam();

		unsigned * ib = new unsigned[numindices];
		unsigned inum = 0;

		while(interpreter->GetMapNext(indices, key, item))
		{
			if(inum >= numindices)
			{
				interpreter->ThrowError("Could not create model, index buffer overflow");
			}
			if(!item.IsNumber())
			{
				interpreter->ThrowError("Could not create model, index is not a number");
				return 0;
			}

			ib[inum++] = (unsigned)item.nvalue;
		}

		localMesh = new LocalMesh(vb, ib, numindices);
	}
	else if(ScriptTypes::CheckPtrType(vertices, TypeFloatArray) && ScriptTypes::CheckPtrType(indices, TypeFloatArray))
	{
		ScriptFloatArray * vertexArray = vertices.GetPointer<ScriptFloatArray>();
		ScriptFloatArray * indexArray = indices.GetPointer<ScriptFloatArray>();

		IVertexBuffer * vb = 0;
		unsigned vnum = 0;
		unsigned numfloats = vertexArray->length;
		unsigned vertexArraySize = numfloats * sizeof(float);
		unsigned numindices = indexArray->length;

		if(numfloats == 0 || numindices == 0 || numindices % 3 != 0)
		{
			interpreter->ThrowError("Could not create model, no vertices or invalid number of indices");
			return 0;
		}

		if(strcmp(type.svalue, "Pos") == 0)
			vb = new VertexBuffer<Vertex_Pos>(vertexArraySize / sizeof(Vertex_Pos));
		else if(strcmp(type.svalue, "PosCol") == 0)
			vb = new VertexBuffer<Vertex_PosCol>(vertexArraySize / sizeof(Vertex_PosCol));
		else if(strcmp(type.svalue, "PosNor") == 0)
			vb = new VertexBuffer<Vertex_PosNor>(vertexArraySize / sizeof(Vertex_PosNor));
		else if(strcmp(type.svalue, "PosTex") == 0)
			vb = new VertexBuffer<Vertex_PosTex>(vertexArraySize / sizeof(Vertex_PosTex));
		else if(strcmp(type.svalue, "PosNorTex") == 0)
			vb = new VertexBuffer<Vertex_PosNorTex>(vertexArraySize / sizeof(Vertex_PosNorTex));
		else if(strcmp(type.svalue, "PosNorTanTex") == 0)
			vb = new VertexBuffer<Vertex_PosNorTanTex>(vertexArraySize / sizeof(Vertex_PosNorTanTex));

		unsigned vertexBufferSize = vb->GetLength() * vb->GetElementSize();
		if(vertexBufferSize != vertexArraySize)
		{
			interpreter->ThrowError("Could not create model, float array is not evenly divisible by vertex type!");
			delete vb;
			return 0;
		}

		memcpy(vb->GetData(), vertexArray->floats, vb->GetLength() * vb->GetElementSize());

		unsigned * ib = new unsigned[numindices];
		for(unsigned i = 0; i < numindices; i++)
		{
			ib[i] = unsigned(indexArray->floats[i]);
		}

		localMesh = new LocalMesh(vb, ib, numindices);
	}
	else
	{
		interpreter->ThrowError("Could not create model, vertex/index buffers were not tables or floatArrays");
	}

	return localMesh;
}

void GpuScriptCallbacks::CreateCamera(ScriptInterpreter * interpreter)
{
	ScriptParam orthographic = interpreter->PopParam();

	Gpu::Camera * camera = new Gpu::Camera();

	if(orthographic.type == ScriptParam::BOOL)
	{
		camera->isOrthoCamera = orthographic.nvalue > 0.0;
	}

	if(camera) interpreter->PushParam(ScriptParam(camera, ScriptTypes::GetHandle(TypeGpuCamera)));
}

void GpuScriptCallbacks::CreateSpriteCamera(ScriptInterpreter * interpreter)
{
	ScriptParam pixelSpace = interpreter->PopParam();
	ScriptParam centerOrigin = interpreter->PopParam();
	ScriptParam yUpwards = interpreter->PopParam();
	ScriptParam surface = interpreter->PopParam();

	Gpu::DrawSurface * gpuSurface = 0;
	if(ScriptTypes::CheckPtrType(surface, TypeGpuDrawSurface))
	{
		gpuSurface = surface.GetPointer<Gpu::DrawSurface>();
	}

	SpriteCamera spriteCamera;
	spriteCamera.pixelSpace = true;
	spriteCamera.centerOrigin = false;
	spriteCamera.yUpwards = false;

	if(pixelSpace.type == ScriptParam::BOOL && pixelSpace.nvalue <= 0.0)
	{
		spriteCamera.pixelSpace = false;
	}
	if(centerOrigin.type == ScriptParam::BOOL && centerOrigin.nvalue > 0.0)
	{
		spriteCamera.centerOrigin = true;
	}
	if(yUpwards.type == ScriptParam::BOOL && yUpwards.nvalue > 0.0)
	{
		spriteCamera.yUpwards = true;
	}

	Gpu::Camera * gpuCamera = new Gpu::Camera(interpreter->GetApp()->sprites->SpriteToGpuCamera(spriteCamera, gpuSurface));

	interpreter->PushParam(ScriptParam(gpuCamera, ScriptTypes::GetHandle(TypeGpuCamera)));
}

void GpuScriptCallbacks::SetCameraPosition(ScriptInterpreter * interpreter)
{
	POP_PTRPARAM(1,camera,TypeGpuCamera);
	POP_NUMPARAM(2,x);
	POP_NUMPARAM(3,y);
	POP_NUMPARAM(4,z);

	Gpu::Camera * gpuCamera = camera.GetPointer<Gpu::Camera>();

	if(!gpuCamera) return;

	gpuCamera->position.x = (float) x.nvalue;
	gpuCamera->position.y = (float) y.nvalue;
	gpuCamera->position.z = (float) z.nvalue;
}

void GpuScriptCallbacks::SetCameraTarget(ScriptInterpreter * interpreter)
{
	POP_PTRPARAM(1,camera,TypeGpuCamera);
	POP_NUMPARAM(2,x);
	POP_NUMPARAM(3,y);
	POP_NUMPARAM(4,z);

	Gpu::Camera * gpuCamera = camera.GetPointer<Gpu::Camera>();

	if(!gpuCamera) return;

	gpuCamera->target.x = (float) x.nvalue;
	gpuCamera->target.y = (float) y.nvalue;
	gpuCamera->target.z = (float) z.nvalue;
}

void GpuScriptCallbacks::SetCameraUp(ScriptInterpreter * interpreter)
{
	POP_PTRPARAM(1, camera, TypeGpuCamera);
	POP_NUMPARAM(2, x);
	POP_NUMPARAM(3, y);
	POP_NUMPARAM(4, z);

	Gpu::Camera * gpuCamera = camera.GetPointer<Gpu::Camera>();

	if(!gpuCamera) return;

	gpuCamera->up.x = (float)x.nvalue;
	gpuCamera->up.y = (float)y.nvalue;
	gpuCamera->up.z = (float)z.nvalue;
}

void GpuScriptCallbacks::SetCameraClipFovOrHeight(ScriptInterpreter * interpreter)
{
	POP_PTRPARAM(1,camera,TypeGpuCamera);
	POP_NUMPARAM(2,nearclip);
	POP_NUMPARAM(3,farclip);
	POP_NUMPARAM(4,fovOrHeight);

	Gpu::Camera * gpuCamera = camera.GetPointer<Gpu::Camera>();
	if(!gpuCamera) return;

	gpuCamera->nearClip = (float) nearclip.nvalue;
	gpuCamera->farClip = (float) farclip.nvalue;
	gpuCamera->fovOrHeight = (float) fovOrHeight.nvalue;
}

void GpuScriptCallbacks::GetCameraRay(ScriptInterpreter * interpreter)
{
	POP_PTRPARAM(1, camera, TypeGpuCamera);
	POP_NUMPARAM(2, x);
	POP_NUMPARAM(3, y);
	ScriptParam surface = interpreter->PopParam();
	interpreter->ClearParams();

	Gpu::Camera * gpuCamera = camera.GetPointer<Gpu::Camera>();

	unsigned width, height;
	if(ScriptTypes::CheckPtrType(surface, TypeGpuDrawSurface))
	{
		Gpu::DrawSurface * gpuSurface = surface.GetPointer<Gpu::DrawSurface>();
		width = gpuSurface->GetTexture()->GetWidth();
		height = gpuSurface->GetTexture()->GetHeight();
	}
	else
	{
		interpreter->GetApp()->gpu->GetBackbufferSize(width, height);
	}

	float aspect = float(width) / float(height);
	glm::vec4 ray(gpuCamera->GetUnprojectedRay(float(x.nvalue), float(y.nvalue), aspect), 0.0f);
	ray *= (gpuCamera->farClip - gpuCamera->nearClip);

	interpreter->PushParam(ScriptParam(new BufferCopyPtr(&ray, sizeof(glm::vec4), 
		interpreter->GetSpecialPtrType(ScriptInterpreter::TypeVector4))));
}

void GpuScriptCallbacks::GetCameraViewMatrix(ScriptInterpreter * interpreter)
{
	POP_PTRPARAM(1, camera, TypeGpuCamera);
	interpreter->ClearParams();

	Gpu::Camera * gpuCamera = camera.GetPointer<Gpu::Camera>();

	glm::mat4 viewMatrix = gpuCamera->GetViewMatrix();

	interpreter->PushParam(ScriptParam(new BufferCopyPtr(&viewMatrix, sizeof(glm::mat4),
		interpreter->GetSpecialPtrType(ScriptInterpreter::TypeMatrix4))));
}

void GpuScriptCallbacks::GetCameraProjMatrix(ScriptInterpreter * interpreter)
{
	POP_PTRPARAM(1, camera, TypeGpuCamera);
	ScriptParam surface = interpreter->PopParam();
	ScriptParam isTex = interpreter->PopParam();
	interpreter->ClearParams();

	Gpu::Camera * gpuCamera = camera.GetPointer<Gpu::Camera>();

	float aspect = 1.0f;
	
	if(ScriptTypes::CheckPtrType(surface, TypeGpuDrawSurface))
	{
		Gpu::DrawSurface * gpuSurface = surface.GetPointer<Gpu::DrawSurface>();
		aspect = float(gpuSurface->GetWidth()) / float(gpuSurface->GetHeight());
	}

	glm::mat4 projectionMatrix = gpuCamera->GetProjMatrix(aspect);

	// Transform NDC space [-1,+1]^2 to texture space [0,1]^2
	static const glm::mat4 texCoordTransform(
		0.5f, 0.0f, 0.0f, 0.0f,
		0.0f, -0.5f, 0.0f, 0.0f,
		0.0f, 0.0f, 1.0f, 0.0f,
		0.5f, 0.5f, 0.0f, 1.0f);

	if(isTex.type == ScriptParam::BOOL && isTex.nvalue > 0.0)
	{
		projectionMatrix = texCoordTransform * projectionMatrix;
	}

	interpreter->PushParam(ScriptParam(new BufferCopyPtr(&projectionMatrix, sizeof(glm::mat4),
		interpreter->GetSpecialPtrType(ScriptInterpreter::TypeMatrix4))));
}

void GpuScriptCallbacks::DrawModel(ScriptInterpreter * interpreter)
{
	POP_PTRPARAM(1,model,TypeGpuComplexModel);
	POP_PTRPARAM(2,camera,TypeGpuCamera);
	ScriptParam lights = interpreter->PopParam();
	ScriptParam surface = interpreter->PopParam();
	ScriptParam instances = interpreter->PopParam();
	ScriptParam effect = interpreter->PopParam();
	interpreter->ClearParams();

	Gpu::ComplexModel * gpuModel = model.GetPointer<Gpu::ComplexModel>();
	Gpu::Camera * gpuCamera = camera.GetPointer<Gpu::Camera>();
	Gpu::Light * gpuLights[5];
	unsigned numGpuLights = 0;
	Gpu::DrawSurface * gpuSurface = 0;
	Gpu::InstanceBuffer * gpuInstances = 0;
	Gpu::Effect * gpuOverrideEffect = 0;

	if(lights.type == ScriptParam::MAPREF)
	{
		ScriptParam index; // NONE
		ScriptParam result;

		while(interpreter->GetMapNext(lights,index,result) && numGpuLights < 5)
		{
			if(!ScriptTypes::CheckPtrType(result,TypeGpuLight)) break;
			gpuLights[numGpuLights++] = result.GetPointer<Gpu::Light>();
		}
	}
	if(ScriptTypes::CheckPtrType(lights, TypeGpuLight))
	{
		gpuLights[numGpuLights++] = lights.GetPointer<Gpu::Light>();
	}

	if(ScriptTypes::CheckPtrType(surface, TypeGpuDrawSurface))
	{
		gpuSurface = surface.GetPointer<Gpu::DrawSurface>();
	}
	if(ScriptTypes::CheckPtrType(instances, TypeGpuInstanceBuffer))
	{
		gpuInstances = instances.GetPointer<Gpu::InstanceBuffer>();
	}
	if(ScriptTypes::CheckPtrType(effect, TypeGpuEffect))
	{
		gpuOverrideEffect = effect.GetPointer<Gpu::Effect>();
	}

	if(gpuModel)
	{
		Gpu::Api * gpu = interpreter->GetApp()->gpu;
		gpuModel->instances = gpuInstances;
		gpuModel->BeDrawn(gpu, gpuCamera, gpuLights, numGpuLights, gpuSurface, gpuOverrideEffect);
	}
}

void GpuScriptCallbacks::Compute(ScriptInterpreter * interpreter)
{
	POP_PTRPARAM(1, effect, TypeGpuEffect);
	POP_NUMPARAM(2, xGroups);
	ScriptParam yGroups = interpreter->PopParam();
	ScriptParam zGroups = interpreter->PopParam();
	interpreter->ClearParams();

	interpreter->GetApp()->gpu->Compute(effect.GetPointer<Gpu::Effect>(), unsigned(xGroups.nvalue),
		yGroups.IsNumber() ? unsigned(yGroups.nvalue) : 1, zGroups.IsNumber() ? unsigned(zGroups.nvalue) : 1);
}

void GpuScriptCallbacks::DrawIndirect(ScriptInterpreter * interpreter)
{
	POP_PTRPARAM(1, effect, TypeGpuEffect);
	ScriptParam vertices = interpreter->PopParam();
	ScriptParam instances = interpreter->PopParam();
	ScriptParam surface = interpreter->PopParam();
	interpreter->ClearParams();

	Gpu::ParamBuffer * gpuVertices = 0;
	Gpu::ParamBuffer * gpuInstances = 0;
	Gpu::DrawSurface * gpuSurface = 0;

	if(ScriptTypes::CheckPtrType(vertices, TypeGpuParamBuffer))
	{
		gpuVertices = vertices.GetPointer<Gpu::ParamBuffer>();
	}
	if(ScriptTypes::CheckPtrType(instances, TypeGpuParamBuffer))
	{
		gpuInstances = instances.GetPointer<Gpu::ParamBuffer>();
	}
	if(ScriptTypes::CheckPtrType(surface, TypeGpuDrawSurface))
	{
		gpuSurface = surface.GetPointer<Gpu::DrawSurface>();
	}

	interpreter->GetApp()->gpu->DrawIndirect(effect.GetPointer<Gpu::Effect>(), gpuVertices, gpuInstances, gpuSurface);
}

void GpuScriptCallbacks::SetModelPosition(ScriptInterpreter * interpreter)
{
	POP_PTRPARAM(1,model,TypeGpuComplexModel);
	POP_NUMPARAM(2,x);
	POP_NUMPARAM(3,y);
	POP_NUMPARAM(4,z);

	Gpu::ComplexModel * gpuModel = model.GetPointer<Gpu::ComplexModel>();

	if(!gpuModel) return;

	gpuModel->position.x = (float) x.nvalue;
	gpuModel->position.y = (float) y.nvalue;
	gpuModel->position.z = (float) z.nvalue;
}

void GpuScriptCallbacks::SetModelRotation(ScriptInterpreter * interpreter)
{
	POP_PTRPARAM(1,model,TypeGpuComplexModel);
	POP_NUMPARAM(2,x);
	POP_NUMPARAM(3,y);
	POP_NUMPARAM(4,z);

	Gpu::ComplexModel * gpuModel = model.GetPointer<Gpu::ComplexModel>();

	if(!gpuModel) return;

	gpuModel->rotation.x = (float) x.nvalue;
	gpuModel->rotation.y = (float) y.nvalue;
	gpuModel->rotation.z = (float) z.nvalue;
}

void GpuScriptCallbacks::SetModelScale(ScriptInterpreter * interpreter)
{
	POP_PTRPARAM(1,model,TypeGpuComplexModel);
	POP_NUMPARAM(2,scale);
	ScriptParam scaleY = interpreter->PopParam();
	ScriptParam scaleZ = interpreter->PopParam();

	Gpu::ComplexModel * gpuModel = model.GetPointer<Gpu::ComplexModel>();

	if(scaleY.IsNumber() && scaleZ.IsNumber())
	{
		gpuModel->scale.x = (float)scale.nvalue;
		gpuModel->scale.y = (float)scaleY.nvalue;
		gpuModel->scale.z = (float)scaleZ.nvalue;
	}
	else
	{
		gpuModel->scale = glm::vec4((float)scale.nvalue, (float)scale.nvalue, (float)scale.nvalue, 1.0f);
	}
}

void GpuScriptCallbacks::SetModelMatrix(ScriptInterpreter * interpreter)
{
	POP_PTRPARAM(1, model, TypeGpuComplexModel);
	POP_SPTRPARAM(3, matrix, TypeMatrix4);
	interpreter->ClearParams();

	Gpu::ComplexModel * gpuModel = model.GetPointer<Gpu::ComplexModel>();
	glm::mat4 * meshMatrix = matrix.GetPointer<glm::mat4>();
	gpuModel->SetMatrix(*meshMatrix);
}

void GpuScriptCallbacks::GetFont(ScriptInterpreter * interpreter)
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

	if(font) interpreter->PushParam(ScriptParam(font, ScriptTypes::GetHandle(TypeGpuFont)));
}

void GpuScriptCallbacks::DrawText(ScriptInterpreter * interpreter)
{
	POP_PTRPARAM(1,font,TypeGpuFont);
	POP_PARAM(2,text,STRING);
	POP_NUMPARAM(3,x);
	POP_NUMPARAM(4,y);
	ScriptParam center = interpreter->PopParam();
	ScriptParam surface = interpreter->PopParam();

	bool centered = false;

	if(center.type != ScriptParam::NONE) centered = center.nvalue > 0.0;

	Gpu::Font * gpuFont = font.GetPointer<Gpu::Font>();

	std::string shortText(text.svalue);
	std::wstring wideText(shortText.begin(),shortText.end());

	Gpu::DrawSurface * drawSurface = 0;
	if(ScriptTypes::CheckPtrType(surface, TypeGpuDrawSurface))
	{
		drawSurface = surface.GetPointer<Gpu::DrawSurface>();
	}

	Gpu::Api * gpu = interpreter->GetApp()->gpu;
	gpu->DrawGpuText(gpuFont,wideText.c_str(),(float)x.nvalue,(float)y.nvalue,centered,drawSurface);
}

void GpuScriptCallbacks::SetFontColor(ScriptInterpreter * interpreter)
{
	POP_PTRPARAM(1,font,TypeGpuFont);
	POP_NUMPARAM(2,r);
	POP_NUMPARAM(3,g);
	POP_NUMPARAM(4,b);
	ScriptParam a = interpreter->PopParam();

	Gpu::Font * gpuFont = font.GetPointer<Gpu::Font>();

	gpuFont->color.r = float(r.nvalue);
	gpuFont->color.g = float(g.nvalue);
	gpuFont->color.b = float(b.nvalue);
	if(a.type == ScriptParam::DOUBLE) gpuFont->color.a = float(a.nvalue);
}

void GpuScriptCallbacks::CreateLight(ScriptInterpreter * interpreter)
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

	interpreter->PushParam(ScriptParam(result, ScriptTypes::GetHandle(TypeGpuLight)));
}

void GpuScriptCallbacks::SetLightColor(ScriptInterpreter * interpreter)
{
	POP_PTRPARAM(1,light,TypeGpuLight);
	POP_NUMPARAM(2,r);
	POP_NUMPARAM(3,g);
	POP_NUMPARAM(4,b);

	Gpu::Light * gpuLight = light.GetPointer<Gpu::Light>();

	if(!gpuLight) return;

	gpuLight->color.r = (float)r.nvalue;
	gpuLight->color.g = (float)g.nvalue;
	gpuLight->color.b = (float)b.nvalue;
}

void GpuScriptCallbacks::SetLightPosition(ScriptInterpreter * interpreter)
{
	POP_PTRPARAM(1,light,TypeGpuLight);
	POP_NUMPARAM(2,x);
	POP_NUMPARAM(3,y);
	POP_NUMPARAM(4,z);
	
	Gpu::Light * gpuLight = light.GetPointer<Gpu::Light>();

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

void GpuScriptCallbacks::SetLightDirection(ScriptInterpreter * interpreter)
{
	POP_PTRPARAM(1,light,TypeGpuLight);
	POP_NUMPARAM(2,x);
	POP_NUMPARAM(3,y);
	POP_NUMPARAM(4,z);

	Gpu::Light * gpuLight = light.GetPointer<Gpu::Light>();

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

void GpuScriptCallbacks::SetLightAttenuation(ScriptInterpreter * interpreter)
{
	POP_PTRPARAM(1,light,TypeGpuLight);
	POP_NUMPARAM(2,atten);

	Gpu::Light * gpuLight = light.GetPointer<Gpu::Light>();

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

void GpuScriptCallbacks::SetLightSpotPower(ScriptInterpreter * interpreter)
{
	POP_PTRPARAM(1, light, TypeGpuLight);
	POP_NUMPARAM(2, power);

	Gpu::Light * gpuLight = light.GetPointer<Gpu::Light>();
	if(gpuLight->GetType() == Gpu::LightType_Spot)
	{
		Gpu::SpotLight * spotlight = static_cast<Gpu::SpotLight*>(gpuLight);
		spotlight->power = (float)power.nvalue;
	}
}

void GpuScriptCallbacks::SetMeshPosition(ScriptInterpreter * interpreter)
{
	POP_PTRPARAM(1,model,TypeGpuComplexModel);
	POP_NUMPARAM(2,meshnum);
	POP_NUMPARAM(3,x);
	POP_NUMPARAM(4,y);
	POP_NUMPARAM(5,z);
	interpreter->ClearParams();

	Gpu::ComplexModel * gpuModel = model.GetPointer<Gpu::ComplexModel>();
	unsigned meshnumber = (unsigned) meshnum.nvalue;
	gpuModel->models[meshnumber].position = glm::vec4(float(x.nvalue),float(y.nvalue),float(z.nvalue),1.0f);
}

void GpuScriptCallbacks::SetMeshRotation(ScriptInterpreter * interpreter)
{
	POP_PTRPARAM(1, model, TypeGpuComplexModel);
	POP_NUMPARAM(2, meshnum);
	POP_NUMPARAM(3, x);
	POP_NUMPARAM(4, y);
	POP_NUMPARAM(5, z);
	interpreter->ClearParams();

	Gpu::ComplexModel * gpuModel = model.GetPointer<Gpu::ComplexModel>();
	unsigned meshnumber = (unsigned)meshnum.nvalue;
	gpuModel->models[meshnumber].rotation = glm::vec4(float(x.nvalue), float(y.nvalue), float(z.nvalue), 0.0f);
}

void GpuScriptCallbacks::SetMeshScale(ScriptInterpreter * interpreter)
{
	POP_PTRPARAM(1, model, TypeGpuComplexModel);
	POP_NUMPARAM(2, meshnum);
	POP_NUMPARAM(3, scale);
	ScriptParam scaleY = interpreter->PopParam();
	ScriptParam scaleZ = interpreter->PopParam();

	Gpu::ComplexModel * gpuModel = model.GetPointer<Gpu::ComplexModel>();

	unsigned index = unsigned(meshnum.nvalue);

	if(index < gpuModel->numModels)
	{
		if(scaleY.IsNumber() && scaleZ.IsNumber())
		{
			gpuModel->models[index].scale.x = (float)scale.nvalue;
			gpuModel->models[index].scale.y = (float)scaleY.nvalue;
			gpuModel->models[index].scale.z = (float)scaleZ.nvalue;
		}
		else
		{
			gpuModel->models[index].scale = glm::vec4((float)scale.nvalue, (float)scale.nvalue, (float)scale.nvalue, 1.0f);
		}
	}
}

void GpuScriptCallbacks::SetMeshMatrix(ScriptInterpreter * interpreter)
{
	POP_PTRPARAM(1, model, TypeGpuComplexModel);
	POP_NUMPARAM(2, meshnum);
	POP_SPTRPARAM(3, matrix, TypeMatrix4);
	interpreter->ClearParams();

	Gpu::ComplexModel * gpuModel = model.GetPointer<Gpu::ComplexModel>();
	glm::mat4 * meshMatrix = matrix.GetPointer<glm::mat4>();
	unsigned meshNumber = unsigned(meshnum.nvalue);
	gpuModel->models[meshNumber].SetMatrix(*meshMatrix);
}

void GpuScriptCallbacks::SetMeshEffect(ScriptInterpreter * interpreter)
{
	POP_PTRPARAM(1,model,TypeGpuComplexModel);
	POP_NUMPARAM(2,meshnum);
	ScriptParam effect = interpreter->PopParam();

	Gpu::ComplexModel * gpuModel = model.GetPointer<Gpu::ComplexModel>();
	unsigned meshnumber = (unsigned) meshnum.nvalue;
	Gpu::Effect * gpuEffect = 0;

	if(meshnumber < gpuModel->numModels)
	{
		if(ScriptTypes::CheckPtrType(effect, TypeGpuEffect))
		{
			gpuEffect = effect.GetPointer<Gpu::Effect>();
		}
		gpuModel->models[meshnumber].effect = gpuEffect;
	}
}

void GpuScriptCallbacks::CreateEffect(ScriptInterpreter * interpreter)
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
		if(!ScriptTypes::CheckPtrType(nameOrShader,TypeGpuShader))
		{
			interpreter->ThrowError("CreateEffect: Pointer parameter is not of type GpuShader");
			return;
		}
		shader = nameOrShader.GetPointer<Gpu::Shader>();
	}

	if(shader)
	{
		Gpu::Effect * effect = new Gpu::Effect(shader);

		interpreter->PushParam(ScriptParam(effect, ScriptTypes::GetHandle(TypeGpuEffect)));
	}
}

void GpuScriptCallbacks::SetMeshTexture(ScriptInterpreter * interpreter)
{
	POP_PTRPARAM(1,model,TypeGpuComplexModel);
	POP_NUMPARAM(2,meshnum);
	POP_PTRPARAM(3,texture,TypeGpuTexture);

	Gpu::ComplexModel * gpuModel = model.GetPointer<Gpu::ComplexModel>();
	Gpu::Texture * gpuTexture = texture.GetPointer<Gpu::Texture>();
	unsigned meshnumber = (unsigned) meshnum.nvalue;

	if(meshnumber < gpuModel->numModels)
	{
		gpuModel->models[meshnumber].texture = gpuTexture;
	}
}

void GpuScriptCallbacks::SetMeshNormal(ScriptInterpreter * interpreter)
{
	POP_PTRPARAM(1,model,TypeGpuComplexModel);
	POP_NUMPARAM(2,meshnum);
	POP_PTRPARAM(3,normal,TypeGpuTexture);

	Gpu::ComplexModel * gpuModel = model.GetPointer<Gpu::ComplexModel>();
	Gpu::Texture * gpuNormal = normal.GetPointer<Gpu::Texture>();
	unsigned meshnumber = (unsigned) meshnum.nvalue;

	if(meshnumber < gpuModel->numModels)
	{
		gpuModel->models[meshnumber].normalMap = gpuNormal;
	}
}

void GpuScriptCallbacks::SetMeshCubeMap(ScriptInterpreter * interpreter)
{
	POP_PTRPARAM(1,model,TypeGpuComplexModel);
	POP_NUMPARAM(2,meshnum);
	POP_PTRPARAM(3,cubemap,TypeGpuCubeMap);

	Gpu::ComplexModel * gpuModel = model.GetPointer<Gpu::ComplexModel>();
	Gpu::CubeMap * gpuCubeMap = cubemap.GetPointer<Gpu::CubeMap>();
	unsigned meshnumber = (unsigned) meshnum.nvalue;

	if(meshnumber < gpuModel->numModels)
	{
		gpuModel->models[meshnumber].cubeMap = gpuCubeMap;
	}
}

void GpuScriptCallbacks::SetMeshColor(ScriptInterpreter * interpreter)
{
	POP_PTRPARAM(1,model,TypeGpuComplexModel);
	POP_NUMPARAM(2,meshnum);
	POP_NUMPARAM(3,r);
	POP_NUMPARAM(4,g);
	POP_NUMPARAM(5,b);
	ScriptParam a = interpreter->PopParam();

	Gpu::ComplexModel * gpuModel = model.GetPointer<Gpu::ComplexModel>();
	unsigned meshnumber = (unsigned) meshnum.nvalue;

	if(meshnumber < gpuModel->numModels)
	{
		gpuModel->models[meshnumber].color.r = (float) r.nvalue;
		gpuModel->models[meshnumber].color.g = (float) g.nvalue;
		gpuModel->models[meshnumber].color.b = (float) b.nvalue;
		if(a.type == ScriptParam::DOUBLE)
		{
			gpuModel->models[meshnumber].color.a = (float) a.nvalue;
		}
	}
}

void GpuScriptCallbacks::SetMeshSpecular(ScriptInterpreter * interpreter)
{
	POP_PTRPARAM(1,model,TypeGpuComplexModel);
	POP_NUMPARAM(2,meshnum);
	POP_NUMPARAM(3,specular);

	Gpu::ComplexModel * gpuModel = model.GetPointer<Gpu::ComplexModel>();
	unsigned meshnumber = (unsigned) meshnum.nvalue;

	if(meshnumber < gpuModel->numModels)
	{
		gpuModel->models[meshnumber].specPower = (float) specular.nvalue;
	}
}

void GpuScriptCallbacks::SetMeshFactors(ScriptInterpreter * interpreter)
{
	POP_PTRPARAM(1, model, TypeGpuComplexModel);
	POP_NUMPARAM(2, meshnum);
	POP_NUMPARAM(3, diffuse);
	POP_NUMPARAM(3, specular);

	Gpu::ComplexModel * gpuModel = model.GetPointer<Gpu::ComplexModel>();
	unsigned meshnumber = (unsigned)meshnum.nvalue;

	if(meshnumber < gpuModel->numModels)
	{
		gpuModel->models[meshnumber].diffuseFactor = (float)diffuse.nvalue;
		gpuModel->models[meshnumber].specFactor = (float)specular.nvalue;
	}
}

void GpuScriptCallbacks::GetMeshBounds(ScriptInterpreter * interpreter)
{
	POP_PTRPARAM(1, model, TypeGpuComplexModel);
	POP_NUMPARAM(2, meshnum);
	interpreter->ClearParams();

	Gpu::ComplexModel * gpuModel = model.GetPointer<Gpu::ComplexModel>();
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

void GpuScriptCallbacks::GetNumMeshes(ScriptInterpreter * interpreter)
{
	POP_PTRPARAM(1,model,TypeGpuComplexModel);

	Gpu::ComplexModel * gpuModel = model.GetPointer<Gpu::ComplexModel>();
	unsigned numMeshes = gpuModel->numModels;

	interpreter->PushParam(ScriptParam(ScriptParam::INT,numMeshes));
}

void GpuScriptCallbacks::SetEffectParam(ScriptInterpreter * interpreter)
{
	POP_PTRPARAM(1,effect,TypeGpuEffect);
	POP_NUMPARAM(2,paramnum);
	ScriptParam value = interpreter->PopParam();

	Gpu::Effect * gpuEffect = effect.GetPointer<Gpu::Effect>();

	if(!gpuEffect) return;

	unsigned paramIndex = (unsigned) paramnum.nvalue;

	if(value.type == ScriptParam::NONE)
	{
		interpreter->ThrowError("Attempted to set null parameter to effect");
	}
	if(value.type == ScriptParam::DOUBLE)
	{
		gpuEffect->SetParam(paramIndex,(float)value.nvalue);
	}
	if(value.type == ScriptParam::POINTER)
	{
		if(value.pvalue->type == ScriptTypes::GetHandle(TypeGpuTexture))
		{
			gpuEffect->SetParam(paramIndex, value.GetPointer<Gpu::Texture>());
		}
		else if(value.pvalue->type == ScriptTypes::GetHandle(TypeGpuCubeMap))
		{
			gpuEffect->SetParam(paramIndex, value.GetPointer<Gpu::CubeMap>());
		}
		else if(value.pvalue->type == ScriptTypes::GetHandle(TypeGpuVolumeTexture))
		{
			gpuEffect->SetParam(paramIndex, value.GetPointer<Gpu::VolumeTexture>());
		}
		else if(value.pvalue->type == ScriptTypes::GetHandle(TypeGpuDrawSurface))
		{
			gpuEffect->SetParam(paramIndex, value.GetPointer<Gpu::DrawSurface>());
		}
		else if(value.pvalue->type == ScriptTypes::GetHandle(TypeGpuParamBuffer))
		{
			gpuEffect->SetParam(paramIndex, value.GetPointer<Gpu::ParamBuffer>());
		}
		else if(value.pvalue->type == ScriptTypes::GetHandle(TypeFloatArray))
		{
			gpuEffect->SetParam(paramIndex, &(value.GetPointer<ScriptFloatArray>()->gpuFloatArray));
		}
		else
		{
			// Warn ?
			std::stringstream error;
			error << "Could not identify type of effect parameter " << paramIndex;
			interpreter->ThrowError(error.str().c_str());
		}
	}
}

void GpuScriptCallbacks::SetSamplerParam(ScriptInterpreter * interpreter)
{
	POP_PTRPARAM(1, effect, TypeGpuEffect);
	POP_PARAM(2, key, STRING);
	POP_NUMPARAM(3, value);
	ScriptParam index = interpreter->PopParam();

	Gpu::Effect * gpuEffect = effect.GetPointer<Gpu::Effect>();

	if(!gpuEffect) return;

	static const char * samplerParamNames[Gpu::SamplerParam::KeyMAX] =
	{
		"addressU",
		"addressV",
		"addressW",
		"filter",
		"anisotropy",
		"comparison",
		"mipDisable"
	};

	Gpu::SamplerParam::ParamKey paramKey = Gpu::SamplerParam::KeyMAX;

	for(unsigned i = 0; i < Gpu::SamplerParam::KeyMAX; ++i)
	{
		if(strcmp(key.svalue, samplerParamNames[i]) == 0)
		{
			paramKey = (Gpu::SamplerParam::ParamKey) i;
			break;
		}
	}

	unsigned paramIndex = Gpu::SamplerParam::DEFAULT_SAMPLER;
	if(index.IsNumber())
	{
		paramIndex = unsigned(index.nvalue);
	}
	
	if(paramKey < Gpu::SamplerParam::KeyMAX)
	{
		gpuEffect->SetSamplerParam(paramKey, unsigned(value.nvalue), paramIndex);
	}
}

void GpuScriptCallbacks::CreateParamBuffer(ScriptInterpreter * interpreter)
{
	POP_NUMPARAM(1, numElements);
	ScriptParam stride = interpreter->PopParam();
	ScriptParam append = interpreter->PopParam();
	interpreter->ClearParams();

	Gpu::ParamBuffer * paramBuffer = interpreter->GetApp()->gpu->CreateParamBuffer(
		unsigned(numElements.nvalue), 0, 
		stride.IsNumber() ? unsigned(stride.nvalue) : 0,
		append.type == ScriptParam::BOOL && append.nvalue > 0.0 ? 0 : -1);

	interpreter->PushParam(ScriptParam(paramBuffer, ScriptTypes::GetHandle(TypeGpuParamBuffer)));
}

void GpuScriptCallbacks::CreateSurface(ScriptInterpreter * interpreter)
{
	static const char * surfaceTypeStrings[Gpu::DrawSurface::Format_Total] = {
		"4x8i",
		"4x16f",
		"3x10f",
		"1x16f",
		"typeless"
	};

	ScriptParam width = interpreter->PopParam();
	ScriptParam height = interpreter->PopParam();
	ScriptParam window = interpreter->PopParam();
	ScriptParam format = interpreter->PopParam();

	Gpu::DrawSurface * surface;
	Gpu::DrawSurface::Format surfaceFormat = Gpu::DrawSurface::Format_4x8int;

	if(format.type == ScriptParam::STRING)
	{
		for(unsigned i = 0; i < Gpu::DrawSurface::Format_Total; ++i)
		{
			if(strcmp(format.svalue, surfaceTypeStrings[i]) == 0)
			{
				surfaceFormat = (Gpu::DrawSurface::Format) i;
				break;
			}
		}
	}

	if(width.type == ScriptParam::DOUBLE && height.type == ScriptParam::DOUBLE)
	{
		if(ScriptTypes::CheckPtrType(window, TypePlatformWindow))
		{
			surface = interpreter->GetApp()->gpu->CreateRelativeDrawSurface(
				window.GetPointer<PlatformWindow>(),
				float(width.nvalue), 
				float(height.nvalue),
				surfaceFormat);
		}
		else
		{
			surface = interpreter->GetApp()->gpu->CreateDrawSurface(
				(unsigned)width.nvalue,
				(unsigned)height.nvalue,
				surfaceFormat);
		}
	}
	else
	{
		surface = interpreter->GetApp()->gpu->CreateRelativeDrawSurface(
			interpreter->GetApp()->platform->GetMainPlatformWindow());
	}

	interpreter->PushParam(ScriptParam(surface, ScriptTypes::GetHandle(TypeGpuDrawSurface)));
};

void GpuScriptCallbacks::DrawSurface(ScriptInterpreter * interpreter) 
{
	POP_PTRPARAM(1,surface,TypeGpuDrawSurface);
	ScriptParam effect = interpreter->PopParam();
	ScriptParam outSurface = interpreter->PopParam();

	Gpu::DrawSurface * gpuSurface = surface.GetPointer<Gpu::DrawSurface>();
	Gpu::Effect * gpuEffect = 0;
	Gpu::DrawSurface * gpuOutSurface = 0;

	if(ScriptTypes::CheckPtrType(effect, TypeGpuEffect))
	{
		gpuEffect = effect.GetPointer<Gpu::Effect>();
	}
	if(ScriptTypes::CheckPtrType(outSurface, TypeGpuDrawSurface))
	{
		gpuOutSurface = outSurface.GetPointer<Gpu::DrawSurface>();
	}
	
	interpreter->GetApp()->gpu->DrawGpuSurface(gpuSurface, gpuEffect, gpuOutSurface);
}

void GpuScriptCallbacks::ClearSurface(ScriptInterpreter * interpreter)
{
	POP_PTRPARAM(1,surface,TypeGpuDrawSurface);
	ScriptParam r = interpreter->PopParam();
	ScriptParam g = interpreter->PopParam();
	ScriptParam b = interpreter->PopParam();
	ScriptParam a = interpreter->PopParam();
	interpreter->ClearParams();

	Gpu::DrawSurface * gpuSurface = surface.GetPointer<Gpu::DrawSurface>();

	if(r.IsNumber() && g.IsNumber() && b.IsNumber())
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

void GpuScriptCallbacks::GetSurfaceTexture(ScriptInterpreter * interpreter)
{
	POP_PTRPARAM(1,surface,TypeGpuDrawSurface);

	Gpu::DrawSurface * gpuSurface = surface.GetPointer<Gpu::DrawSurface>();

	Gpu::Texture * texture = gpuSurface->GetTexture();

	interpreter->ClearParams();
	interpreter->PushParam(ScriptParam(new NonDeletingPtr<true>(texture, ScriptTypes::GetHandle(TypeGpuTexture))));
}

void GpuScriptCallbacks::CreateModel(ScriptInterpreter * interpreter)
{
	POP_PARAM(1,type,STRING);
	ScriptParam vertices = interpreter->PopParam();
	ScriptParam indices = interpreter->PopParam();

	LocalMesh * localMesh = FloatsToLocalMesh(interpreter, type, vertices, indices);

	if(localMesh)
	{
		Gpu::Mesh * mesh = interpreter->GetApp()->gpu->CreateGpuMesh(localMesh->vertexBuffer, localMesh->numTriangles, localMesh->indexBuffer);

		if(mesh)
		{
			Gpu::ComplexModel * complexModel = new Gpu::ComplexModel(1);
			complexModel->models[0].mesh = mesh;
			complexModel->models[0].destructMesh = true;

			interpreter->PushParam(ScriptParam(complexModel, ScriptTypes::GetHandle(TypeGpuComplexModel)));
		}

		delete localMesh;
	}
}

void GpuScriptCallbacks::CreateSpriteModel(ScriptInterpreter * interpreter)
{
	POP_PTRPARAM(1, texture, TypeGpuTexture);
	ScriptParam yUpwards = interpreter->PopParam();

	Gpu::Mesh * gpuMesh = interpreter->GetApp()->sprites->GetQuadMesh();
	Gpu::Texture * gpuTexture = texture.GetPointer<Gpu::Texture>();

	Gpu::ComplexModel * complexModel = new Gpu::ComplexModel(1);
	complexModel->models[0].mesh = gpuMesh;
	complexModel->models[0].texture = gpuTexture;
	complexModel->models[0].scale = glm::vec4(gpuTexture->GetWidth(), gpuTexture->GetHeight(), 1.0f, 1.0f);

	if(yUpwards.type == ScriptParam::BOOL && yUpwards.nvalue <= 0.0)
	{
		complexModel->models[0].scale.y *= -1.0f;
		complexModel->models[0].position.y = float(gpuTexture->GetHeight());
	}

	interpreter->PushParam(ScriptParam(complexModel, ScriptTypes::GetHandle(TypeGpuComplexModel)));
}

void GpuScriptCallbacks::DecomposeModel(ScriptInterpreter * interpreter)
{
	POP_PTRPARAM(1, model, TypeGpuComplexModel);

	Gpu::ComplexModel * gpuModel = model.GetPointer<Gpu::ComplexModel>();

	ScriptParam table = interpreter->CreateMap();

	for(unsigned i = 0; i < gpuModel->numModels; ++i)
	{
		Gpu::ComplexModel * newModel = new Gpu::ComplexModel(1);
		newModel->models[0] = gpuModel->models[i];

		gpuModel->ApplyTransform(&(newModel->models[0]));

		newModel->models[0].destructMesh = false;
		newModel->models[0].destructEffect = false;

		interpreter->SetMapNext(table,
			ScriptParam(ScriptParam::INT, double(i)),
			ScriptParam(newModel, ScriptTypes::GetHandle(TypeGpuComplexModel)));
	}

	interpreter->PushParam(table);
}

void GpuScriptCallbacks::SetClearColor(ScriptInterpreter * interpreter)
{
	POP_NUMPARAM(1,r);
	POP_NUMPARAM(2,g);
	POP_NUMPARAM(3,b);
	ScriptParam a = interpreter->PopParam();

	float alpha = 1.0f;
	if(a.type == ScriptParam::DOUBLE)
	{
		alpha = (float) a.nvalue;
	}

	interpreter->GetApp()->gpu->SetClearColor(
		(float) r.nvalue,
		(float) g.nvalue,
		(float) b.nvalue,
		alpha);
}

void GpuScriptCallbacks::SetBlendMode(ScriptInterpreter * interpreter)
{
	POP_PARAM(1,mode,STRING);

	const char * modeChars = mode.svalue;
	bool success = false;

	if(_stricmp(modeChars, "alpha") == 0)
	{
		interpreter->GetApp()->gpu->SetBlendMode(Gpu::BlendMode_Alpha);
		success = true;
	}
	if(_stricmp(modeChars, "additive") == 0)
	{
		interpreter->GetApp()->gpu->SetBlendMode(Gpu::BlendMode_Additive);
		success = true;
	}
	if(_stricmp(modeChars, "none") == 0)
	{
		interpreter->GetApp()->gpu->SetBlendMode(Gpu::BlendMode_None);
		success = true;
	}

	if(!success)
	{
		interpreter->ThrowError("Unrecognized blend mode!");
	}
}

void GpuScriptCallbacks::SetDepthMode(ScriptInterpreter * interpreter)
{
	POP_PARAM(1, mode, STRING);
	const char * modeChars = mode.svalue;

	static const char * depthModes[Gpu::DepthMode_Count] =
	{
		"readwrite",
		"read",
		"write",
		"none"
	};

	for(unsigned i = 0; i < Gpu::DepthMode_Count; ++i)
	{
		if(_stricmp(modeChars, depthModes[i]) == 0)
		{
			interpreter->GetApp()->gpu->SetDepthMode((Gpu::DepthMode)i);
			return;
		}
	}
	interpreter->ThrowError("Unrecognized depth mode!");
}

void GpuScriptCallbacks::SetWireframe(ScriptInterpreter * interpreter)
{
	POP_NUMPARAM(1,wireframe);

	interpreter->GetApp()->gpu->SetDrawWireframe(wireframe.nvalue > 0.0f);
}

void GpuScriptCallbacks::SetMultisampling(ScriptInterpreter * interpreter)
{
	POP_NUMPARAM(1,level);

	interpreter->GetApp()->gpu->SetMultisampling(unsigned(level.nvalue));
}

void GpuScriptCallbacks::SetAnisotropy(ScriptInterpreter * interpreter)
{
	POP_NUMPARAM(1, level);
	
	interpreter->GetApp()->gpu->SetAnisotropy(unsigned(level.nvalue));
}

void GpuScriptCallbacks::GetBaseEffect(ScriptInterpreter * interpreter)
{
	Gpu::Effect * effect = interpreter->GetApp()->gpu->GetBaseEffect();

	interpreter->PushParam(ScriptParam(new NonDeletingPtr<false>(effect, ScriptTypes::GetHandle(TypeGpuEffect))));
}

void GpuScriptCallbacks::GetScreenSize(ScriptInterpreter * interpreter)
{
	ScriptParam windowOrIndex = interpreter->PopParam();
	interpreter->ClearParams();

	unsigned width = 0, height = 0;
	const PlatformApi * platform = interpreter->GetApp()->platform;

	if(ScriptTypes::CheckPtrType(windowOrIndex, TypePlatformWindow))
	{
		platform->GetScreenSize(width, height, windowOrIndex.GetPointer<PlatformWindow>());
	}
	else if(windowOrIndex.IsNumber())
	{
		platform->GetScreenSize(width, height, unsigned(windowOrIndex.nvalue));
	}
	else
	{
		platform->GetScreenSize(width, height);
	}

	interpreter->PushParam(ScriptParam(ScriptParam::INT, width));
	interpreter->PushParam(ScriptParam(ScriptParam::INT, height));
}

void GpuScriptCallbacks::GetBackbufferSize(ScriptInterpreter * interpreter)
{
	ScriptParam window = interpreter->PopParam();
	PlatformWindow * platformWindow = 0;
	if(window.CheckPointer(ScriptTypes::GetHandle(TypePlatformWindow)))
	{
		platformWindow = window.GetPointer<PlatformWindow>();
	}

	unsigned width, height;
	interpreter->GetApp()->gpu->GetBackbufferSize(width, height, platformWindow);

	interpreter->ClearParams();

	interpreter->PushParam(ScriptParam(ScriptParam::INT, width));
	interpreter->PushParam(ScriptParam(ScriptParam::INT, height));
}

void GpuScriptCallbacks::CreateRect(ScriptInterpreter * interpreter)
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

	if(ScriptTypes::CheckPtrType(updateRect, TypeGpuComplexModel))
	{
		gpuModel = updateRect.GetPointer<Gpu::ComplexModel>();
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

		interpreter->PushParam(ScriptParam(gpuModel, ScriptTypes::GetHandle(TypeGpuComplexModel)));
	}

	delete localMesh;
}

void GpuScriptCallbacks::CreateRectStroke(ScriptInterpreter * interpreter)
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

	if(ScriptTypes::CheckPtrType(updateRect, TypeGpuComplexModel))
	{
		gpuModel = updateRect.GetPointer<Gpu::ComplexModel>();
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

		interpreter->PushParam(ScriptParam(gpuModel, ScriptTypes::GetHandle(TypeGpuComplexModel)));
	}

	delete localMesh;
}

void GpuScriptCallbacks::CreateEllipse(ScriptInterpreter * interpreter)
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

		if(ScriptTypes::CheckPtrType(updateEllipse,TypeGpuComplexModel))
		{
			gpuModel = updateEllipse.GetPointer<Gpu::ComplexModel>();
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

			interpreter->PushParam(ScriptParam(gpuModel, ScriptTypes::GetHandle(TypeGpuComplexModel)));
		}

		delete localMesh;
	}
}

void GpuScriptCallbacks::CreateEllipseStroke(ScriptInterpreter * interpreter)
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

	if(ScriptTypes::CheckPtrType(updateEllipse, TypeGpuComplexModel))
	{
		gpuModel = updateEllipse.GetPointer<Gpu::ComplexModel>();
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

		interpreter->PushParam(ScriptParam(gpuModel, ScriptTypes::GetHandle(TypeGpuComplexModel)));
	}

	delete localMesh;
}

void GpuScriptCallbacks::CreateIsoSurface(ScriptInterpreter * interpreter)
{
	POP_NUMPARAM(1, gridSize);
	ScriptParam threshold = interpreter->PopParam();

	if(gridSize.nvalue < 2.0f)
		interpreter->ThrowError("Cannot create IsoSurface, grid size is too small!");

	IsoSurface * isoSurface = new IsoSurface(unsigned(gridSize.nvalue), interpreter->GetApp()->gpu);
	if(threshold.IsNumber()) isoSurface->SetThreshold(float(threshold.nvalue));

	interpreter->ClearParams();
	interpreter->PushParam(ScriptParam(isoSurface, ScriptTypes::GetHandle(TypeIsoSurface)));
}

void GpuScriptCallbacks::AddIsoSurfaceBall(ScriptInterpreter * interpreter)
{
	POP_PTRPARAM(1, iso, TypeIsoSurface);
	POP_NUMPARAM(2, x);
	POP_NUMPARAM(3, y);
	POP_NUMPARAM(4, z);
	POP_NUMPARAM(5, r);

	IsoSurface * isoSurface = iso.GetPointer<IsoSurface>();

	glm::vec3 startingVector(float(x.nvalue), float(y.nvalue), float(z.nvalue));
	float radius = float(r.nvalue);
	isoSurface->AddMetaball(startingVector, radius * radius);
}
void GpuScriptCallbacks::AddIsoSurfacePlane(ScriptInterpreter * interpreter)
{
	POP_PTRPARAM(1, iso, TypeIsoSurface);
	POP_NUMPARAM(2, x);
	POP_NUMPARAM(3, y);
	POP_NUMPARAM(4, z);
	POP_NUMPARAM(5, nx);
	POP_NUMPARAM(6, ny);
	POP_NUMPARAM(7, nz);

	IsoSurface * isoSurface = iso.GetPointer<IsoSurface>();

	glm::vec3 pos(float(x.nvalue), float(y.nvalue), float(z.nvalue));
	glm::vec3 nor(float(nx.nvalue), float(ny.nvalue), float(nz.nvalue));
	isoSurface->AddMetaPlane(pos, nor);
}

void GpuScriptCallbacks::ClearIsoSurface(ScriptInterpreter * interpreter)
{
	POP_PTRPARAM(1, iso, TypeIsoSurface);
	IsoSurface * isoSurface = iso.GetPointer<IsoSurface>();

	isoSurface->Clear();
}

void GpuScriptCallbacks::GetIsoSurfaceModel(ScriptInterpreter * interpreter)
{
	POP_PTRPARAM(1, iso, TypeIsoSurface);
	IsoSurface * isoSurface = iso.GetPointer<IsoSurface>();

	isoSurface->UpdateObjects();
	isoSurface->UpdateMesh(interpreter->GetApp()->gpu);

	Gpu::Mesh * surfaceMesh = isoSurface->GetMesh();
	Gpu::ComplexModel * complexModel = new Gpu::ComplexModel(1);
	complexModel->models[0].mesh = surfaceMesh;
	
	interpreter->PushParam(ScriptParam(complexModel, ScriptTypes::GetHandle(TypeGpuComplexModel)));
}

void GpuScriptCallbacks::CreateScene(ScriptInterpreter * interpreter)
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
	interpreter->PushParam(ScriptParam(scene, ScriptTypes::GetHandle(TypeGpuScene)));
}

void GpuScriptCallbacks::AddToScene(ScriptInterpreter * interpreter)
{
	POP_PTRPARAM(1,scene,TypeGpuScene);
	ScriptParam object = interpreter->PopParam();

	Gpu::Scene * gpuScene = scene.GetPointer<Gpu::Scene>();

	if(object.type == ScriptParam::POINTER)
	{
		if(ScriptTypes::CheckPtrType(object,TypeGpuComplexModel))
		{
			Gpu::ComplexModel * model = object.GetPointer<Gpu::ComplexModel>();
			for(unsigned i = 0; i < model->numModels; ++i)
			{
				gpuScene->Add(&model->models[i]);
			}
		}
		if(ScriptTypes::CheckPtrType(object, TypeGpuTexture))
		{
			// FIXME - if I create a new sprite here, who's going to delete it when the scene is destroyed?
		}
		if(ScriptTypes::CheckPtrType(object,TypeGpuLight))
		{
			Gpu::Light * light = object.GetPointer<Gpu::Light>();
			gpuScene->Add(light);
		}
	}
	
}

void GpuScriptCallbacks::ClearScene(ScriptInterpreter * interpreter)
{
	POP_PTRPARAM(1,scene,TypeGpuScene);

	Gpu::Scene * gpuScene = scene.GetPointer<Gpu::Scene>();

	gpuScene->ClearModels();
	//gpuScene->ClearSprites();
	gpuScene->ClearLights();
}

void GpuScriptCallbacks::DrawScene(ScriptInterpreter * interpreter)
{
	POP_PTRPARAM(1,scene,TypeGpuScene);
	POP_PTRPARAM(2,camera,TypeGpuCamera);

	Gpu::Scene * gpuScene = scene.GetPointer<Gpu::Scene>();
	Gpu::Camera * gpuCamera = camera.GetPointer<Gpu::Camera>();

	gpuScene->SetCamera(gpuCamera);

	interpreter->GetApp()->gpu->Draw(gpuScene);
}

void GpuScriptCallbacks::BeginTimestamp(ScriptInterpreter * interpreter)
{
	POP_PARAM(1, name, STRING);
	POP_NUMPARAM(2, cpu);
	POP_NUMPARAM(3, gpu);

	std::string sname(name.svalue);
	std::wstring wname(sname.begin(), sname.end());

	if(cpu.nvalue > 0.0)
	{
		interpreter->GetApp()->platform->BeginTimestamp(wname.c_str());
	}
	
	if(gpu.nvalue > 0.0)
	{
		interpreter->GetApp()->gpu->BeginTimestamp(wname);
	}
}

void GpuScriptCallbacks::EndTimestamp(ScriptInterpreter * interpreter)
{
	POP_PARAM(1, name, STRING);
	POP_NUMPARAM(2, cpu);
	POP_NUMPARAM(3, gpu);

	std::string sname(name.svalue);
	std::wstring wname(sname.begin(), sname.end());

	if(cpu.nvalue > 0.0)
	{
		interpreter->GetApp()->platform->EndTimestamp(wname.c_str());
	}
	
	if(gpu.nvalue > 0.0)
	{
		interpreter->GetApp()->gpu->EndTimestamp(wname);
	}
}

void GpuScriptCallbacks::GetTimestampData(ScriptInterpreter * interpreter)
{
	POP_PARAM(1, name, STRING);
	POP_NUMPARAM(2, cpu);

	std::string sname(name.svalue);
	std::wstring wname(sname.begin(), sname.end());
	
	float time = 0.0f;

	if(cpu.nvalue > 0.0)
	{
		time = interpreter->GetApp()->platform->GetTimestampTime(wname.c_str());
	}
	else
	{
		Gpu::TimestampData tsData = interpreter->GetApp()->gpu->GetTimestampData(wname);
		time = tsData.data[Gpu::TimestampData::Time];
	}

	interpreter->ClearParams();
	interpreter->PushParam(ScriptParam(ScriptParam::DOUBLE, double(time)));
}

void GpuScriptCallbacks::CreateInstanceBuffer(ScriptInterpreter * interpreter)
{
	POP_PARAM(1, type, STRING);
	POP_NUMPARAM(2, size);

	unsigned bufferSize = unsigned(size.nvalue);
	InstanceType instanceType = InstanceType_Pos;
	void * initialData = 0;

	if(strcmp(type.svalue, "PosCol") == 0)
	{
		instanceType = InstanceType_PosCol;
		initialData = new Instance_PosCol[bufferSize];
	}
	else if(strcmp(type.svalue, "PosSca") == 0)
	{
		instanceType = InstanceType_PosSca;
		initialData = new Instance_PosSca[bufferSize];
	}
	else if(strcmp(type.svalue, "PosRotSca") == 0)
	{
		instanceType = InstanceType_PosRotSca;
		initialData = new Instance_PosRotSca[bufferSize];
	}
	else
	{
		initialData = new Instance_Pos[bufferSize];
	}

	unsigned dataSize = bufferSize * VertApi::GetInstanceSize(instanceType);
	Gpu::InstanceBuffer * buffer = interpreter->GetApp()->gpu->CreateInstanceBuffer(bufferSize, initialData, instanceType);
	ScriptFloatArray * floatArray = new ScriptFloatArray(dataSize / sizeof(float));
	memcpy(floatArray->floats, initialData, dataSize);

	interpreter->ClearParams();
	interpreter->PushParam(ScriptParam(buffer, ScriptTypes::GetHandle(TypeGpuInstanceBuffer)));
	interpreter->PushParam(ScriptParam(floatArray, ScriptTypes::GetHandle(TypeFloatArray)));

	delete[] initialData;
}

void GpuScriptCallbacks::UpdateInstanceBuffer(ScriptInterpreter * interpreter)
{
	POP_PTRPARAM(1, ibuf, TypeGpuInstanceBuffer);
	POP_PTRPARAM(2, floats, TypeFloatArray);
	POP_NUMPARAM(3, size);

	Gpu::InstanceBuffer * buffer = ibuf.GetPointer<Gpu::InstanceBuffer>();
	ScriptFloatArray * floatArray = floats.GetPointer<ScriptFloatArray>();
	unsigned activeInstances = unsigned(size.nvalue);

	interpreter->GetApp()->gpu->UpdateInstanceBuffer(buffer, activeInstances, floatArray->floats);
}

} // namespace Ingenuity
