#include "stdafx.h"

#include "WavefrontLoader.h"

#include "GpuApi.h"
#include "AssetMgr.h"
#include "StepMgr.h"
#include "GeoBuilder.h"

#include <sstream>

namespace Ingenuity {

WavefrontLoader::WavefrontLoader(
		StepMgr * steppables,
		AssetMgr * assets,
		Gpu::Api * gpu,
		Files::Directory * directory,
		const wchar_t * path,
		bool noNormals,
		bool noTangents,
		bool consolidate)
	: SteppableLoader(steppables, directory, path, WavefrontModelAsset)
	, state(Initializing)
	, path(path)
	, gpu(gpu)
	, assets(assets)
	, steppables(steppables)
	, model(0)
	, debugMesh(0)
	, objParser(noNormals)
	, mtlParser(assets)
	, noTangents(noTangents)
	, genDebugMesh(false)
	, finished(false)
	, debugMeshAcquired(false)
{
}

WavefrontLoader::~WavefrontLoader()
{
	if(debugMesh && !debugMeshAcquired) delete debugMesh;
}

Gpu::ComplexModel * WavefrontLoader::LoadModel()
{
	SetConsolidate(true);

	state = Initializing;

	while(state != Idle)
	{
		Step();
	}

	return model;
}

void WavefrontLoader::Step()
{
	Files::Api * files = assets->GetFileApi();

	switch(state)
	{
	case Initializing:
	{
		if(model) delete model; // delete its child meshes first??
		model = 0;

		int directoryIndex = path.find_last_of(L"\\/");
		subDirectory = path.substr(0, directoryIndex + 1);
		mtlParser.SetDirectory(directory, subDirectory.c_str());

		objParser.Reset();

		if(response.buffer) delete[] response.buffer;
		response.buffer = 0;
		response.bufferLength = 0;
		response.complete = false;
		fileOffset = 0;

		files->OpenAndRead(directory, path.c_str(), &response);

		state = OpeningMesh;

		break;
	}
	case OpeningMesh:
	{
		if(response.complete)
		{
			if(response.buffer)
			{
				fileString = response.buffer;
				state = LoadingMesh;
			}
			else
			{
				finished = true; break;
			}
		}
		break;
	}
	case LoadingMesh:
	{
		if(!objParser.IsReady())
		{
			objParser.ParseLine(gpu, GetNextLine());
		}
		else
		{
			if(objParser.hasMaterialLib())
			{
				std::string cMaterialFile(objParser.GetMaterialLib());
				std::wstring materialFile(cMaterialFile.begin(), cMaterialFile.end());
				std::wstring materialPath = (subDirectory + materialFile);

				if(response.buffer) delete[] response.buffer;
				response.buffer = 0;
				response.bufferLength = 0;
				response.complete = false;
				fileOffset = 0;

				files->OpenAndRead(directory, materialPath.c_str(), &response);

				state = OpeningMtl;
			}
			else
			{
				state = CreatingMeshes;
			}
		}
		break;
	}
	case OpeningMtl:
	{
		if(response.complete)
		{
			if(response.buffer)
			{
				fileString = response.buffer;
				state = LoadingMtl;
			}
			else
			{
				finished = true; break;
			}
		}
		break;
	}
	case LoadingMtl:
	{
		if(!mtlParser.IsReady())
		{
			mtlParser.ParseLine(GetNextLine());
		}
		else
		{
			state = CreatingMeshes;
		}
		break;
	}
	case CreatingMeshes:
	{
		model = new Gpu::ComplexModel(objParser.GetNumMeshes());

		for(unsigned i = 0; i < objParser.GetNumMeshes(); ++i)
		{
			if(objParser.hasMaterialLib())
			{
				WavefrontMtl* mtl = mtlParser.GetMtl(objParser.GetMaterial(i));
				if(mtl)
				{
					if(mtl->normal && !noTangents)
					{
						LocalMesh * lmesh = objParser.GetMesh(i);
						GeoBuilder().GenerateTangents(lmesh->vertexBuffer,
							lmesh->numTriangles, lmesh->indexBuffer);

						TangentGenerator * tangentGenerator = new TangentGenerator(lmesh);
						steppables->Add(tangentGenerator);
						tangentTasks.push_back(tangentGenerator);

						//GeoBuilder().GenerateTangents(lmesh->vertexBuffer, 
						//	lmesh->numTriangles, lmesh->indexBuffer);
						//GeoBuilder()->GenerateRotationalTangents(lmesh->vertexBuffer,
						//	lmesh->numTriangles / 3,lmesh->indexBuffer);
						model->models[i].normalMap = mtl->normal;

						if(genDebugMesh && i == 0) GenerateDebugMesh(lmesh);
					}

					model->models[i].texture = mtl->tex;
				}
			}

			//copyTasks.push_back(gpu->CreateLargeGpuMesh(objParser.GetMesh(i)));

			/*model->models[i].mesh = objParser.GetMesh(i)->GpuOnly(gpu);*/
		}

		if(tangentTasks.size() > 0)
		{
			state = GeneratingTangents;
		}
		else
		{
			state = Finalizing;
		}
		break;
	}
		//case CopyingMeshes:
		//{
		//	bool copiesFinished = true;

		//	for(unsigned i = 0; i < copyTasks.size(); ++i)
		//	{
		//		if(copyTasks[i]->IsFinished())
		//		{
		//			model->models[i].mesh = copyTasks[i]->GetMesh();
		//		}
		//		else
		//		{
		//			copiesFinished = false;
		//		}
		//	}
		//	
		//	if(copiesFinished)
		//	{
		//		state = Finalizing;
		//	}
		//	break;
		//}
	case GeneratingTangents:
	{
		bool tangentsReady = true;

		for(unsigned i = 0; i < tangentTasks.size(); ++i)
		{
			if(tangentTasks[i]->IsFinished())
			{
				TangentGenerator * tangentGen = tangentTasks[i];
				tangentTasks.erase(tangentTasks.begin() + i);
				steppables->Remove(tangentGen);
				delete tangentGen;
				i--;
			}
			else
			{
				tangentsReady = false;
				break;
			}
		}

		if(tangentsReady)
		{
			state = Finalizing;
		}
		break;
	}
	case Finalizing:
	{
		//for(unsigned i = 0; i < copyTasks.size(); ++i)
		//{
		//	delete copyTasks[i];
		//}
		//copyTasks.clear();

		for(unsigned i = 0; i < objParser.GetNumMeshes(); ++i)
		{
			model->models[i].boundingSphere = GeoBuilder().GenerateBoundingSphere(objParser.GetMesh(i)->vertexBuffer);
			model->models[i].mesh = objParser.GetMesh(i)->ToGpuMesh(gpu);
			model->models[i].destructMesh = true;
		}

		finished = true;
		state = Idle;
		break;
	}
	}
}

float WavefrontLoader::GetProgress()
{
	if(fileString.length() > 0)
	{
		float numerator = (float)fileOffset;// meshFile->lineOffset;
		float denominator = (float)fileString.length();// meshFile->GetByteLength();
		return numerator / denominator;
	}
	return 1.0f;
}

bool WavefrontLoader::IsFinished()
{
	return finished;
}

void WavefrontLoader::GenerateDebugMesh(LocalMesh * input)
{
	if(!input) return;
	if(input->vertexBuffer->GetVertexType() != VertexType_PosNorTanTex) return;

	unsigned numVertices = input->vertexBuffer->GetLength();
	unsigned numDebugVertices = numVertices * 3; // A triangle in the debug mesh for each vertex in the original mesh

	VertexBuffer<Vertex_PosCol> * verts = new VertexBuffer<Vertex_PosCol>(numDebugVertices);
	Vertex_PosCol * vertData = (Vertex_PosCol *)verts->GetData();
	Vertex_PosNorTanTex * srcData = (Vertex_PosNorTanTex *)input->vertexBuffer->GetData();

	unsigned * k = new unsigned[numDebugVertices];

	for(unsigned i = 0; i < numVertices; i++)
	{
		int tri = i * 3;
		vertData[tri].position = srcData[i].position;
		vertData[tri + 1].position = srcData[i].position + (srcData[i].tangent * 0.05f);
		vertData[tri + 2].position = srcData[i].position + glm::vec3(0.0f, 0.005f, 0.0f);
	}

	for(unsigned i = 0; i < numDebugVertices; i++)
	{
		vertData[i].color = glm::vec3(1.0f, 0.0f, 0.0f);
		k[i] = i;
	}

	debugMesh = gpu->CreateGpuMesh(verts, numVertices, k);

	delete verts;
	delete[] k;
}

Gpu::Mesh * WavefrontLoader::GetDebugMesh()
{
	if(debugMesh)
	{
		debugMeshAcquired = true;
	}
	return debugMesh;
}

} // namespace Ingenuity
