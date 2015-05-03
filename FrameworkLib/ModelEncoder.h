#pragma once

#include "FilesApi.h"
#include "GeoBuilder.h"

namespace Ingenuity {

class ModelEncoder
{
public:

	struct MeshMeta
	{
		VertexType vertexType;
		unsigned numVertices;
		unsigned numIndices;

		MeshMeta(LocalMesh * mesh)
			: vertexType(mesh->vertexBuffer->GetVertexType())
			, numVertices(mesh->vertexBuffer->GetLength())
			, numIndices(mesh->GetNumIndices())
		{}

		MeshMeta() : vertexType(VertexType_Count), numVertices(0), numIndices(0) {}
	};

	struct ModelMeta
	{
		LocalMesh * mesh;
		wchar_t diffuseTexturePath[256];
		wchar_t normalTexturePath[256];
		wchar_t cubeTexturePath[256];
		glm::vec4 diffuseColor = glm::vec4(1.0f);
	};
	
	class Loader : public SimpleLoader
	{
		Gpu::Api * gpu;
		AssetMgr * assets;
		Files::Directory * subDir;
		std::vector<ModelMeta> models;
		int texTicket;

	public:
		Loader(AssetMgr * assets, Gpu::Api * gpu, Files::Directory * dir, const wchar_t * path)
			: SimpleLoader(assets->GetFileApi(), dir, path, ModelAsset)
			, gpu(gpu)
			, assets(assets)
			, subDir(0)
			, texTicket(-1) 
		{}
		virtual ~Loader();
		virtual void Respond() override;
		virtual bool IsAssetReady() override;
	};

	static unsigned GetEncodedBytes(LocalMesh * mesh);

	static char * EncodeMesh(LocalMesh * mesh, unsigned & outLength);
	static LocalMesh * DecodeMesh(char * data, unsigned dataLength);

	static char * EncodeModel(ModelMeta model, unsigned & outLength);
	static ModelMeta DecodeModel(char * data, unsigned dataLength);

	static char * EncodeModels(std::vector<ModelMeta> & models, unsigned & outLength);
	static std::vector<ModelMeta> DecodeModels(char * data, unsigned dataLength);

	//static Gpu::ComplexModel * ToGpuModel(Gpu::Api * gpu, std::vector<ModelMeta> & models);

};

} // namespace Ingenuity
