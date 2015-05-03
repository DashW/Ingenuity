#include "ModelEncoder.h"

#include <regex>

namespace Ingenuity {

ModelEncoder::Loader::~Loader()
{
	for(unsigned i = 0; i < models.size(); ++i)
	{
		delete models[i].mesh;
	}
}

void ModelEncoder::Loader::Respond()
{
	if(buffer)
	{
		models = DecodeModels(buffer, bufferLength);

		int directoryIndex = path.find_last_of(L"\\/");
		std::wstring subDirPath = path.substr(0, directoryIndex + 1);
		subDirPath = std::regex_replace(subDirPath, std::wregex(L"\\b\\/"), L"//");
		subDir = files->GetSubDirectory(directory, subDirPath.c_str());
		if(!subDir) subDir = directory;

		AssetBatch batch;
		for(unsigned i = 0; i < models.size(); ++i)
		{
			if(*models[i].diffuseTexturePath)
			{
				batch.emplace_back(subDir, models[i].diffuseTexturePath, TextureAsset);
			}
			if(*models[i].normalTexturePath)
			{
				batch.emplace_back(subDir, models[i].normalTexturePath, TextureAsset);
			}
			if(*models[i].cubeTexturePath)
			{
				batch.emplace_back(subDir, models[i].cubeTexturePath, CubeMapAsset);
			}
		}
		if(batch.size() > 0)
		{
			texTicket = assets->Load(batch);
		}
	}
}

bool ModelEncoder::Loader::IsAssetReady()
{
	if(Files::Response::complete && (texTicket == -1 || assets->IsLoaded(texTicket)))
	{
		if(!asset)
		{
			Gpu::ComplexModel * gpuModel = new Gpu::ComplexModel(models.size());

			for(unsigned i = 0; i < models.size(); ++i)
			{
				gpuModel->models[i].boundingSphere = GeoBuilder().GenerateBoundingSphere(models[i].mesh->vertexBuffer);
				gpuModel->models[i].mesh = models[i].mesh->ToGpuMesh(gpu);
				gpuModel->models[i].color = models[i].diffuseColor;
				gpuModel->models[i].destructMesh = true;

				if(*models[i].diffuseTexturePath)
				{
					gpuModel->models[i].texture = assets->GetAsset<Gpu::Texture>(subDir, models[i].diffuseTexturePath);
				}
				if(*models[i].normalTexturePath)
				{
					gpuModel->models[i].normalMap = assets->GetAsset<Gpu::Texture>(subDir, models[i].normalTexturePath);
				}
				if(*models[i].cubeTexturePath)
				{
					gpuModel->models[i].cubeMap = assets->GetAsset<Gpu::CubeMap>(subDir, models[i].cubeTexturePath);
				}
			}

			asset = gpuModel;
			texTicket = -1;
		}

		return true;
	}
	return false;
}

unsigned ModelEncoder::GetEncodedBytes(LocalMesh * mesh)
{
	if(!mesh) return sizeof(MeshMeta);

	unsigned verticesByteLength = mesh->vertexBuffer->GetLength() * mesh->vertexBuffer->GetElementSize();
	unsigned indicesByteLength = mesh->GetNumIndices() * sizeof(unsigned);

	return sizeof(MeshMeta) + verticesByteLength + indicesByteLength;
}

char * ModelEncoder::EncodeMesh(LocalMesh * mesh, unsigned & outLength)
{
	MeshMeta meshMeta(mesh);

	unsigned verticesByteLength = mesh->vertexBuffer->GetLength() * mesh->vertexBuffer->GetElementSize();
	unsigned indicesByteLength = mesh->GetNumIndices() * sizeof(unsigned);

	outLength = sizeof(meshMeta) + verticesByteLength + indicesByteLength;

	char * data = new char[outLength];

	memcpy(data, &meshMeta, sizeof(meshMeta));
	memcpy(data + sizeof(meshMeta), mesh->vertexBuffer->GetData(), verticesByteLength);
	memcpy(data + sizeof(meshMeta) + verticesByteLength, mesh->indexBuffer, indicesByteLength);

	return data;
}

LocalMesh * ModelEncoder::DecodeMesh(char * data, unsigned dataLength)
{
	if(!data || dataLength < sizeof(MeshMeta)) return 0;

	MeshMeta meshMeta;
	memcpy(&meshMeta, data, sizeof(meshMeta));

	unsigned verticesByteLength = meshMeta.numVertices * VertApi::GetVertexSize(meshMeta.vertexType);
	unsigned indicesByteLength = meshMeta.numIndices * sizeof(unsigned);

	if(dataLength < sizeof(MeshMeta) + verticesByteLength + indicesByteLength) return 0;
		
	IVertexBuffer * vertexBuffer = 0;
	switch(meshMeta.vertexType)
	{
	case VertexType_Pos:
		vertexBuffer = new VertexBuffer<Vertex_Pos>(meshMeta.numVertices);
		break;
	case VertexType_PosCol:
		vertexBuffer = new VertexBuffer<Vertex_PosCol>(meshMeta.numVertices);
		break;
	case VertexType_PosNor:
		vertexBuffer = new VertexBuffer<Vertex_PosNor>(meshMeta.numVertices);
		break;
	case VertexType_PosTex:
		vertexBuffer = new VertexBuffer<Vertex_PosTex>(meshMeta.numVertices);
		break;
	case VertexType_PosNorTex:
		vertexBuffer = new VertexBuffer<Vertex_PosNorTex>(meshMeta.numVertices);
		break;
	case VertexType_PosNorTanTex:
		vertexBuffer = new VertexBuffer<Vertex_PosNorTanTex>(meshMeta.numVertices);
		break;
	}

	memcpy(vertexBuffer->GetData(), data + sizeof(MeshMeta), verticesByteLength);

	unsigned * indexBuffer = new unsigned[meshMeta.numIndices];

	memcpy(indexBuffer, data + sizeof(MeshMeta) + verticesByteLength, indicesByteLength);

	return new LocalMesh(vertexBuffer, indexBuffer, meshMeta.numIndices);
}

char * ModelEncoder::EncodeModel(ModelMeta meta, unsigned & outLength)
{
	unsigned meshLength = 0;
	char * meshData = EncodeMesh(meta.mesh, meshLength);
	
	meta.mesh = 0;

	outLength = sizeof(ModelMeta) + meshLength;
	char * data = new char[outLength];

	memcpy(data, &meta, sizeof(ModelMeta));
	memcpy(data + sizeof(ModelMeta), meshData, meshLength);

	delete[meshLength] meshData;

	return data;
}

ModelEncoder::ModelMeta ModelEncoder::DecodeModel(char * data, unsigned dataLength)
{
	ModelMeta modelMeta;
	memcpy(&modelMeta, data, sizeof(ModelMeta));
	modelMeta.mesh = DecodeMesh(data + sizeof(ModelMeta), dataLength - sizeof(ModelMeta));
	return modelMeta;
}

char * ModelEncoder::EncodeModels(std::vector<ModelMeta> & models, unsigned & outLength)
{
	outLength = 0;
	std::vector<char*> encodedModels;
	std::vector<unsigned> modelByteLengths;

	for(unsigned i = 0; i < models.size(); ++i)
	{
		modelByteLengths.push_back(0);
		encodedModels.push_back(EncodeModel(models[i], modelByteLengths[i]));
		outLength += modelByteLengths[i];
	}

	unsigned currentByte = 0;
	char * data = new char[outLength];

	for(unsigned i = 0; i < encodedModels.size(); ++i)
	{
		memcpy(data + currentByte, encodedModels[i], modelByteLengths[i]);
		delete[modelByteLengths[i]] encodedModels[i];
		currentByte += modelByteLengths[i];
	}

	return data;
}

std::vector<ModelEncoder::ModelMeta> ModelEncoder::DecodeModels(char * data, unsigned dataLength)
{
	std::vector<ModelMeta> modelMetas;

	unsigned prevDataLength = dataLength;

	while(dataLength > sizeof(ModelMeta) && dataLength <= prevDataLength)
	{
		ModelMeta modelMeta = DecodeModel(data, dataLength);
		if(modelMeta.mesh) modelMetas.push_back(modelMeta);
		unsigned bytes = (sizeof(ModelMeta) + GetEncodedBytes(modelMetas.back().mesh));
		prevDataLength = dataLength;
		dataLength -= bytes;
		data += bytes;
	}

	return modelMetas;
}

//Gpu::ComplexModel * ToGpuModel(Gpu::Api * gpu, std::vector<ModelEncoder::ModelMeta> & models)
//{
//	Gpu::ComplexModel * gpuModel = new Gpu::ComplexModel(models.size());
//	
//	for(unsigned i = 0; i < models.size(); ++i)
//	{
//		gpuModel->models[i].mesh = models[i].mesh->ToGpuMesh(gpu);
//		gpuModel->models[i].color = models[i].diffuseColor;
//		//gpuModel->models[i].
//	}
//}

} // namespace Ingenuity
