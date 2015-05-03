#pragma once

#include "AssetMgr.h"
#include "FilesApi.h"
#include "GpuApi.h"

#include "ModelEncoder.h"

struct aiNode;
struct aiScene;

namespace Assimp
{
	class Importer;
}

namespace Ingenuity
{

struct LocalMesh;


class AssimpLoader : public SteppableLoader
{
private:
	class AssimpIOStream;
	class AssimpIOSystem;
	class AssimpProgressHandler;

	struct AssimpMaterial
	{
		std::string name;
		std::wstring diffuseTexturePath;
		std::wstring normalTexturePath;
		std::wstring cubeTexturePath;
		glm::vec4 diffuseColor = glm::vec4(1.0f);
		
		Gpu::Texture * diffuseTexture = 0;
		Gpu::Texture * normalTexture = 0;
		Gpu::CubeMap * cubeTexture = 0;
	};

private:
	AssimpIOSystem * assimpIoSystem;
	Assimp::Importer * assimpImporter;
	const aiScene * assimpScene;

	AssetMgr * assets;
	Gpu::Api * gpu;

	std::vector<AssimpMaterial> materials;
	std::vector<LocalMesh*> localMeshes;
	std::vector<unsigned> meshMaterials;
	Gpu::ComplexModel * resultModel;
	float progress;
	unsigned textureTicket;

	void ProcessMaterials(const aiScene * scene);
	void ProcessLoadedTextures();
	void TraverseScene(const aiScene * scene);
	void TraverseNode(const aiScene * scene, const aiNode * nd, const glm::mat4 & parentTransform);

public:
	AssimpLoader(
		StepMgr * steppables, 
		AssetMgr * assets, 
		Gpu::Api * gpu, 
		Files::Directory * dir, 
		const wchar_t * path, 
		AssetType type);
	virtual ~AssimpLoader();

	virtual void Load() override;
	virtual void Step() override;
	virtual float GetProgress() override;
	virtual bool IsFinished() override;
	virtual IAsset * GetAsset() override;

	std::vector<ModelEncoder::ModelMeta> ToEncoderModels();
};

} // namespace Ingenuity