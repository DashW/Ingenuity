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

unsigned ScriptTypes::typeHandles[TypeCount];

Files::Directory * ScriptCallbacks::projectDirectory = 0;

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
	if(strcmp(name, "DataDir") == 0)
	{
		directoryPtr = IngenuityHelper::GetDataDirectory();
	}
	if(strcmp(name, "ProjectDir") == 0)
	{
		directoryPtr = projectDirectory;
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
		if(strcmp(type, "ColladaModel") == 0)
		{
			assetType = ColladaModelAsset;
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

	return ScriptParam(scriptFloatArray, ScriptTypes::GetHandle(TypeFloatArray));
}

ScriptParam ScriptCallbacks::IndexBufferToFloats(ScriptInterpreter * interpreter, unsigned * indexBuffer, unsigned numTriangles)
{
	if(!interpreter || !indexBuffer) return ScriptParam();

	ScriptFloatArray * scriptFloatArray = new ScriptFloatArray(numTriangles * 3);

	for(unsigned i = 0; i < numTriangles * 3; i++)
	{
		scriptFloatArray->floats[i] = float(indexBuffer[i]);
	}
	
	return ScriptParam(scriptFloatArray, ScriptTypes::GetHandle(TypeFloatArray));
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

	bool hasModuleName = modulename.type == ScriptParam::STRING;
	
	files->OpenAndRead(dir, wpath.c_str(), new RequireResponse(interpreter, wpath, hasModuleName ? modulename.svalue : 0));
}

void ScriptCallbacks::CreateWindow(ScriptInterpreter * interpreter)
{
	ScriptParam width = interpreter->PopParam();
	ScriptParam height = interpreter->PopParam();
	interpreter->ClearParams();

	Gpu::Api * gpu = interpreter->GetApp()->gpu;
	PlatformWindow * window = interpreter->GetApp()->platform->CreatePlatformWindow(gpu, 
		width.IsNumber() ? unsigned(width.nvalue) : 640,
		height.IsNumber() ? unsigned(height.nvalue) : 480);

	interpreter->PushParam(ScriptParam(window, ScriptTypes::GetHandle(TypePlatformWindow)));;
}

void ScriptCallbacks::GetMainWindow(ScriptInterpreter * interpreter)
{
	PlatformWindow * window = interpreter->GetApp()->platform->GetMainPlatformWindow();
	interpreter->PushParam(ScriptParam(new NonDeletingPtr(window, ScriptTypes::GetHandle(TypePlatformWindow))));;
}

void ScriptCallbacks::GetWindowStatus(ScriptInterpreter * interpreter)
{

}

void ScriptCallbacks::GetWindowSurface(ScriptInterpreter * interpreter)
{
	POP_PTRPARAM(1, window, TypePlatformWindow);
	interpreter->ClearParams();

	Gpu::DrawSurface * surface = interpreter->GetApp()->gpu->GetWindowDrawSurface(window.GetPointer<PlatformWindow>());
	if(surface)
	{
		interpreter->PushParam(ScriptParam(new NonDeletingPtr(surface, ScriptTypes::GetHandle(TypeGpuDrawSurface))));
	}
}

void ScriptCallbacks::SetWindowProps(ScriptInterpreter * interpreter)
{
	POP_PTRPARAM(1, window, TypePlatformWindow);
	ScriptParam width = interpreter->PopParam();
	ScriptParam height = interpreter->PopParam();
	ScriptParam undecorated = interpreter->PopParam();
	ScriptParam resizeable = interpreter->PopParam();
	interpreter->ClearParams();

	PlatformWindow * platformWindow = window.GetPointer<PlatformWindow>();

	if(undecorated.type == ScriptParam::BOOL)
	{
		platformWindow->SetUndecorated(undecorated.nvalue > 0.0);
	}

	if(resizeable.type == ScriptParam::BOOL)
	{
		platformWindow->SetResizeable(resizeable.nvalue > 0.0);
	}

	if(width.IsNumber() && height.IsNumber())
	{
		platformWindow->SetSize(unsigned(width.nvalue), unsigned(height.nvalue));
	}

	//interpreter->GetApp()->gpu->OnWindowCreated(platformWindow);
}

void ScriptCallbacks::CreateFloatArray(ScriptInterpreter * interpreter)
{
	ScriptParam content = interpreter->PopParam();

	ScriptFloatArray * floatArray = 0;

	if(content.IsNumber())
	{
		if(content.nvalue < 0.0)
		{
			interpreter->ThrowError("Attempted to create FloatArray of negative length!");
			return;
		}
		floatArray = new ScriptFloatArray(unsigned(content.nvalue));
	}
	if(content.CheckPointer(interpreter->GetSpecialPtrType(ScriptInterpreter::TypeVector4)))
	{
		floatArray = new ScriptFloatArray(4);
		*((glm::vec4*)floatArray->floats) = *(content.GetPointer<glm::vec4>());
	}
	if(content.CheckPointer(interpreter->GetSpecialPtrType(ScriptInterpreter::TypeMatrix4)))
	{
		floatArray = new ScriptFloatArray(16);
		*((glm::mat4*)floatArray->floats) = *(content.GetPointer<glm::mat4>());
	}

	interpreter->PushParam(ScriptParam(floatArray, ScriptTypes::GetHandle(TypeFloatArray)));
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

	if(index.nvalue < 0.0 || uindex + numVals > scriptFloatArray->length)
	{
		interpreter->ThrowError("Attempted to get a FloatArray index out of range");
		return;
	}

	for(unsigned i = uindex; i < uindex + numVals; ++i)
	{
		interpreter->PushParam(ScriptParam(ScriptParam::DOUBLE, double(scriptFloatArray->floats[i])));
	}
}

void ScriptCallbacks::GetFloatArrayLength(ScriptInterpreter * interpreter)
{
	POP_PTRPARAM(1, floatArray, TypeFloatArray);
	interpreter->ClearParams();
	ScriptFloatArray * scriptFloatArray = floatArray.GetPointer<ScriptFloatArray>();
	interpreter->PushParam(ScriptParam(ScriptParam::DOUBLE, double(scriptFloatArray->length)));
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

	// Shouldn't this default to false??
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
	POP_NUMPARAM(1, length)

		LocalMesh * localCapsule = GeoBuilder().BuildCapsule(0.5f, float(length.nvalue), 20, 20);

	interpreter->PushParam(VertexBufferToFloats(interpreter, localCapsule->vertexBuffer));
	interpreter->PushParam(IndexBufferToFloats(interpreter, localCapsule->indexBuffer, localCapsule->numTriangles));

	delete localCapsule;
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

		asset = interpreter->GetApp()->assets->GetAsset<IAsset>(directoryPtr, subPathWide.c_str());
	}
	else
	{
		asset = interpreter->GetApp()->assets->GetAsset<IAsset>(directoryOrName.svalue);
	}

	if(asset)
	{
		switch(asset->GetAssetType())
		{
			case TextureAsset:
			{
				Gpu::Texture * texAsset = dynamic_cast<Gpu::Texture*>(asset);
				interpreter->PushParam(ScriptParam(new NonDeletingPtr(texAsset, ScriptTypes::GetHandle(TypeGpuTexture))));
				break;
			}
			case CubeMapAsset:
			{
				Gpu::CubeMap * cubeAsset = dynamic_cast<Gpu::CubeMap*>(asset);
				interpreter->PushParam(ScriptParam(new NonDeletingPtr(cubeAsset, ScriptTypes::GetHandle(TypeGpuCubeMap))));
				break;
			}
			case VolumeTexAsset:
			{
				Gpu::VolumeTexture * vTexAsset = dynamic_cast<Gpu::VolumeTexture*>(asset);
				interpreter->PushParam(ScriptParam(new NonDeletingPtr(vTexAsset, ScriptTypes::GetHandle(TypeGpuVolumeTexture))));
				break;
			}
			case ShaderAsset:
			{
				Gpu::Shader * shdrAsset = dynamic_cast<Gpu::Shader*>(asset);
				interpreter->PushParam(ScriptParam(new NonDeletingPtr(shdrAsset, ScriptTypes::GetHandle(TypeGpuShader))));
				break;
			}
			case ModelAsset:
			{
				Gpu::ComplexModel * mdlAsset = dynamic_cast<Gpu::ComplexModel*>(asset);
				interpreter->PushParam(ScriptParam(mdlAsset, ScriptTypes::GetHandle(TypeGpuComplexModel)));
				break;
			}
			case RawHeightMapAsset:
			{
				HeightParser * heightParser = dynamic_cast<HeightParser*>(asset);
				interpreter->PushParam(ScriptParam(new NonDeletingPtr(heightParser, ScriptTypes::GetHandle(TypeHeightParser))));
				break;
			}
			case ImageAsset:
			{
				Image::Buffer * imgAsset = dynamic_cast<Image::Buffer*>(asset);
				interpreter->PushParam(ScriptParam(new NonDeletingPtr(imgAsset, ScriptTypes::GetHandle(TypeImageBuffer))));
				break;
			}
			case AudioAsset:
			{
				Audio::Item * audioItem = dynamic_cast<Audio::Item*>(asset);
				interpreter->PushParam(ScriptParam(new NonDeletingPtr(audioItem, ScriptTypes::GetHandle(TypeAudioItem))));
				break;
			}
			case SvgAsset:
			{
				SvgParser * svgParser = dynamic_cast<SvgParser*>(asset);
				interpreter->PushParam(ScriptParam(new NonDeletingPtr(svgParser, ScriptTypes::GetHandle(TypeSVGParser))));
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
		
		interpreter->PushParam(ScriptParam(model, ScriptTypes::GetHandle(TypeGpuComplexModel)));
	}
}

void ScriptCallbacks::GetSVGModel(ScriptInterpreter * interpreter)
{
	POP_PTRPARAM(1, svg, TypeSVGParser);
	POP_NUMPARAM(2, stroke);
	
	SvgParser * svgParser = svg.GetPointer<SvgParser>();
	Gpu::Api * gpu = interpreter->GetApp()->gpu;
	Gpu::ComplexModel * model = 0;
	
	if(stroke.nvalue > 0.0)
	{
		POP_NUMPARAM(3, anim);
		model = svgParser->GetAnimatedStroke(gpu, float(anim.nvalue));
	}
	else
	{
		model = svgParser->GetModel(gpu);
	}

	interpreter->ClearParams();
	if(model)
	{
		interpreter->PushParam(ScriptParam(model, ScriptTypes::GetHandle(TypeGpuComplexModel)));
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

	if(ScriptTypes::CheckPtrType(sound,TypeAudioItem))
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

void ScriptCallbacks::CreatePhysicsWorld(ScriptInterpreter * interpreter)
{
	PhysicsWorld * physicsWorld = interpreter->GetApp()->physics->CreateWorld(0.0f);

	interpreter->ClearParams();
	interpreter->PushParam(ScriptParam(physicsWorld, ScriptTypes::GetHandle(TypePhysicsWorld)));
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

	interpreter->PushParam(ScriptParam(material, ScriptTypes::GetHandle(TypePhysicsMaterial)));
}

void ScriptCallbacks::CreatePhysicsAnchor(ScriptInterpreter * interpreter)
{
	interpreter->ClearParams();
	PhysicsObject * physicsObject = interpreter->GetApp()->physics->CreateAnchor();
	interpreter->PushParam(ScriptParam(physicsObject, ScriptTypes::GetHandle(TypePhysicsObject)));
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

	interpreter->PushParam(ScriptParam(physicsObject, ScriptTypes::GetHandle(TypePhysicsObject)));
}

void ScriptCallbacks::CreatePhysicsSphere(ScriptInterpreter * interpreter)
{
	POP_NUMPARAM(1, r);
	POP_NUMPARAM(2, kinematic);
	interpreter->ClearParams();

	PhysicsObject * physicsObject = interpreter->GetApp()->physics->CreateSphere(float(r.nvalue), kinematic.nvalue > 0.0);

	interpreter->PushParam(ScriptParam(physicsObject, ScriptTypes::GetHandle(TypePhysicsObject)));
}

void ScriptCallbacks::CreatePhysicsCapsule(ScriptInterpreter * interpreter)
{
	POP_NUMPARAM(1, r);
	POP_NUMPARAM(2, l);
	POP_NUMPARAM(3, kinematic);
	
	PhysicsObject * physicsObject = interpreter->GetApp()->physics->CreateCapsule(float(r.nvalue), float(l.nvalue), kinematic.nvalue > 0.0);

	interpreter->PushParam(ScriptParam(physicsObject, ScriptTypes::GetHandle(TypePhysicsObject)));
}

void ScriptCallbacks::CreatePhysicsMesh(ScriptInterpreter * interpreter)
{
	POP_PARAM(1, type, STRING);
	POP_PTRPARAM(2, vtx, TypeFloatArray);
	POP_PTRPARAM(3, idx, TypeFloatArray);
	interpreter->ClearParams();

	LocalMesh * localMesh = FloatsToLocalMesh(interpreter, type, vtx, idx);

	PhysicsObject * physicsObject = interpreter->GetApp()->physics->CreateMesh(localMesh, false, true);

	interpreter->PushParam(ScriptParam(physicsObject, ScriptTypes::GetHandle(TypePhysicsObject)));
}

void ScriptCallbacks::CreatePhysicsHeightmap(ScriptInterpreter * interpreter)
{
	POP_PTRPARAM(1, heightmap, TypeHeightParser);
	interpreter->ClearParams();

	HeightParser * heightParser = heightmap.GetPointer<HeightParser>();
	PhysicsObject * physicsObject = interpreter->GetApp()->physics->CreateHeightmap(heightParser);

	interpreter->PushParam(ScriptParam(physicsObject, ScriptTypes::GetHandle(TypePhysicsObject)));
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

	interpreter->PushParam(ScriptParam(spring, ScriptTypes::GetHandle(TypePhysicsSpring)));
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
	ScriptParam isLocal = interpreter->PopParam();
	interpreter->ClearParams();

	PhysicsObject * physicsObject = object.GetPointer<PhysicsObject>();
	PhysicsApi * physics = interpreter->GetApp()->physics;

	glm::mat4 matrix;
	if(isLocal.type == ScriptParam::BOOL)
	{
		if(isLocal.nvalue > 0.0)
		{
			matrix = physics->GetLocalMatrix(physicsObject);
		}
		else
		{
			matrix = physics->GetGlobalMatrix(physicsObject);
		}
	}
	else
	{
		matrix = physics->GetMatrix(physicsObject);
	}

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
		prop = PhysicsSpring::ForceDamping;
	}
	if(strcmp(name.svalue, "torqueDamping") == 0)
	{
		prop = PhysicsSpring::TorqueDamping;
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
	if(strcmp(name.svalue, "broken") == 0)
	{
		prop = PhysicsSpring::Broken;
	}

	if(prop > -1)
	{
		interpreter->GetApp()->physics->SetSpringProperty(
			spring.GetPointer<PhysicsSpring>(),
			(PhysicsSpring::Property) prop,
			float(value.nvalue));
	}
	else
	{
		interpreter->ThrowError("Unrecognized physics spring property!");
	}
}

void ScriptCallbacks::CreatePhysicsRagdoll(ScriptInterpreter * interpreter)
{
	POP_PTRPARAM(1, world, TypePhysicsWorld);
	interpreter->ClearParams();

	PhysicsWorld * physicsWorld = world.GetPointer<PhysicsWorld>();
	PhysicsRagdoll * physicsRagdoll = interpreter->GetApp()->physics->CreateRagdoll(physicsWorld);

	interpreter->PushParam(ScriptParam(physicsRagdoll, ScriptTypes::GetHandle(TypePhysicsRagdoll)));
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
		interpreter->PushParam(ScriptParam(new NonDeletingPtr(physicsObject, ScriptTypes::GetHandle(TypePhysicsObject))));
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
		interpreter->PushParam(ScriptParam(new NonDeletingPtr(physicsObject, ScriptTypes::GetHandle(TypePhysicsObject))));
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

	interpreter->PushParam(ScriptParam(complexModel, ScriptTypes::GetHandle(TypeGpuComplexModel)));
}

#ifdef USE_LEAPMOTION_HELPER

void ScriptCallbacks::CreateLeapHelper(ScriptInterpreter * interpreter)
{
	LeapMotionHelper * leapHelper = new LeapMotionHelper();

	interpreter->PushParam(ScriptParam(leapHelper, ScriptTypes::GetHandle(TypeLeapHelper)));
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

void ScriptCallbacks::GetLeapFinger(ScriptInterpreter * interpreter)
{
	POP_PTRPARAM(1, helper, TypeLeapHelper);
	POP_NUMPARAM(2, index);

	LeapMotionHelper * leapHelper = helper.GetPointer<LeapMotionHelper>();
	glm::vec3 position = leapHelper->GetFingerPosition(unsigned(index.nvalue));
	
	interpreter->PushParam(ScriptParam(ScriptParam::BOOL, leapHelper->IsFingerVisible(unsigned(index.nvalue))));
	interpreter->PushParam(ScriptParam(ScriptParam::DOUBLE, double(position.x)));
	interpreter->PushParam(ScriptParam(ScriptParam::DOUBLE, double(position.y)));
	interpreter->PushParam(ScriptParam(ScriptParam::DOUBLE, double(position.z)));
}

#endif

} // namespace Ingenuity
