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
#include "PlatformApi.h"
#include "IsoSurface.h"
#include "PhysicsApi.h"
#include "WavefrontLoader.h"
#include "HeightParser.h"
#include "LeapMotionHelper.h"
#include <sstream>
#include <vector>

namespace Ingenuity {

unsigned ScriptCallbacks::typeHandles[ScriptCallbacks::TypeCount];
std::wstring ScriptCallbacks::projectDirPath;

struct ScriptFloatArray
{
	float * floats;
	unsigned length;
	Gpu::FloatArray gpuFloatArray;

	ScriptFloatArray(unsigned length) :
		floats(new float[length]),
		length(length),
		gpuFloatArray(floats, length) {}
	~ScriptFloatArray() { delete[] floats; }
};

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
			assetType = SvgAsset;
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
		wideFile = projectDirPath + wideFile;
	}

	batch.emplace_back(directoryPtr, wideFile.c_str(), assetType, name);
}

//ScriptParam ScriptCallbacks::VertexBufferToMap(ScriptInterpreter * interpreter, IVertexBuffer * vertexBuffer)
//{
//	if(!interpreter || !vertexBuffer) return ScriptParam();
//
//	ScriptParam map = interpreter->CreateMap();
//
//	VertexType vType = vertexBuffer->GetVertexType();
//
//	for(unsigned i = 0; i < vertexBuffer->GetLength(); i++)
//	{
//		ScriptParam vertex = interpreter->CreateMap();
//
//		switch(vType)
//		{
//		case VertexType_Pos:
//		{
//			VertexBuffer<Vertex_Pos> * vb = static_cast<VertexBuffer<Vertex_Pos>*>(vertexBuffer);
//			interpreter->SetMapNext(vertex, ScriptParam(ScriptParam::DOUBLE, 1.0), ScriptParam(ScriptParam::FLOAT, vb->Get(i).position.x));
//			interpreter->SetMapNext(vertex, ScriptParam(ScriptParam::DOUBLE, 2.0), ScriptParam(ScriptParam::FLOAT, vb->Get(i).position.y));
//			interpreter->SetMapNext(vertex, ScriptParam(ScriptParam::DOUBLE, 3.0), ScriptParam(ScriptParam::FLOAT, vb->Get(i).position.z));
//			break;
//		}
//		case VertexType_PosCol:
//		{
//			VertexBuffer<Vertex_PosCol> * vb = static_cast<VertexBuffer<Vertex_PosCol>*>(vertexBuffer);
//			interpreter->SetMapNext(vertex, ScriptParam(ScriptParam::DOUBLE, 1.0), ScriptParam(ScriptParam::FLOAT, vb->Get(i).position.x));
//			interpreter->SetMapNext(vertex, ScriptParam(ScriptParam::DOUBLE, 2.0), ScriptParam(ScriptParam::FLOAT, vb->Get(i).position.y));
//			interpreter->SetMapNext(vertex, ScriptParam(ScriptParam::DOUBLE, 3.0), ScriptParam(ScriptParam::FLOAT, vb->Get(i).position.z));
//			interpreter->SetMapNext(vertex, ScriptParam(ScriptParam::DOUBLE, 4.0), ScriptParam(ScriptParam::FLOAT, vb->Get(i).color.r));
//			interpreter->SetMapNext(vertex, ScriptParam(ScriptParam::DOUBLE, 5.0), ScriptParam(ScriptParam::FLOAT, vb->Get(i).color.g));
//			interpreter->SetMapNext(vertex, ScriptParam(ScriptParam::DOUBLE, 6.0), ScriptParam(ScriptParam::FLOAT, vb->Get(i).color.b));
//			break;
//		}
//		case VertexType_PosNor:
//		{
//			VertexBuffer<Vertex_PosNor> * vb = static_cast<VertexBuffer<Vertex_PosNor>*>(vertexBuffer);
//			interpreter->SetMapNext(vertex, ScriptParam(ScriptParam::DOUBLE, 1.0), ScriptParam(ScriptParam::FLOAT, vb->Get(i).position.x));
//			interpreter->SetMapNext(vertex, ScriptParam(ScriptParam::DOUBLE, 2.0), ScriptParam(ScriptParam::FLOAT, vb->Get(i).position.y));
//			interpreter->SetMapNext(vertex, ScriptParam(ScriptParam::DOUBLE, 3.0), ScriptParam(ScriptParam::FLOAT, vb->Get(i).position.z));
//			interpreter->SetMapNext(vertex, ScriptParam(ScriptParam::DOUBLE, 4.0), ScriptParam(ScriptParam::FLOAT, vb->Get(i).normal.x));
//			interpreter->SetMapNext(vertex, ScriptParam(ScriptParam::DOUBLE, 5.0), ScriptParam(ScriptParam::FLOAT, vb->Get(i).normal.y));
//			interpreter->SetMapNext(vertex, ScriptParam(ScriptParam::DOUBLE, 6.0), ScriptParam(ScriptParam::FLOAT, vb->Get(i).normal.z));
//			break;
//		}
//		case VertexType_PosTex:
//		{
//			VertexBuffer<Vertex_PosTex> * vb = static_cast<VertexBuffer<Vertex_PosTex>*>(vertexBuffer);
//			interpreter->SetMapNext(vertex, ScriptParam(ScriptParam::DOUBLE, 1.0), ScriptParam(ScriptParam::FLOAT, vb->Get(i).position.x));
//			interpreter->SetMapNext(vertex, ScriptParam(ScriptParam::DOUBLE, 2.0), ScriptParam(ScriptParam::FLOAT, vb->Get(i).position.y));
//			interpreter->SetMapNext(vertex, ScriptParam(ScriptParam::DOUBLE, 3.0), ScriptParam(ScriptParam::FLOAT, vb->Get(i).position.z));
//			interpreter->SetMapNext(vertex, ScriptParam(ScriptParam::DOUBLE, 4.0), ScriptParam(ScriptParam::FLOAT, vb->Get(i).texCoord.x));
//			interpreter->SetMapNext(vertex, ScriptParam(ScriptParam::DOUBLE, 5.0), ScriptParam(ScriptParam::FLOAT, vb->Get(i).texCoord.y));
//			break;
//		}
//		case VertexType_PosNorTex:
//		{
//			VertexBuffer<Vertex_PosNorTex> * vb = static_cast<VertexBuffer<Vertex_PosNorTex>*>(vertexBuffer);
//			interpreter->SetMapNext(vertex, ScriptParam(ScriptParam::DOUBLE, 1.0), ScriptParam(ScriptParam::FLOAT, vb->Get(i).position.x));
//			interpreter->SetMapNext(vertex, ScriptParam(ScriptParam::DOUBLE, 2.0), ScriptParam(ScriptParam::FLOAT, vb->Get(i).position.y));
//			interpreter->SetMapNext(vertex, ScriptParam(ScriptParam::DOUBLE, 3.0), ScriptParam(ScriptParam::FLOAT, vb->Get(i).position.z));
//			interpreter->SetMapNext(vertex, ScriptParam(ScriptParam::DOUBLE, 4.0), ScriptParam(ScriptParam::FLOAT, vb->Get(i).normal.x));
//			interpreter->SetMapNext(vertex, ScriptParam(ScriptParam::DOUBLE, 5.0), ScriptParam(ScriptParam::FLOAT, vb->Get(i).normal.y));
//			interpreter->SetMapNext(vertex, ScriptParam(ScriptParam::DOUBLE, 6.0), ScriptParam(ScriptParam::FLOAT, vb->Get(i).normal.z));
//			interpreter->SetMapNext(vertex, ScriptParam(ScriptParam::DOUBLE, 7.0), ScriptParam(ScriptParam::FLOAT, vb->Get(i).texCoord.x));
//			interpreter->SetMapNext(vertex, ScriptParam(ScriptParam::DOUBLE, 8.0), ScriptParam(ScriptParam::FLOAT, vb->Get(i).texCoord.y));
//			break;
//		}
//		case VertexType_PosNorTanTex:
//		{
//			VertexBuffer<Vertex_PosNorTanTex> * vb = static_cast<VertexBuffer<Vertex_PosNorTanTex>*>(vertexBuffer);
//			interpreter->SetMapNext(vertex, ScriptParam(ScriptParam::DOUBLE, 1.0), ScriptParam(ScriptParam::FLOAT, vb->Get(i).position.x));
//			interpreter->SetMapNext(vertex, ScriptParam(ScriptParam::DOUBLE, 2.0), ScriptParam(ScriptParam::FLOAT, vb->Get(i).position.y));
//			interpreter->SetMapNext(vertex, ScriptParam(ScriptParam::DOUBLE, 3.0), ScriptParam(ScriptParam::FLOAT, vb->Get(i).position.z));
//			interpreter->SetMapNext(vertex, ScriptParam(ScriptParam::DOUBLE, 4.0), ScriptParam(ScriptParam::FLOAT, vb->Get(i).normal.x));
//			interpreter->SetMapNext(vertex, ScriptParam(ScriptParam::DOUBLE, 5.0), ScriptParam(ScriptParam::FLOAT, vb->Get(i).normal.y));
//			interpreter->SetMapNext(vertex, ScriptParam(ScriptParam::DOUBLE, 6.0), ScriptParam(ScriptParam::FLOAT, vb->Get(i).normal.z));
//			interpreter->SetMapNext(vertex, ScriptParam(ScriptParam::DOUBLE, 7.0), ScriptParam(ScriptParam::FLOAT, vb->Get(i).tangent.x));
//			interpreter->SetMapNext(vertex, ScriptParam(ScriptParam::DOUBLE, 8.0), ScriptParam(ScriptParam::FLOAT, vb->Get(i).tangent.y));
//			interpreter->SetMapNext(vertex, ScriptParam(ScriptParam::DOUBLE, 9.0), ScriptParam(ScriptParam::FLOAT, vb->Get(i).tangent.z));
//			interpreter->SetMapNext(vertex, ScriptParam(ScriptParam::DOUBLE, 10.0), ScriptParam(ScriptParam::FLOAT, vb->Get(i).tanChirality));
//			interpreter->SetMapNext(vertex, ScriptParam(ScriptParam::DOUBLE, 11.0), ScriptParam(ScriptParam::FLOAT, vb->Get(i).texCoord.x));
//			interpreter->SetMapNext(vertex, ScriptParam(ScriptParam::DOUBLE, 12.0), ScriptParam(ScriptParam::FLOAT, vb->Get(i).texCoord.y));
//			break;
//		}
//		default:
//			break;
//		}
//
//		interpreter->SetMapNext(map, ScriptParam(ScriptParam::DOUBLE, double(i+1)), vertex);
//	}
//
//	return map;
//}

//ScriptParam ScriptCallbacks::IndexBufferToMap(ScriptInterpreter * interpreter, unsigned * indexBuffer, unsigned numTriangles)
//{
//	if(!interpreter || !indexBuffer) return ScriptParam();
//
//	ScriptParam map = interpreter->CreateMap();
//
//	for(unsigned i = 0; i < numTriangles * 3; i++)
//	{
//		interpreter->SetMapNext(map, ScriptParam(ScriptParam::DOUBLE, double(i+1)), ScriptParam(ScriptParam::DOUBLE, double(indexBuffer[i])));
//	}
//
//	return map;
//}

ScriptParam ScriptCallbacks::VertexBufferToFloats(ScriptInterpreter * interpreter, IVertexBuffer * vertexBuffer)
{
	if(!interpreter || !vertexBuffer) return ScriptParam();

	const unsigned rawBufferSize = vertexBuffer->GetLength() * vertexBuffer->GetElementSize();
	const unsigned floatBufferSize = rawBufferSize / sizeof(float);

	ScriptFloatArray * scriptFloatArray = new ScriptFloatArray(floatBufferSize);

	memcpy(scriptFloatArray->floats, vertexBuffer->GetData(), rawBufferSize);

	return ScriptParam(scriptFloatArray, typeHandles[TypeFloatArray]);
}

ScriptParam ScriptCallbacks::IndexBufferToFloats(ScriptInterpreter * interpreter, unsigned * indexBuffer, unsigned numTriangles)
{
	if(!interpreter || !indexBuffer) return ScriptParam();

	ScriptFloatArray * scriptFloatArray = new ScriptFloatArray(numTriangles * 3);

	for(unsigned i = 0; i < numTriangles * 3; i++)
	{
		scriptFloatArray->floats[i] = float(indexBuffer[i]);
	}
	
	return ScriptParam(scriptFloatArray, typeHandles[TypeFloatArray]);
}

LocalMesh * ScriptCallbacks::FloatsToLocalMesh(ScriptInterpreter * interpreter, ScriptParam type, ScriptParam vertices, ScriptParam indices)
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
	else if(CheckPtrType(vertices, TypeFloatArray) && CheckPtrType(indices, TypeFloatArray))
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

void ScriptCallbacks::ClearConsole(ScriptInterpreter * interpreter)
{
	interpreter->GetLogger().Clear();
}

void ScriptCallbacks::Help(ScriptInterpreter * interpreter)
{
	ScriptParam funcName = interpreter->PopParam();

	if(funcName.type == ScriptParam::STRING)
	{
		bool found = false;
		for(unsigned i = 0; i < interpreter->callbackMeta.size(); i++)
		{
			if(strcmp(funcName.svalue, interpreter->callbackMeta[i].name) == 0)
			{
				interpreter->GetLogger().Log("%s - %s\n",
					interpreter->callbackMeta[i].name,
					interpreter->callbackMeta[i].help);
				found = true;
				break;
			}
		}
		if(!found)
		{
			interpreter->GetLogger().Log("Unrecognized function name: %s", funcName.svalue);
		}
	}
	else
	{
		for(unsigned i = 0; i < interpreter->callbackMeta.size(); i++)
		{
			interpreter->GetLogger().Log("%s - %s\n",
				interpreter->callbackMeta[i].name,
				interpreter->callbackMeta[i].help);
		}
	}
}

void ScriptCallbacks::Require(ScriptInterpreter * interpreter)
{
	struct RequireResponse : public Files::Response
	{
		ScriptInterpreter * interpreter;
		std::string filename;
		std::string modulename;

		RequireResponse(ScriptInterpreter * i, std::wstring filename, const char* modulename) : interpreter(i)
		{
			this->filename.assign(filename.begin(), filename.end());
			this->modulename = modulename ? modulename : "";
		}

		virtual void Respond() override
		{
			closeOnComplete = true; deleteOnComplete = true;

			if(buffer)
			{
				interpreter->LoadScript(buffer, bufferLength, filename.c_str(), modulename.c_str());
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
	ScriptParam modulename = interpreter->PopParam();

	interpreter->incDependencies();

	Files::Api * files = interpreter->GetApp()->files;
	Files::Directory * dir = GetDirectory(files, directory.svalue);
	std::string spath(path.svalue);
	std::wstring wpath(spath.begin(), spath.end());

	if(dir->isProjectDir)
	{
		wpath = projectDirPath + wpath;
	}

	bool hasModuleName = modulename.type == ScriptParam::STRING;
	
	files->OpenAndRead(dir, wpath.c_str(), new RequireResponse(interpreter, wpath, hasModuleName ? modulename.svalue : 0));
}

void ScriptCallbacks::DrawSprite(ScriptInterpreter * interpreter)
{
	POP_PTRPARAM(1,texture,TypeGpuTexture);
	POP_NUMPARAM(2,x);
	POP_NUMPARAM(3,y);
	ScriptParam size = interpreter->PopParam();
	ScriptParam surface = interpreter->PopParam();

	Gpu::Sprite sprite;
	sprite.position.x = (float) x.nvalue;
	sprite.position.y = (float) y.nvalue;
	sprite.texture = texture.GetPointer<Gpu::Texture>();

	if(size.type == ScriptParam::DOUBLE) 
	{
		sprite.size = float(size.nvalue);
	}

	Gpu::DrawSurface * drawSurface = 0;
	if(CheckPtrType(surface, TypeGpuDrawSurface))
	{
		drawSurface = surface.GetPointer<Gpu::DrawSurface>();
	}

	interpreter->GetApp()->gpu->DrawGpuSprite(&sprite, drawSurface);
}

void ScriptCallbacks::CreateCamera(ScriptInterpreter * interpreter)
{
	ScriptParam orthographic = interpreter->PopParam();

	Gpu::Camera * camera = new Gpu::Camera();

	if(orthographic.type == ScriptParam::BOOL)
	{
		camera->isOrthoCamera = orthographic.nvalue > 0.0;
	}

	if(camera) interpreter->PushParam(ScriptParam(camera,typeHandles[TypeGpuCamera]));
}

void ScriptCallbacks::SetCameraPosition(ScriptInterpreter * interpreter)
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

void ScriptCallbacks::SetCameraTarget(ScriptInterpreter * interpreter)
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

void ScriptCallbacks::SetCameraUp(ScriptInterpreter * interpreter)
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

void ScriptCallbacks::SetCameraClipFovOrHeight(ScriptInterpreter * interpreter)
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

void ScriptCallbacks::GetCameraRay(ScriptInterpreter * interpreter)
{
	POP_PTRPARAM(1, camera, TypeGpuCamera);
	POP_NUMPARAM(2, x);
	POP_NUMPARAM(3, y);
	ScriptParam surface = interpreter->PopParam();
	interpreter->ClearParams();

	Gpu::Camera * gpuCamera = camera.GetPointer<Gpu::Camera>();

	unsigned width, height;
	if(CheckPtrType(surface, TypeGpuDrawSurface))
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

void ScriptCallbacks::CreateGrid(ScriptInterpreter * interpreter)
{
	POP_NUMPARAM(1, width);
	POP_NUMPARAM(2, depth);
	POP_NUMPARAM(3, cols);
	POP_NUMPARAM(4, rows);
	ScriptParam texX = interpreter->PopParam();
	ScriptParam texY = interpreter->PopParam();
	ScriptParam texW = interpreter->PopParam();
	ScriptParam texH = interpreter->PopParam();

	float w = float(width.nvalue);
	float d = float(depth.nvalue);
	unsigned c = unsigned(cols.nvalue);
	unsigned r = unsigned(rows.nvalue);

	LocalMesh * localGrid = 0;

	if(texX.IsNumber() && texY.IsNumber() && texW.IsNumber() && texH.IsNumber())
	{
		float tx = float(texX.nvalue);
		float ty = float(texY.nvalue);
		float tw = float(texW.nvalue);
		float th = float(texH.nvalue);

		localGrid = GeoBuilder().BuildGrid(w, d, c, r, &Gpu::Rect(tx, ty, tw, th));
	}
	else
	{
		localGrid = GeoBuilder().BuildGrid(w, d, c, r);
	}

	interpreter->PushParam(VertexBufferToFloats(interpreter, localGrid->vertexBuffer));
	interpreter->PushParam(IndexBufferToFloats(interpreter, localGrid->indexBuffer, localGrid->numTriangles));

	delete localGrid;
}

void ScriptCallbacks::CreateCube(ScriptInterpreter * interpreter)
{
	ScriptParam texCoords = interpreter->PopParam();

	bool generateTexCoords = true;
	if(texCoords.type == ScriptParam::BOOL || texCoords.IsNumber())
	{
		generateTexCoords = texCoords.nvalue > 0.0;
	}

	LocalMesh * localCube = GeoBuilder().BuildCube(generateTexCoords);

	interpreter->PushParam(VertexBufferToFloats(interpreter, localCube->vertexBuffer));
	interpreter->PushParam(IndexBufferToFloats(interpreter, localCube->indexBuffer, localCube->numTriangles));

	delete localCube;
}

void ScriptCallbacks::CreateSphere(ScriptInterpreter * interpreter)
{
	LocalMesh * localSphere = GeoBuilder().BuildSphere(0.5f, 20, 20);

	interpreter->PushParam(VertexBufferToFloats(interpreter, localSphere->vertexBuffer));
	interpreter->PushParam(IndexBufferToFloats(interpreter, localSphere->indexBuffer, localSphere->numTriangles));

	delete localSphere;
}

void ScriptCallbacks::CreateCylinder(ScriptInterpreter * interpreter)
{
	POP_NUMPARAM(1, length);

	LocalMesh * localCylinder = GeoBuilder().BuildCylinder(0.5f, float(length.nvalue), 20, 20);

	interpreter->PushParam(VertexBufferToFloats(interpreter, localCylinder->vertexBuffer));
	interpreter->PushParam(IndexBufferToFloats(interpreter, localCylinder->indexBuffer, localCylinder->numTriangles));

	delete localCylinder;
}

void ScriptCallbacks::CreateCapsule(ScriptInterpreter * interpreter)
{
	POP_NUMPARAM(1,length)

	LocalMesh * localCapsule = GeoBuilder().BuildCapsule(0.5f, float(length.nvalue), 20, 20);

	interpreter->PushParam(VertexBufferToFloats(interpreter, localCapsule->vertexBuffer));
	interpreter->PushParam(IndexBufferToFloats(interpreter, localCapsule->indexBuffer, localCapsule->numTriangles));

	delete localCapsule;
}

void ScriptCallbacks::DrawModel(ScriptInterpreter * interpreter)
{
	POP_PTRPARAM(1,model,TypeGpuComplexModel);
	POP_PTRPARAM(2,camera,TypeGpuCamera);
	ScriptParam lights = interpreter->PopParam();
	ScriptParam surface = interpreter->PopParam();
	ScriptParam instances = interpreter->PopParam();

	Gpu::ComplexModel * gpuModel = model.GetPointer<Gpu::ComplexModel>();
	Gpu::Camera * gpuCamera = camera.GetPointer<Gpu::Camera>();
	Gpu::Light * gpuLights[5];
	unsigned numGpuLights = 0;
	Gpu::DrawSurface * gpuSurface = 0;
	Gpu::InstanceBuffer * gpuInstances = 0;

	if(lights.type == ScriptParam::MAPREF)
	{
		ScriptParam index; // NONE
		ScriptParam result;

		while(interpreter->GetMapNext(lights,index,result) && numGpuLights < 5)
		{
			if(!CheckPtrType(result,TypeGpuLight)) break;
			gpuLights[numGpuLights++] = result.GetPointer<Gpu::Light>();
		}
	}
	if(CheckPtrType(lights, TypeGpuLight))
	{
		gpuLights[numGpuLights++] = lights.GetPointer<Gpu::Light>();
	}

	if(CheckPtrType(surface, TypeGpuDrawSurface))
	{
		gpuSurface = surface.GetPointer<Gpu::DrawSurface>();
	}
	if(CheckPtrType(instances, TypeGpuInstanceBuffer))
	{
		gpuInstances = instances.GetPointer<Gpu::InstanceBuffer>();
	}

	if(gpuModel)
	{
		Gpu::Api * gpu = interpreter->GetApp()->gpu;
		gpuModel->instances = gpuInstances;
		gpuModel->BeDrawn(gpu, gpuCamera, gpuLights, numGpuLights, gpuSurface);
	}
}

void ScriptCallbacks::SetModelPosition(ScriptInterpreter * interpreter)
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

void ScriptCallbacks::SetModelRotation(ScriptInterpreter * interpreter)
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

void ScriptCallbacks::SetModelScale(ScriptInterpreter * interpreter)
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

	if(font) interpreter->PushParam(ScriptParam(font, typeHandles[TypeGpuFont]));
}

void ScriptCallbacks::DrawText(ScriptInterpreter * interpreter)
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
	if(CheckPtrType(surface, TypeGpuDrawSurface))
	{
		drawSurface = surface.GetPointer<Gpu::DrawSurface>();
	}

	Gpu::Api * gpu = interpreter->GetApp()->gpu;
	gpu->DrawGpuText(gpuFont,wideText.c_str(),(float)x.nvalue,(float)y.nvalue,centered,drawSurface);
}

void ScriptCallbacks::SetFontColor(ScriptInterpreter * interpreter)
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

	interpreter->PushParam(ScriptParam(result,typeHandles[TypeGpuLight]));
}

void ScriptCallbacks::SetLightColor(ScriptInterpreter * interpreter)
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

void ScriptCallbacks::SetLightPosition(ScriptInterpreter * interpreter)
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

void ScriptCallbacks::SetLightDirection(ScriptInterpreter * interpreter)
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

void ScriptCallbacks::SetLightAttenuation(ScriptInterpreter * interpreter)
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

void ScriptCallbacks::SetMeshPosition(ScriptInterpreter * interpreter)
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

void ScriptCallbacks::SetMeshRotation(ScriptInterpreter * interpreter)
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

void ScriptCallbacks::SetMeshMatrix(ScriptInterpreter * interpreter)
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

void ScriptCallbacks::SetMeshEffect(ScriptInterpreter * interpreter)
{
	POP_PTRPARAM(1,model,TypeGpuComplexModel);
	POP_NUMPARAM(2,meshnum);
	ScriptParam effect = interpreter->PopParam();

	Gpu::ComplexModel * gpuModel = model.GetPointer<Gpu::ComplexModel>();
	unsigned meshnumber = (unsigned) meshnum.nvalue;
	Gpu::Effect * gpuEffect = 0;

	if(meshnumber < gpuModel->numModels)
	{
		if(CheckPtrType(effect, TypeGpuEffect))
		{
			gpuEffect = effect.GetPointer<Gpu::Effect>();
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
		if(!CheckPtrType(nameOrShader,TypeGpuShader))
		{
			interpreter->ThrowError("CreateEffect: Pointer parameter is not of type GpuShader");
			return;
		}
		shader = nameOrShader.GetPointer<Gpu::Shader>();
	}

	if(shader)
	{
		Gpu::Effect * effect = new Gpu::Effect(shader);

		interpreter->PushParam(ScriptParam(effect,typeHandles[TypeGpuEffect]));
	}
}

void ScriptCallbacks::SetMeshTexture(ScriptInterpreter * interpreter)
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

void ScriptCallbacks::SetMeshNormal(ScriptInterpreter * interpreter)
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

void ScriptCallbacks::SetMeshCubeMap(ScriptInterpreter * interpreter)
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

void ScriptCallbacks::SetMeshColor(ScriptInterpreter * interpreter)
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

void ScriptCallbacks::SetMeshSpecular(ScriptInterpreter * interpreter)
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

void ScriptCallbacks::SetMeshFactors(ScriptInterpreter * interpreter)
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

void ScriptCallbacks::GetMeshBounds(ScriptInterpreter * interpreter)
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

void ScriptCallbacks::GetNumMeshes(ScriptInterpreter * interpreter)
{
	POP_PTRPARAM(1,model,TypeGpuComplexModel);

	Gpu::ComplexModel * gpuModel = model.GetPointer<Gpu::ComplexModel>();
	unsigned numMeshes = gpuModel->numModels;

	interpreter->PushParam(ScriptParam(ScriptParam::INT,numMeshes));
}

void ScriptCallbacks::SetEffectParam(ScriptInterpreter * interpreter)
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
		if(value.pvalue->type == typeHandles[TypeGpuTexture])
		{
			gpuEffect->SetParam(paramIndex, value.GetPointer<Gpu::Texture>());
		}
		else if(value.pvalue->type == typeHandles[TypeGpuCubeMap])
		{
			gpuEffect->SetParam(paramIndex, value.GetPointer<Gpu::CubeMap>());
		}
		else if(value.pvalue->type == typeHandles[TypeGpuVolumeTexture])
		{
			gpuEffect->SetParam(paramIndex, value.GetPointer<Gpu::VolumeTexture>());
		}
		else if(value.pvalue->type == typeHandles[TypeGpuDrawSurface])
		{
			gpuEffect->SetParam(paramIndex, value.GetPointer<Gpu::DrawSurface>());
		}
		else if(value.pvalue->type == typeHandles[TypeFloatArray])
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

void ScriptCallbacks::SetSamplerParam(ScriptInterpreter * interpreter)
{
	POP_PTRPARAM(1, effect, TypeGpuEffect);
	POP_NUMPARAM(2, paramnum);
	POP_PARAM(3, key, STRING);
	ScriptParam value = interpreter->PopParam();

	Gpu::Effect * gpuEffect = effect.GetPointer<Gpu::Effect>();

	if(!gpuEffect) return;

	unsigned paramIndex = (unsigned)paramnum.nvalue;

	interpreter->ThrowError("SetSamplerParam not yet implemented!!!");

}

void ScriptCallbacks::CreateFloatArray(ScriptInterpreter * interpreter)
{
	POP_NUMPARAM(1, length);

	if(length.nvalue < 0.0)
	{
		interpreter->ThrowError("Attempted to create FloatArray of negative length!");
		return;
	}

	ScriptFloatArray * floatArray = new ScriptFloatArray(unsigned(length.nvalue));

	interpreter->PushParam(ScriptParam(floatArray, typeHandles[TypeFloatArray]));
}

void ScriptCallbacks::SetFloatArray(ScriptInterpreter * interpreter)
{
	POP_PTRPARAM(1, floatArray, TypeFloatArray);
	POP_NUMPARAM(2, index);
	POP_NUMPARAM(3, value);

	ScriptFloatArray * scriptFloatArray = floatArray.GetPointer<ScriptFloatArray>();
	unsigned uindex = unsigned(index.nvalue);

	while(value.IsNumber())
	{
		if(index.nvalue < 0.0 || uindex >= scriptFloatArray->length)
		{
			interpreter->ThrowError("Attempted to set a FloatArray index out of range");
			return;
		}

		scriptFloatArray->floats[uindex++] = float(value.nvalue);

		value = interpreter->PopParam();
	}
}

void ScriptCallbacks::GetFloatArray(ScriptInterpreter * interpreter)
{
	POP_PTRPARAM(1, floatArray, TypeFloatArray);
	POP_NUMPARAM(2, index);
	ScriptParam numValues = interpreter->PopParam();

	ScriptFloatArray * scriptFloatArray = floatArray.GetPointer<ScriptFloatArray>();
	unsigned uindex = unsigned(index.nvalue);
	unsigned numVals = 1;
	if(numValues.IsNumber()) numVals = unsigned(numValues.nvalue);

	if(index.nvalue < 0.0 || uindex + numVals >= scriptFloatArray->length)
	{
		interpreter->ThrowError("Attempted to get a FloatArray index out of range");
		return;
	}

	for(unsigned i = uindex; i < uindex + numVals; ++i)
	{
		interpreter->PushParam(ScriptParam(ScriptParam::DOUBLE, double(scriptFloatArray->floats[i])));
	}
}

void ScriptCallbacks::CreateSurface(ScriptInterpreter * interpreter)
{
	ScriptParam width = interpreter->PopParam();
	ScriptParam height = interpreter->PopParam();
	ScriptParam screen = interpreter->PopParam();
	ScriptParam format = interpreter->PopParam();

	Gpu::DrawSurface * surface;
	Gpu::DrawSurface::Format surfaceFormat = Gpu::DrawSurface::Format_4x8int;

	if(format.type == ScriptParam::STRING)
	{
		if(strcmp(format.svalue, "4x16f") == 0)
			surfaceFormat = Gpu::DrawSurface::Format_4x16float;
		if(strcmp(format.svalue, "3x10f") == 0)
			surfaceFormat = Gpu::DrawSurface::Format_3x10float;
		if(strcmp(format.svalue, "1x16f") == 0)
			surfaceFormat = Gpu::DrawSurface::Format_1x16float;
	}

	if(width.type == ScriptParam::DOUBLE && height.type == ScriptParam::DOUBLE)
	{
		if(screen.type == ScriptParam::BOOL && screen.nvalue > 0.0)
		{
			surface = interpreter->GetApp()->gpu->CreateScreenDrawSurface(
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
		surface = interpreter->GetApp()->gpu->CreateScreenDrawSurface();
	}

	interpreter->PushParam(ScriptParam(surface,typeHandles[TypeGpuDrawSurface]));
};

void ScriptCallbacks::ShadeSurface(ScriptInterpreter * interpreter)
{
	POP_PTRPARAM(1,inSurface,TypeGpuDrawSurface);
	POP_PTRPARAM(2,effect,TypeGpuEffect);
	POP_PTRPARAM(3,outSurface,TypeGpuDrawSurface);

	Gpu::DrawSurface * gpuInSurface = inSurface.GetPointer<Gpu::DrawSurface>();
	Gpu::Effect * gpuEffect = effect.GetPointer<Gpu::Effect>();
	Gpu::DrawSurface * gpuOutSurface = outSurface.GetPointer<Gpu::DrawSurface>();

	Gpu::Api * gpu = interpreter->GetApp()->gpu;

	gpu->DrawGpuSurface(gpuInSurface,gpuEffect,gpuOutSurface);
}

void ScriptCallbacks::DrawSurface(ScriptInterpreter * interpreter) 
{
	POP_PTRPARAM(1,surface,TypeGpuDrawSurface);

	Gpu::DrawSurface * gpuSurface = surface.GetPointer<Gpu::DrawSurface>();

	Gpu::Sprite surfaceSprite;
	surfaceSprite.pixelSpace = true;
	surfaceSprite.texture = gpuSurface->GetTexture();
	
	interpreter->GetApp()->gpu->DrawGpuSprite(&surfaceSprite);
}

void ScriptCallbacks::ClearSurface(ScriptInterpreter * interpreter)
{
	POP_PTRPARAM(1,surface,TypeGpuDrawSurface);
	ScriptParam r = interpreter->PopParam();
	ScriptParam g = interpreter->PopParam();
	ScriptParam b = interpreter->PopParam();
	ScriptParam a = interpreter->PopParam();

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

void ScriptCallbacks::GetSurfaceTexture(ScriptInterpreter * interpreter)
{
	POP_PTRPARAM(1,surface,TypeGpuDrawSurface);

	Gpu::DrawSurface * gpuSurface = surface.GetPointer<Gpu::DrawSurface>();

	Gpu::Texture * texture = gpuSurface->GetTexture();

	interpreter->ClearParams();
	interpreter->PushParam(ScriptParam(new NonDeletingPtr(texture,typeHandles[TypeGpuTexture])));
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
				else if(i == 3)
				{
					// Final parameter (asset short name) is optional
					properties[i] = 0;
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

	interpreter->ClearParams();
	interpreter->PushParam(ScriptParam(ScriptParam::INT,ticketNum));
}

void ScriptCallbacks::GetLoadProgress(ScriptInterpreter * interpreter)
{
	POP_NUMPARAM(1,ticket);
	interpreter->ClearParams();

	unsigned ticketNum = (unsigned) ticket.nvalue;

	float progress = interpreter->GetApp()->assets->GetProgress(ticketNum);

	interpreter->PushParam(ScriptParam(ScriptParam::DOUBLE,double(progress)));
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
		Files::Directory * directoryPtr = GetDirectory(interpreter->GetApp()->files, directoryOrName.svalue);

		std::string subPathShort(subPath.svalue);
		std::wstring subPathWide(subPathShort.begin(), subPathShort.end());

		if(directoryPtr && directoryPtr->isProjectDir)
		{
			subPathWide = projectDirPath + subPathWide;
		}

		asset = interpreter->GetApp()->assets->GetAsset<IAsset>(directoryPtr, subPathWide.c_str());
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
				interpreter->PushParam(ScriptParam(new NonDeletingPtr(texAsset, typeHandles[TypeGpuTexture])));
				break;
			}
			case CubeMapAsset:
			{
				Gpu::CubeMap * cubeAsset = dynamic_cast<Gpu::CubeMap*>(asset);
				interpreter->PushParam(ScriptParam(new NonDeletingPtr(cubeAsset, typeHandles[TypeGpuCubeMap])));
				break;
			}
			case VolumeTexAsset:
			{
				Gpu::VolumeTexture * vTexAsset = dynamic_cast<Gpu::VolumeTexture*>(asset);
				interpreter->PushParam(ScriptParam(new NonDeletingPtr(vTexAsset, typeHandles[TypeGpuVolumeTexture])));
				break;
			}
			case ShaderAsset:
			{
				Gpu::Shader * shdrAsset = dynamic_cast<Gpu::Shader*>(asset);
				interpreter->PushParam(ScriptParam(new NonDeletingPtr(shdrAsset, typeHandles[TypeGpuShader])));
				break;
			}
			case ModelAsset:
			{
				Gpu::ComplexModel * mdlAsset = dynamic_cast<Gpu::ComplexModel*>(asset);
				interpreter->PushParam(ScriptParam(mdlAsset, typeHandles[TypeGpuComplexModel]));
				break;
			}
			case RawHeightMapAsset:
			{
				HeightParser * heightParser = dynamic_cast<HeightParser*>(asset);
				interpreter->PushParam(ScriptParam(new NonDeletingPtr(heightParser, typeHandles[TypeHeightParser])));
				break;
			}
			case ImageAsset:
			{
				Image::Buffer * imgAsset = dynamic_cast<Image::Buffer*>(asset);
				interpreter->PushParam(ScriptParam(new NonDeletingPtr(imgAsset, typeHandles[TypeImageBuffer])));
				break;
			}
			case AudioAsset:
			{
				Audio::Item * audioItem = dynamic_cast<Audio::Item*>(asset);
				interpreter->PushParam(ScriptParam(new NonDeletingPtr(audioItem, typeHandles[TypeAudioItem])));
				break;
			}
			case SvgAsset:
			{
				SvgParser * svgParser = dynamic_cast<SvgParser*>(asset);
				interpreter->PushParam(ScriptParam(new NonDeletingPtr(svgParser, typeHandles[TypeSVGParser])));
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

			interpreter->PushParam(ScriptParam(complexModel, typeHandles[TypeGpuComplexModel]));
		}

		delete localMesh;
	}
}

void ScriptCallbacks::SetHeightmapScale(ScriptInterpreter * interpreter)
{
	POP_PTRPARAM(1, heightmap, TypeHeightParser);
	POP_NUMPARAM(2, x);
	POP_NUMPARAM(3, y);
	POP_NUMPARAM(4, z);
	interpreter->ClearParams();

	HeightParser * heightParser = heightmap.GetPointer<HeightParser>();
	if(heightParser)
	{
		heightParser->SetScale(float(x.nvalue), float(y.nvalue), float(z.nvalue));
	}
}

void ScriptCallbacks::GetHeightmapHeight(ScriptInterpreter * interpreter)
{
	POP_PTRPARAM(1,heightmap,TypeHeightParser);
	POP_NUMPARAM(2,x);
	POP_NUMPARAM(3,z);

	HeightParser * heightParser = heightmap.GetPointer<HeightParser>();
	float y = 0.0f;
	
	if(heightParser)
	{
		y = heightParser->GetHeight((float)x.nvalue,(float)z.nvalue);
	}

	interpreter->PushParam(ScriptParam(ScriptParam::DOUBLE,double(y)));
}

void ScriptCallbacks::GetHeightmapModel(ScriptInterpreter * interpreter)
{
	POP_PTRPARAM(1, heightmap, TypeHeightParser);
	interpreter->ClearParams();

	HeightParser * heightParser = heightmap.GetPointer<HeightParser>();

	float y = 0.0f;

	if(heightParser)
	{
		Gpu::Api * gpu = interpreter->GetApp()->gpu;
		Gpu::Mesh * mesh = heightParser->GetMesh(&Gpu::Rect(0.0f, 0.0f, 1.0f, 1.0f))->GpuOnly(gpu);
		Gpu::ComplexModel * model = new Gpu::ComplexModel(1);
		model->models[0].mesh = mesh;
		model->models[0].destructMesh = true;
		
		interpreter->PushParam(ScriptParam(model,typeHandles[TypeGpuComplexModel]));
	}
}

void ScriptCallbacks::GetSVGModel(ScriptInterpreter * interpreter)
{
	POP_PTRPARAM(1, svg, TypeSVGParser);
	POP_NUMPARAM(2, stroke);
	POP_NUMPARAM(3, anim);
	
	SvgParser * svgParser = svg.GetPointer<SvgParser>();
	Gpu::Api * gpu = interpreter->GetApp()->gpu;
	Gpu::ComplexModel * model = 0;
	
	if(stroke.nvalue > 0.0)
	{
		model = svgParser->GetAnimatedStroke(gpu, float(anim.nvalue));
	}
	else
	{
		model = svgParser->GetModel(gpu);
	}

	interpreter->ClearParams();
	if(model)
	{
		interpreter->PushParam(ScriptParam(model, typeHandles[TypeGpuComplexModel]));
	}
}

void ScriptCallbacks::GetWavefrontMesh(ScriptInterpreter * interpreter)
{
	POP_PARAM(1, name, STRING);
	POP_NUMPARAM(2, index);
	interpreter->ClearParams();

	AssetMgr * assets = interpreter->GetApp()->assets;

	AssetLoader * loader = assets->GetLoader(name.svalue);

	if(loader && loader->type == WavefrontModelAsset)
	{
		WavefrontLoader * wLoader = static_cast<WavefrontLoader*>(loader);

		unsigned i = unsigned(index.nvalue);
		
		if(i < wLoader->GetNumMeshes())
		{
			LocalMesh * mesh = wLoader->GetMesh(i);
			if(mesh)
			{
				const char * typeName = VertApi::GetVertexName(mesh->vertexBuffer->GetVertexType());
				ScriptParam vertices = VertexBufferToFloats(interpreter, mesh->vertexBuffer);
				ScriptParam indices = IndexBufferToFloats(interpreter, mesh->indexBuffer, mesh->numTriangles);

				interpreter->PushParam(ScriptParam(ScriptParam::STRING,typeName));
				interpreter->PushParam(vertices);
				interpreter->PushParam(indices);
			}
		}
	}
}

void ScriptCallbacks::SetClearColor(ScriptInterpreter * interpreter)
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

void ScriptCallbacks::SetBlendMode(ScriptInterpreter * interpreter)
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

void ScriptCallbacks::SetAnisotropy(ScriptInterpreter * interpreter)
{
	POP_NUMPARAM(1, level);
	
	interpreter->GetApp()->gpu->SetAnisotropy(unsigned(level.nvalue));
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

	interpreter->PushParam(ScriptParam(ScriptParam::DOUBLE, double(mouse.x)));
	interpreter->PushParam(ScriptParam(ScriptParam::DOUBLE, double(mouse.y)));
}

void ScriptCallbacks::GetMouseDelta(ScriptInterpreter * interpreter)
{
	InputState * input = interpreter->GetApp()->input;
	MouseState & mouse = input->GetMouseState();

	interpreter->PushParam(ScriptParam(ScriptParam::DOUBLE, double(mouse.dX)));
	interpreter->PushParam(ScriptParam(ScriptParam::DOUBLE, double(mouse.dY)));
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

	if(CheckPtrType(updateRect, TypeGpuComplexModel))
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

		interpreter->PushParam(ScriptParam(gpuModel,typeHandles[TypeGpuComplexModel]));
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

	if(CheckPtrType(updateRect, TypeGpuComplexModel))
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

		interpreter->PushParam(ScriptParam(gpuModel,typeHandles[TypeGpuComplexModel]));
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

		if(CheckPtrType(updateEllipse,TypeGpuComplexModel))
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

			interpreter->PushParam(ScriptParam(gpuModel,typeHandles[TypeGpuComplexModel]));
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

	if(CheckPtrType(updateEllipse, TypeGpuComplexModel))
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

		interpreter->PushParam(ScriptParam(gpuModel,typeHandles[TypeGpuComplexModel]));
	}

	delete localMesh;
}

void ScriptCallbacks::CreateIsoSurface(ScriptInterpreter * interpreter)
{
	POP_NUMPARAM(1, gridSize);
	ScriptParam threshold = interpreter->PopParam();

	if(gridSize.nvalue < 2.0f)
		interpreter->ThrowError("Cannot create IsoSurface, grid size is too small!");

	IsoSurface * isoSurface = new IsoSurface(unsigned(gridSize.nvalue), interpreter->GetApp()->gpu);
	if(threshold.IsNumber()) isoSurface->SetThreshold(float(threshold.nvalue));

	interpreter->ClearParams();
	interpreter->PushParam(ScriptParam(isoSurface, typeHandles[TypeIsoSurface]));
}

void ScriptCallbacks::AddIsoSurfaceBall(ScriptInterpreter * interpreter)
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
void ScriptCallbacks::AddIsoSurfacePlane(ScriptInterpreter * interpreter)
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

void ScriptCallbacks::ClearIsoSurface(ScriptInterpreter * interpreter)
{
	POP_PTRPARAM(1, iso, TypeIsoSurface);
	IsoSurface * isoSurface = iso.GetPointer<IsoSurface>();

	isoSurface->Clear();
}

void ScriptCallbacks::GetIsoSurfaceModel(ScriptInterpreter * interpreter)
{
	POP_PTRPARAM(1, iso, TypeIsoSurface);
	IsoSurface * isoSurface = iso.GetPointer<IsoSurface>();

	isoSurface->UpdateObjects();
	isoSurface->UpdateMesh(interpreter->GetApp()->gpu);

	Gpu::Mesh * surfaceMesh = isoSurface->GetMesh();
	Gpu::ComplexModel * complexModel = new Gpu::ComplexModel(1);
	complexModel->models[0].mesh = surfaceMesh;
	
	interpreter->PushParam(ScriptParam(complexModel, typeHandles[TypeGpuComplexModel]));
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
	interpreter->PushParam(ScriptParam(scene,typeHandles[TypeGpuScene]));
}

void ScriptCallbacks::AddToScene(ScriptInterpreter * interpreter)
{
	POP_PTRPARAM(1,scene,TypeGpuScene);
	ScriptParam object = interpreter->PopParam();

	Gpu::Scene * gpuScene = scene.GetPointer<Gpu::Scene>();

	if(object.type == ScriptParam::POINTER)
	{
		if(CheckPtrType(object,TypeGpuComplexModel))
		{
			Gpu::ComplexModel * model = object.GetPointer<Gpu::ComplexModel>();
			for(unsigned i = 0; i < model->numModels; ++i)
			{
				gpuScene->Add(&model->models[i]);
			}
		}
		if(CheckPtrType(object, TypeGpuTexture))
		{
			// FIXME - if I create a new sprite here, who's going to delete it when the scene is destroyed?
		}
		if(CheckPtrType(object,TypeGpuLight))
		{
			Gpu::Light * light = object.GetPointer<Gpu::Light>();
			gpuScene->Add(light);
		}
	}
	
}

void ScriptCallbacks::ClearScene(ScriptInterpreter * interpreter)
{
	POP_PTRPARAM(1,scene,TypeGpuScene);

	Gpu::Scene * gpuScene = scene.GetPointer<Gpu::Scene>();

	gpuScene->ClearModels();
	gpuScene->ClearSprites();
	gpuScene->ClearLights();
}

void ScriptCallbacks::DrawScene(ScriptInterpreter * interpreter)
{
	POP_PTRPARAM(1,scene,TypeGpuScene);
	POP_PTRPARAM(2,camera,TypeGpuCamera);

	Gpu::Scene * gpuScene = scene.GetPointer<Gpu::Scene>();
	Gpu::Camera * gpuCamera = camera.GetPointer<Gpu::Camera>();

	gpuScene->SetCamera(gpuCamera);

	interpreter->GetApp()->gpu->Draw(gpuScene);
}

void ScriptCallbacks::GetImageSize(ScriptInterpreter * interpreter)
{
	POP_PTRPARAM(1,image,TypeImageBuffer);

	Image::Buffer * imageBuffer = image.GetPointer<Image::Buffer>();
	Image::Api * imaging = interpreter->GetApp()->imaging;

	unsigned w, h;
	imaging->GetImageSize(imageBuffer,w,h);

	interpreter->ClearParams();
	interpreter->PushParam(ScriptParam(ScriptParam::INT, double(w)));
	interpreter->PushParam(ScriptParam(ScriptParam::INT, double(h)));
}

void ScriptCallbacks::GetImagePixel(ScriptInterpreter * interpreter)
{
	POP_PTRPARAM(1,image,TypeImageBuffer);
	POP_NUMPARAM(2,u);
	POP_NUMPARAM(3,v);

	Image::Buffer * imageBuffer = image.GetPointer<Image::Buffer>();
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
		wpath = projectDirPath + wpath;
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
	POP_PTRPARAM(1, sound, TypeAudioItem);
	ScriptParam seek = interpreter->PopParam();
	ScriptParam loop = interpreter->PopParam();
	
	Audio::Item * audioItem = sound.GetPointer<Audio::Item>();
	bool loopBool = false;
	float seekTime = 0.0f;

	if(seek.IsNumber())
	{
		seekTime = float(seek.nvalue);
	}
	if(loop.type == ScriptParam::BOOL || loop.IsNumber())
	{
		loopBool = loop.nvalue > 0.0;
	}

	interpreter->GetApp()->audio->Play(audioItem, seekTime, loopBool);
}

void ScriptCallbacks::PauseSound(ScriptInterpreter * interpreter)
{
	POP_PTRPARAM(1, sound, TypeAudioItem);

	// Pausing sound globally should not be possible from the script

	Audio::Item * audioItem = sound.GetPointer<Audio::Item>();
	interpreter->GetApp()->audio->Pause(audioItem);
}

void ScriptCallbacks::GetAmplitude(ScriptInterpreter * interpreter)
{
	ScriptParam sound = interpreter->PopParam();
	Audio::Item * item = 0;

	if(CheckPtrType(sound,TypeAudioItem))
	{
		item = sound.GetPointer<Audio::Item>();
	}

	float amplitude = interpreter->GetApp()->audio->GetAmplitude(item);

	interpreter->ClearParams();
	interpreter->PushParam(ScriptParam(ScriptParam::DOUBLE, double(amplitude)));
}

void ScriptCallbacks::GetSoundDuration(ScriptInterpreter * interpreter)
{
	POP_PTRPARAM(1, sound, TypeAudioItem);

	Audio::Item * item = sound.GetPointer<Audio::Item>();

	float duration = interpreter->GetApp()->audio->GetDuration(item);

	interpreter->ClearParams();
	interpreter->PushParam(ScriptParam(ScriptParam::DOUBLE, double(duration)));
}

void ScriptCallbacks::GetSoundProgress(ScriptInterpreter * interpreter)
{
	POP_PTRPARAM(1, sound, TypeAudioItem);

	Audio::Item * item = sound.GetPointer<Audio::Item>();

	float progress = interpreter->GetApp()->audio->GetProgress(item);

	interpreter->ClearParams();
	interpreter->PushParam(ScriptParam(ScriptParam::DOUBLE, double(progress)));
}

void ScriptCallbacks::BeginTimestamp(ScriptInterpreter * interpreter)
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

void ScriptCallbacks::EndTimestamp(ScriptInterpreter * interpreter)
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

void ScriptCallbacks::GetTimestampData(ScriptInterpreter * interpreter)
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

void ScriptCallbacks::CreateInstanceBuffer(ScriptInterpreter * interpreter)
{
	POP_PARAM(1, type, STRING);
	POP_NUMPARAM(2, size);

	unsigned bufferSize = unsigned(size.nvalue);
	InstanceType instanceType = InstanceType_Pos;

	if(strcmp(type.svalue, "PosCol") == 0)
	{
		instanceType = InstanceType_PosCol;
	}
	if(strcmp(type.svalue, "PosSca") == 0)
	{
		instanceType = InstanceType_PosSca;
	}
	
	Gpu::InstanceBuffer * buffer = interpreter->GetApp()->gpu->CreateInstanceBuffer(bufferSize, 0, instanceType);

	interpreter->ClearParams();
	interpreter->PushParam(ScriptParam(buffer, typeHandles[TypeGpuInstanceBuffer]));
}

void ScriptCallbacks::UpdateInstanceBuffer(ScriptInterpreter * interpreter)
{
	POP_PTRPARAM(1, ibuf, TypeGpuInstanceBuffer);
	POP_PTRPARAM(2, floats, TypeFloatArray);
	POP_NUMPARAM(3, size);

	Gpu::InstanceBuffer * buffer = ibuf.GetPointer<Gpu::InstanceBuffer>();
	ScriptFloatArray * floatArray = floats.GetPointer<ScriptFloatArray>();
	unsigned activeInstances = unsigned(size.nvalue);

	interpreter->GetApp()->gpu->UpdateInstanceBuffer(buffer, activeInstances, floatArray->floats);
}

void ScriptCallbacks::CreatePhysicsWorld(ScriptInterpreter * interpreter)
{
	PhysicsWorld * physicsWorld = interpreter->GetApp()->physics->CreateWorld(0.0f);

	interpreter->ClearParams();
	interpreter->PushParam(ScriptParam(physicsWorld, typeHandles[TypePhysicsWorld]));
}

void ScriptCallbacks::UpdatePhysicsWorld(ScriptInterpreter * interpreter)
{
	POP_PTRPARAM(1, world, TypePhysicsWorld);
	POP_NUMPARAM(2, delta);
	interpreter->ClearParams();

	PhysicsWorld * physicsWorld = world.GetPointer<PhysicsWorld>();

	interpreter->GetApp()->physics->UpdateWorld(physicsWorld, float(delta.nvalue));
}

void ScriptCallbacks::CreatePhysicsMaterial(ScriptInterpreter * interpreter)
{
	POP_NUMPARAM(1, elasticity); // default 0.63
	POP_NUMPARAM(2, sfriction);  // default 0.95
	POP_NUMPARAM(3, kfriction);  // default 0.7
	POP_NUMPARAM(4, softness);   // default 0.32
	interpreter->ClearParams();

	PhysicsMaterial::Properties properties(
		float(elasticity.nvalue),
		float(sfriction.nvalue),
		float(kfriction.nvalue),
		float(softness.nvalue));

	PhysicsMaterial * material = interpreter->GetApp()->physics->CreateMaterial(properties);

	interpreter->PushParam(ScriptParam(material, typeHandles[TypePhysicsMaterial]));
}

void ScriptCallbacks::CreatePhysicsAnchor(ScriptInterpreter * interpreter)
{
	interpreter->ClearParams();
	PhysicsObject * physicsObject = interpreter->GetApp()->physics->CreateAnchor();
	interpreter->PushParam(ScriptParam(physicsObject, typeHandles[TypePhysicsObject]));
}

void ScriptCallbacks::CreatePhysicsCuboid(ScriptInterpreter * interpreter)
{
	POP_NUMPARAM(1, w);
	POP_NUMPARAM(2, h);
	POP_NUMPARAM(3, d);
	POP_NUMPARAM(4, kinematic);
	interpreter->ClearParams();

	glm::vec3 size(float(w.nvalue), float(h.nvalue), float(d.nvalue));

	PhysicsObject * physicsObject = interpreter->GetApp()->physics->CreateCuboid(size, kinematic.nvalue > 0.0);

	interpreter->PushParam(ScriptParam(physicsObject, typeHandles[TypePhysicsObject]));
}

void ScriptCallbacks::CreatePhysicsSphere(ScriptInterpreter * interpreter)
{
	POP_NUMPARAM(1, r);
	POP_NUMPARAM(2, kinematic);
	interpreter->ClearParams();

	PhysicsObject * physicsObject = interpreter->GetApp()->physics->CreateSphere(float(r.nvalue), kinematic.nvalue > 0.0);

	interpreter->PushParam(ScriptParam(physicsObject, typeHandles[TypePhysicsObject]));
}

void ScriptCallbacks::CreatePhysicsCapsule(ScriptInterpreter * interpreter)
{
	POP_NUMPARAM(1, r);
	POP_NUMPARAM(2, l);
	POP_NUMPARAM(3, kinematic);
	
	PhysicsObject * physicsObject = interpreter->GetApp()->physics->CreateCapsule(float(r.nvalue), float(l.nvalue), kinematic.nvalue > 0.0);

	interpreter->PushParam(ScriptParam(physicsObject, typeHandles[TypePhysicsObject]));
}

void ScriptCallbacks::CreatePhysicsMesh(ScriptInterpreter * interpreter)
{
	POP_PARAM(1, type, STRING);
	POP_PTRPARAM(2, vtx, TypeFloatArray);
	POP_PTRPARAM(3, idx, TypeFloatArray);
	interpreter->ClearParams();

	LocalMesh * localMesh = FloatsToLocalMesh(interpreter, type, vtx, idx);

	PhysicsObject * physicsObject = interpreter->GetApp()->physics->CreateMesh(localMesh, false, true);

	interpreter->PushParam(ScriptParam(physicsObject, typeHandles[TypePhysicsObject]));
}

void ScriptCallbacks::CreatePhysicsHeightmap(ScriptInterpreter * interpreter)
{
	POP_PTRPARAM(1, heightmap, TypeHeightParser);
	interpreter->ClearParams();

	HeightParser * heightParser = heightmap.GetPointer<HeightParser>();
	PhysicsObject * physicsObject = interpreter->GetApp()->physics->CreateHeightmap(heightParser);

	interpreter->PushParam(ScriptParam(physicsObject, typeHandles[TypePhysicsObject]));
}

void ScriptCallbacks::CreatePhysicsSpring(ScriptInterpreter * interpreter)
{
	POP_PTRPARAM(1, obj1, TypePhysicsObject);
	POP_PTRPARAM(2, obj2, TypePhysicsObject);
	POP_SPTRPARAM(3, attach1, TypeVector4);
	POP_SPTRPARAM(4, attach2, TypeVector4);

	PhysicsObject * physObj1 = obj1.GetPointer<PhysicsObject>();
	PhysicsObject * physObj2 = obj2.GetPointer<PhysicsObject>();
	glm::vec4 attachVec1 = *attach1.GetPointer<glm::vec4>();
	glm::vec4 attachVec2 = *attach2.GetPointer<glm::vec4>();

	PhysicsApi * physics = interpreter->GetApp()->physics;

	PhysicsSpring * spring = physics->CreateSpring(physObj1, physObj2,
		glm::vec3(attachVec1), glm::vec3(attachVec2));

	interpreter->PushParam(ScriptParam(spring, typeHandles[TypePhysicsSpring]));
}

void ScriptCallbacks::AddToPhysicsWorld(ScriptInterpreter * interpreter)
{
	POP_PTRPARAM(1, world, TypePhysicsWorld);
	POP_PTRPARAM(2, object, TypePhysicsObject);
	ScriptParam isStatic = interpreter->PopParam();

	interpreter->ClearParams();

	PhysicsWorld * physicsWorld = world.GetPointer<PhysicsWorld>();
	PhysicsObject * physicsObject = object.GetPointer<PhysicsObject>();

	bool physicsStatic = false;
	if(isStatic.IsNumber() || isStatic.type == ScriptParam::BOOL) physicsStatic = isStatic.nvalue > 0.0;

	interpreter->GetApp()->physics->AddToWorld(physicsWorld, physicsObject, physicsStatic);
}

void ScriptCallbacks::RemoveFromPhysicsWorld(ScriptInterpreter * interpreter)
{
	POP_PTRPARAM(1, world, TypePhysicsWorld);
	POP_PTRPARAM(2, object, TypePhysicsObject);

	interpreter->ClearParams();

	PhysicsWorld * physicsWorld = world.GetPointer<PhysicsWorld>();
	PhysicsObject * physicsObject = object.GetPointer<PhysicsObject>();

	interpreter->GetApp()->physics->RemoveFromWorld(physicsWorld, physicsObject);
}

void ScriptCallbacks::SetPhysicsPosition(ScriptInterpreter * interpreter)
{
	POP_PTRPARAM(1, object, TypePhysicsObject);
	POP_NUMPARAM(2, x);
	POP_NUMPARAM(3, y);
	POP_NUMPARAM(4, z);
	ScriptParam local = interpreter->PopParam();
	interpreter->ClearParams();

	PhysicsObject * physicsObject = object.GetPointer<PhysicsObject>();
	glm::vec3 position(float(x.nvalue), float(y.nvalue), float(z.nvalue));
	
	if((local.IsNumber() || local.type == ScriptParam::BOOL) && local.nvalue > 0.0)
	{
		interpreter->GetApp()->physics->SetLocalPosition(physicsObject, position);
	}
	else
	{
		interpreter->GetApp()->physics->SetPosition(physicsObject, position);
	}
}

void ScriptCallbacks::SetPhysicsRotation(ScriptInterpreter * interpreter)
{
	POP_PTRPARAM(1, object, TypePhysicsObject);
	POP_NUMPARAM(2, x);
	POP_NUMPARAM(3, y);
	POP_NUMPARAM(4, z);
	ScriptParam local = interpreter->PopParam();
	interpreter->ClearParams();

	PhysicsObject * physicsObject = object.GetPointer<PhysicsObject>();
	glm::vec3 rotation(float(x.nvalue), float(y.nvalue), float(z.nvalue));

	if((local.IsNumber() || local.type == ScriptParam::BOOL) && local.nvalue > 0.0)
	{
		interpreter->GetApp()->physics->SetLocalRotation(physicsObject, rotation);
	}
	else
	{
		interpreter->GetApp()->physics->SetRotation(physicsObject, rotation);
	}
}

void ScriptCallbacks::SetPhysicsScale(ScriptInterpreter * interpreter)
{
	POP_PTRPARAM(1, object, TypePhysicsObject);
	POP_NUMPARAM(2, x);
	ScriptParam y = interpreter->PopParam();
	ScriptParam z = interpreter->PopParam();
	interpreter->ClearParams();

	PhysicsObject * physicsObject = object.GetPointer<PhysicsObject>();
	glm::vec3 scale(float(x.nvalue));

	if(y.IsNumber() && z.IsNumber())
	{
		scale = glm::vec3(float(x.nvalue), float(y.nvalue), float(z.nvalue));
	}

	interpreter->GetApp()->physics->SetScale(physicsObject, scale);
}

void ScriptCallbacks::SetPhysicsMass(ScriptInterpreter * interpreter)
{
	POP_PTRPARAM(1, object, TypePhysicsObject);
	POP_NUMPARAM(2, mass);
	interpreter->ClearParams();

	PhysicsObject * physicsObject = object.GetPointer<PhysicsObject>();

	interpreter->GetApp()->physics->SetMass(physicsObject, float(mass.nvalue));
}

void ScriptCallbacks::SetPhysicsMaterial(ScriptInterpreter * interpreter)
{
	POP_PTRPARAM(1, object, TypePhysicsObject);
	POP_PTRPARAM(2, material, TypePhysicsMaterial);
	interpreter->ClearParams();

	PhysicsObject * physicsObject = object.GetPointer<PhysicsObject>();
	PhysicsMaterial * physicsMaterial = material.GetPointer<PhysicsMaterial>();

	interpreter->GetApp()->physics->SetMaterial(physicsObject, physicsMaterial);
}

void ScriptCallbacks::GetPhysicsPosition(ScriptInterpreter * interpreter)
{
	POP_PTRPARAM(1, object, TypePhysicsObject);

	interpreter->ClearParams();

	PhysicsObject * physicsObject = object.GetPointer<PhysicsObject>();
	glm::vec3 position = interpreter->GetApp()->physics->GetPosition(physicsObject);

	interpreter->PushParam(ScriptParam(ScriptParam::DOUBLE, double(position.x)));
	interpreter->PushParam(ScriptParam(ScriptParam::DOUBLE, double(position.y)));
	interpreter->PushParam(ScriptParam(ScriptParam::DOUBLE, double(position.z)));
}

//void ScriptCallbacks::GetPhysicsRotation(ScriptInterpreter * interpreter)
//{
//	POP_PTRPARAM(1, object, PhysicsObject);
//
//	interpreter->ClearParams();
//
//	PhysicsObject * physicsObject = static_cast<PhysicsObject*>(object.pvalue->ptr);
//	glm::vec3 rotation = interpreter->GetApp()->physics->GetRotation(physicsObject);
//
//	interpreter->PushParam(ScriptParam(ScriptParam::DOUBLE, double(rotation.x)));
//	interpreter->PushParam(ScriptParam(ScriptParam::DOUBLE, double(rotation.y)));
//	interpreter->PushParam(ScriptParam(ScriptParam::DOUBLE, double(rotation.z)));
//}

void ScriptCallbacks::GetPhysicsMatrix(ScriptInterpreter * interpreter)
{
	POP_PTRPARAM(1, object, TypePhysicsObject);
	interpreter->ClearParams();

	PhysicsObject * physicsObject = object.GetPointer<PhysicsObject>();
	PhysicsApi * physics = interpreter->GetApp()->physics;
	glm::mat4 matrix = physics->GetMatrix(physicsObject);
	const unsigned matrixTypeHandle = interpreter->GetSpecialPtrType(ScriptInterpreter::TypeMatrix4);
	interpreter->PushParam(new BufferCopyPtr(&matrix, sizeof(glm::mat4), matrixTypeHandle));
}

void ScriptCallbacks::SetPhysicsMatrix(ScriptInterpreter * interpreter)
{
	POP_PTRPARAM(1, object, TypePhysicsObject);
	POP_SPTRPARAM(2, matrix, TypeMatrix4);

	PhysicsObject * physicsObject = object.GetPointer<PhysicsObject>();
	glm::mat4 * objectMatrix = matrix.GetPointer<glm::mat4>();
	interpreter->GetApp()->physics->SetTargetMatrix(physicsObject, *objectMatrix);
}

void ScriptCallbacks::SetPhysicsSpringProperty(ScriptInterpreter * interpreter)
{
	POP_PTRPARAM(1, spring, TypePhysicsSpring);
	POP_PARAM(2, name, STRING);
	POP_NUMPARAM(3, value);

	int prop = -1;

	if(strcmp(name.svalue, "stiffness") == 0)
	{
		prop = PhysicsSpring::Stiffness;
	}
	if(strcmp(name.svalue, "damping") == 0)
	{
		prop = PhysicsSpring::Damping;
	}
	if(strcmp(name.svalue, "length") == 0)
	{
		prop = PhysicsSpring::Length;
	}
	if(strcmp(name.svalue, "extends") == 0)
	{
		prop = PhysicsSpring::Extends;
	}
	if(strcmp(name.svalue, "compresses") == 0)
	{
		prop = PhysicsSpring::Compresses;
	}

	if(prop > -1)
	{
		interpreter->GetApp()->physics->SetSpringProperty(
			spring.GetPointer<PhysicsSpring>(),
			(PhysicsSpring::Property) prop,
			float(value.nvalue));
	}
}

void ScriptCallbacks::CreatePhysicsRagdoll(ScriptInterpreter * interpreter)
{
	POP_PTRPARAM(1, world, TypePhysicsWorld);
	interpreter->ClearParams();

	PhysicsWorld * physicsWorld = world.GetPointer<PhysicsWorld>();
	PhysicsRagdoll * physicsRagdoll = interpreter->GetApp()->physics->CreateRagdoll(physicsWorld);

	interpreter->PushParam(ScriptParam(physicsRagdoll, typeHandles[TypePhysicsRagdoll]));
}

void ScriptCallbacks::AddPhysicsRagdollBone(ScriptInterpreter * interpreter)
{
	POP_PTRPARAM(1, ragdoll, TypePhysicsRagdoll);
	POP_PTRPARAM(2, object, TypePhysicsObject);
	POP_NUMPARAM(3, parent);
	POP_NUMPARAM(4, cone);
	POP_NUMPARAM(5, min);
	POP_NUMPARAM(6, max);
	POP_NUMPARAM(7, childRotX);
	POP_NUMPARAM(8, childRotY);
	POP_NUMPARAM(9, childRotZ);
	POP_NUMPARAM(10, parenRotX);
	POP_NUMPARAM(11, parenRotY);
	POP_NUMPARAM(12, parenRotZ);
	interpreter->ClearParams();

	PhysicsRagdoll * physicsRagdoll = ragdoll.GetPointer<PhysicsRagdoll>();
	PhysicsObject * physicsObject = object.GetPointer<PhysicsObject>();
	
	glm::vec3 joint(float(cone.nvalue), float(min.nvalue), float(max.nvalue));
	glm::vec3 childRot(float(childRotX.nvalue), float(childRotY.nvalue), float(childRotZ.nvalue));
	glm::vec3 parentRot(float(parenRotX.nvalue), float(parenRotY.nvalue), float(parenRotZ.nvalue));
	interpreter->GetApp()->physics->AddRagdollBone(physicsRagdoll, physicsObject, int(parent.nvalue), joint, childRot, parentRot);
}

void ScriptCallbacks::GetPhysicsRagdollBone(ScriptInterpreter * interpreter)
{
	POP_PTRPARAM(1, ragdoll, TypePhysicsRagdoll);
	POP_NUMPARAM(2, index);
	interpreter->ClearParams();

	PhysicsRagdoll * physicsRagdoll = ragdoll.GetPointer<PhysicsRagdoll>();
	PhysicsObject * physicsObject = interpreter->GetApp()->physics->GetRagdollObject(physicsRagdoll, unsigned(index.nvalue));

	if(physicsObject)
	{
		interpreter->PushParam(ScriptParam(new NonDeletingPtr(physicsObject, typeHandles[TypePhysicsObject])));
	}
}

void ScriptCallbacks::FinalizePhysicsRagdoll(ScriptInterpreter * interpreter)
{
	POP_PTRPARAM(1, ragdoll, TypePhysicsRagdoll);

	PhysicsRagdoll * physicsRagdoll = ragdoll.GetPointer<PhysicsRagdoll>();

	interpreter->GetApp()->physics->FinalizeRagdoll(physicsRagdoll);
}

void ScriptCallbacks::PickPhysicsObject(ScriptInterpreter * interpreter)
{
	POP_PTRPARAM(1, world, TypePhysicsWorld);
	POP_SPTRPARAM(2, origin, TypeVector4);
	POP_SPTRPARAM(3, ray, TypeVector4);
	interpreter->ClearParams();

	PhysicsApi * physics = interpreter->GetApp()->physics;
	PhysicsWorld * physicsWorld = world.GetPointer<PhysicsWorld>();

	glm::vec4 originVec4 = *origin.GetPointer<glm::vec4>();
	glm::vec4 rayVec4 = *ray.GetPointer<glm::vec4>();

	float t;
	glm::vec3 position;
	glm::vec3 normal;
	PhysicsObject * physicsObject = physics->PickObject(physicsWorld, 
		glm::vec3(originVec4), glm::vec3(rayVec4), t, position, normal);

	glm::vec4 posVec4(position, 1.0f);
	glm::vec4 norVec4(normal, 0.0f);

	if(physicsObject)
	{
		interpreter->PushParam(ScriptParam(new NonDeletingPtr(physicsObject, typeHandles[TypePhysicsObject])));
		interpreter->PushParam(ScriptParam(new BufferCopyPtr(&posVec4, sizeof(glm::vec4), 
			interpreter->GetSpecialPtrType(ScriptInterpreter::TypeVector4))));
		interpreter->PushParam(ScriptParam(new BufferCopyPtr(&norVec4, sizeof(glm::vec4), 
			interpreter->GetSpecialPtrType(ScriptInterpreter::TypeVector4))));
		interpreter->PushParam(ScriptParam(ScriptParam::DOUBLE, double(t)));
	}
}

void ScriptCallbacks::GetPhysicsDebugModel(ScriptInterpreter * interpreter)
{
	POP_PTRPARAM(1, object, TypePhysicsObject);

	PhysicsObject * physicsObject = object.GetPointer<PhysicsObject>();

	LocalMesh * localMesh = interpreter->GetApp()->physics->GetDebugMesh(physicsObject);
	
	Gpu::ComplexModel * complexModel = new Gpu::ComplexModel(1);
	complexModel->models[0].mesh = localMesh->GpuOnly(interpreter->GetApp()->gpu);
	complexModel->models[0].destructMesh = true;

	interpreter->PushParam(ScriptParam(complexModel, typeHandles[TypeGpuComplexModel]));
}

void ScriptCallbacks::CreateLeapHelper(ScriptInterpreter * interpreter)
{
	LeapMotionHelper * leapHelper = new LeapMotionHelper();

	interpreter->PushParam(ScriptParam(leapHelper, typeHandles[TypeLeapHelper]));
}

void ScriptCallbacks::GetLeapFrameTime(ScriptInterpreter * interpreter)
{
	POP_PTRPARAM(1, helper, TypeLeapHelper);
	interpreter->ClearParams();

	LeapMotionHelper * leapHelper = helper.GetPointer<LeapMotionHelper>();

	interpreter->PushParam(ScriptParam(ScriptParam::DOUBLE, double(leapHelper->GetFrameDelta())));
}

void ScriptCallbacks::GetLeapNumBones(ScriptInterpreter * interpreter)
{
	POP_PTRPARAM(1, helper, TypeLeapHelper);
	interpreter->ClearParams();
	
	LeapMotionHelper * leapHelper = helper.GetPointer<LeapMotionHelper>();

	unsigned numBones = leapHelper->GetNumBones();

	interpreter->PushParam(ScriptParam(ScriptParam::INT, double(numBones)));
}

void ScriptCallbacks::GetLeapBoneDetails(ScriptInterpreter * interpreter)
{
	POP_PTRPARAM(1, helper, TypeLeapHelper);
	POP_NUMPARAM(2, index);
	interpreter->ClearParams();

	LeapMotionHelper * leapHelper = helper.GetPointer<LeapMotionHelper>();
	
	unsigned uIndex = unsigned(index.nvalue);
	bool visibility = leapHelper->IsBoneVisible(uIndex);
	float length = leapHelper->GetBoneLength(uIndex);
	float radius = leapHelper->GetBoneRadius(uIndex);

	interpreter->PushParam(ScriptParam(ScriptParam::BOOL, double(visibility)));
	interpreter->PushParam(ScriptParam(ScriptParam::DOUBLE, double(length)));
	interpreter->PushParam(ScriptParam(ScriptParam::DOUBLE, double(radius)));
}

void ScriptCallbacks::GetLeapBonePosition(ScriptInterpreter * interpreter)
{
	POP_PTRPARAM(1, helper, TypeLeapHelper);
	POP_NUMPARAM(2, index);
	interpreter->ClearParams();

	LeapMotionHelper * leapHelper = helper.GetPointer<LeapMotionHelper>();

	unsigned uIndex = unsigned(index.nvalue);
	glm::mat4 matrix = leapHelper->GetBoneMatrix(uIndex);
	glm::vec4 position = matrix[3];

	interpreter->PushParam(ScriptParam(ScriptParam::DOUBLE, double(position.x)));
	interpreter->PushParam(ScriptParam(ScriptParam::DOUBLE, double(position.y)));
	interpreter->PushParam(ScriptParam(ScriptParam::DOUBLE, double(position.z)));
}

void ScriptCallbacks::SetLeapPosition(ScriptInterpreter * interpreter)
{
	POP_PTRPARAM(1, helper, TypeLeapHelper);
	POP_NUMPARAM(2, x);
	POP_NUMPARAM(3, y);
	POP_NUMPARAM(4, z);
	interpreter->ClearParams();

	LeapMotionHelper * leapHelper = helper.GetPointer<LeapMotionHelper>();
	
	leapHelper->SetPosition(glm::vec3(float(x.nvalue), float(y.nvalue), float(z.nvalue)));
}

void ScriptCallbacks::SetLeapScale(ScriptInterpreter * interpreter)
{
	POP_PTRPARAM(1, helper, TypeLeapHelper);
	POP_NUMPARAM(2, scale);
	interpreter->ClearParams();

	LeapMotionHelper * leapHelper = helper.GetPointer<LeapMotionHelper>();
	leapHelper->SetUniformScale(float(scale.nvalue));
}

void ScriptCallbacks::GetLeapBoneMatrix(ScriptInterpreter * interpreter)
{
	POP_PTRPARAM(1, helper, TypeLeapHelper);
	POP_NUMPARAM(2, index);

	LeapMotionHelper * leapHelper = helper.GetPointer<LeapMotionHelper>();
	glm::mat4 matrix = leapHelper->GetBoneMatrix(unsigned(index.nvalue));
	unsigned matrixTypeHandle = interpreter->GetSpecialPtrType(ScriptInterpreter::TypeMatrix4);
	interpreter->PushParam(new BufferCopyPtr(&matrix, sizeof(glm::mat4), matrixTypeHandle));
}

} // namespace Ingenuity
