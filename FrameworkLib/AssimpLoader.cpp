#include "AssimpLoader.h"

#include "assimp/vector3.h"
#include "assimp/cimport.h"
#include "assimp/IOStream.hpp"
#include "assimp/IOSystem.hpp"
#include "assimp/matrix4x4.h"
#include "assimp/postprocess.h"
#include "assimp/ProgressHandler.hpp"
#include "assimp/scene.h"

#include "GeoBuilder.h"

namespace Ingenuity {

class AssimpLoader::AssimpIOStream : public Assimp::IOStream
{
	Files::Api * files;
	Files::File * file;
	unsigned seekHead;

public:
	AssimpIOStream(Files::Api * files, Files::File * file)
		: files(files), file(file), seekHead(0) {}

	virtual ~AssimpIOStream()
	{
		files->Close(&file);
	}

	virtual size_t Read(void * buffer, size_t pSize, size_t pCount) override
	{
		unsigned int numChars = pSize;
		const char * charBufferPtr = files->ReadChars(file, numChars, seekHead);
		memcpy(buffer, charBufferPtr, numChars);
		delete[] charBufferPtr;
		return numChars;
	}

	virtual size_t Write(const void* pvBuffer, size_t pSize, size_t pCount) override
	{
		// Not yet implemented!
		//__debugbreak();
		return 0;
	}

	virtual aiReturn Seek(size_t pOffset, aiOrigin pOrigin) override
	{
		switch(pOrigin)
		{
		case aiOrigin_SET:
			seekHead = pOffset;
			break;
		case aiOrigin_CUR:
			seekHead += pOffset;
			break;
		case aiOrigin_END:
			seekHead = file->GetByteLength() + pOffset;
			break;
		}
		return AI_SUCCESS;
	}

	virtual size_t Tell() const override
	{
		return seekHead;
	}

	virtual size_t FileSize() const override
	{
		return file->GetByteLength();
	}

	virtual void Flush() override
	{
		// Not yet implemented!
	}
};

class AssimpLoader::AssimpIOSystem : public Assimp::IOSystem
{
	Files::Api * files;
	Files::Directory * localDir;

public:
	AssimpIOSystem(Files::Api * files)
		: files(files)
		, localDir(files->GetKnownDirectory(Files::AppDir))
	{}

	virtual ~AssimpIOSystem() {}

	void SetDirectory(Files::Directory * dir) { localDir = dir; }

	virtual bool Exists(const char* pFile) const override
	{
		if(!pFile) return false;
		std::string sPath(pFile);
		std::wstring wPath(sPath.begin(), sPath.end());

		Files::File * file = files->Open(localDir, wPath.c_str());
		bool result = file && file->openState != Files::Failed;
		files->Close(&file);

		return result;
	}

	virtual char getOsSeparator() const override
	{
		// This *should* always work
		return '/';
	}

	virtual Assimp::IOStream * Open(const char* pFile, const char* pMode = "rb") override
	{
		if(!pFile) return 0;
		std::string sPath(pFile);
		std::wstring wPath(sPath.begin(), sPath.end());

		Files::File * file = files->Open(localDir, wPath.c_str());
		if(file && file->openState != Files::Failed)
		{
			return new AssimpIOStream(files, file);
		}
		return 0;
	}

	virtual void Close(Assimp::IOStream * pFile) override
	{
		delete pFile;
	}
};

class AssimpLoader::AssimpProgressHandler : public Assimp::ProgressHandler
{
	AssimpLoader * owner;

public:
	AssimpProgressHandler(AssimpLoader * creator)
		: owner(creator) {}

	virtual ~AssimpProgressHandler() {}

	virtual bool Update(float percentage = -1.f) override
	{
		owner->progress = percentage;
		return true;
	}
};

AssimpLoader::AssimpLoader(
		StepMgr * steppables, 
		AssetMgr * assets, 
		Gpu::Api * gpu, 
		Files::Directory * dir, 
		const wchar_t * path, 
		AssetType type)
	: SteppableLoader(steppables, dir, path, type)
	, assimpScene(0)
	, assets(assets)
	, gpu(gpu)
	, resultModel(0)
	, progress(0.0f)
	, textureTicket(0)
{
	assimpIoSystem = new AssimpIOSystem(assets->GetFileApi());
	assimpIoSystem->SetDirectory(dir);

	assimpImporter.SetIOHandler(assimpIoSystem);
	assimpImporter.SetProgressHandler(new AssimpProgressHandler(this));
}

AssimpLoader::~AssimpLoader()
{

}

void AssimpLoader::ProcessMaterials(const aiScene * sc)
{
	AssetBatch textureBatch;

	for(unsigned i = 0; i < sc->mNumMaterials; ++i)
	{
		materials.emplace_back();

		aiMaterial * mat = sc->mMaterials[i];

		aiString path;
		std::string sPath;
		if(mat->GetTexture(aiTextureType_DIFFUSE, 0, &path) == AI_SUCCESS)
		{
			sPath = path.C_Str();
			materials.back().diffuseTexturePath.assign(sPath.begin(), sPath.end());
			textureBatch.push_back(AssetRequest(directory, materials.back().diffuseTexturePath.c_str(), TextureAsset));
		}
		if(mat->GetTexture(aiTextureType_NORMALS, 0, &path) == AI_SUCCESS)
		{
			sPath = path.C_Str();
			materials.back().normalTexturePath.assign(sPath.begin(), sPath.end());
			textureBatch.push_back(AssetRequest(directory, materials.back().normalTexturePath.c_str(), TextureAsset));
		}
		if(mat->GetTexture(aiTextureType_REFLECTION, 0, &path) == AI_SUCCESS)
		{
			sPath = path.C_Str();
			materials.back().cubeTexturePath.assign(sPath.begin(), sPath.end());
			textureBatch.push_back(AssetRequest(directory, materials.back().cubeTexturePath.c_str(), CubeMapAsset));
		}
		
		aiColor4D color;
		if(aiGetMaterialColor(mat, AI_MATKEY_COLOR_DIFFUSE, &color) == AI_SUCCESS)
		{
			materials.back().diffuseColor = glm::vec4(color.r, color.g, color.b, color.a);
		}
	}
	
	textureTicket = assets->Load(textureBatch);
}

void AssimpLoader::ProcessLoadedTextures()
{
	for(unsigned i = 0; i < materials.size(); ++i)
	{
		materials[i].diffuseTexture = assets->GetAsset<Gpu::Texture>(directory, materials[i].diffuseTexturePath.c_str());
		materials[i].normalTexture = assets->GetAsset<Gpu::Texture>(directory, materials[i].normalTexturePath.c_str());
		materials[i].cubeTexture = assets->GetAsset<Gpu::CubeMap>(directory, materials[i].cubeTexturePath.c_str());
	}
}

void AssimpLoader::TraverseScene(const aiScene * sc)
{
	TraverseNode(assimpScene, assimpScene->mRootNode, glm::mat4());

	resultModel = new Gpu::ComplexModel(localMeshes.size());

	for(unsigned i = 0; i < localMeshes.size(); ++i)
	{
		resultModel->models[i].boundingSphere = GeoBuilder().GenerateBoundingSphere(localMeshes[i]->vertexBuffer);
		resultModel->models[i].mesh = localMeshes[i]->GpuOnly(gpu);
		resultModel->models[i].destructMesh = true;

		if(meshMaterials[i] < materials.size())
		{
			AssimpMaterial & mat = materials[meshMaterials[i]];
			resultModel->models[i].texture = mat.diffuseTexture;
			resultModel->models[i].normalMap = mat.normalTexture;
			resultModel->models[i].cubeMap = mat.cubeTexture;
			resultModel->models[i].color = mat.diffuseColor;
		}
	}
}

void AssimpLoader::TraverseNode(const aiScene *sc, const aiNode* nd, const glm::mat4 & parentTransform)
{
	aiMatrix4x4 m = nd->mTransformation;

	// update transform
	aiTransposeMatrix4(&m);
	glm::mat4 transformMatrix;
	memcpy(&transformMatrix, &m, 16 * sizeof(float));

	transformMatrix = parentTransform * transformMatrix;

	for(unsigned n = 0; n < nd->mNumMeshes; ++n) {
		const struct aiMesh* mesh = sc->mMeshes[nd->mMeshes[n]];

		IVertexBuffer * vertexBuffer = 0;
		VertexType vertexType;

		if(mesh->mPrimitiveTypes == aiPrimitiveType_TRIANGLE && mesh->HasPositions())
		{
			if(mesh->HasNormals())
			{
				if(mesh->HasTextureCoords(0))
				{
					if(mesh->HasTangentsAndBitangents())
					{
						vertexBuffer = new VertexBuffer<Vertex_PosNorTanTex>(mesh->mNumVertices);
						vertexType = VertexType_PosNorTanTex;
					}
					else
					{
						vertexBuffer = new VertexBuffer<Vertex_PosNorTex>(mesh->mNumVertices);
						vertexType = VertexType_PosNorTex;
					}
				}
				else
				{
					vertexBuffer = new VertexBuffer<Vertex_PosNor>(mesh->mNumVertices);
					vertexType = VertexType_PosNor;
				}
			}
		}
		else
		{
			// ERROR!
		}

		if(vertexBuffer)
		{
			for(unsigned i = 0; i < mesh->mNumVertices; ++i)
			{
				switch(vertexType)
				{
				case VertexType_PosNor:
					((VertexBuffer<Vertex_PosNor>*)vertexBuffer)->Set(i, Vertex_PosNor(
						mesh->mVertices[i].x,
						mesh->mVertices[i].y,
						mesh->mVertices[i].z,
						mesh->mNormals[i].x,
						mesh->mNormals[i].y,
						mesh->mNormals[i].z));
					break;
				case VertexType_PosNorTex:
					((VertexBuffer<Vertex_PosNorTex>*)vertexBuffer)->Set(i, Vertex_PosNorTex(
						mesh->mVertices[i].x,
						mesh->mVertices[i].y,
						mesh->mVertices[i].z,
						mesh->mNormals[i].x,
						mesh->mNormals[i].y,
						mesh->mNormals[i].z,
						mesh->mTextureCoords[0][i].x,
						mesh->mTextureCoords[0][i].y
						));
					break;
				case VertexType_PosNorTanTex:
					((VertexBuffer<Vertex_PosNorTanTex>*)vertexBuffer)->Set(i, Vertex_PosNorTanTex(
						mesh->mVertices[i].x,
						mesh->mVertices[i].y,
						mesh->mVertices[i].z,
						mesh->mNormals[i].x,
						mesh->mNormals[i].y,
						mesh->mNormals[i].z,
						mesh->mTangents[i].x,
						mesh->mTangents[i].y,
						mesh->mTangents[i].z,
						mesh->mBitangents[i] * mesh->mTangents[i], // Dot product // Correct order?
						mesh->mTextureCoords[0][i].x,
						mesh->mTextureCoords[0][i].y
						));
					break;
				}
			}

			vertexBuffer->Transform(transformMatrix);

			unsigned * indices = new unsigned[mesh->mNumFaces * 3];

			for(unsigned t = 0; t < mesh->mNumFaces; ++t) 
			{
				const aiFace* face = &mesh->mFaces[t];

				for(unsigned i = 0; i < face->mNumIndices; i++) 
				{
					indices[(t * 3) + i] = face->mIndices[i];
				}
			}

			localMeshes.push_back(new LocalMesh(vertexBuffer, indices, mesh->mNumFaces * 3));
			meshMaterials.push_back(mesh->mMaterialIndex);
		}

	}

	for(unsigned n = 0; n < nd->mNumChildren; ++n) {
		TraverseNode(sc, nd->mChildren[n], transformMatrix);
	}
}

void AssimpLoader::Load()
{
	std::string sPath(path.begin(), path.end());

	assimpScene = assimpImporter.ReadFile(sPath, aiProcess_Triangulate | aiProcess_FlipUVs);
}

void AssimpLoader::Step()
{
	if(assimpScene && !resultModel)
	{
		if(!textureTicket)
		{
			ProcessMaterials(assimpScene);

			if(!textureTicket)
			{
				TraverseScene(assimpScene);
			}
		}
		else
		{
			if(assets->IsLoaded(textureTicket))
			{
				ProcessLoadedTextures();
				TraverseScene(assimpScene);
			}
		}
	}
}

float AssimpLoader::GetProgress()
{
	return progress;
}

bool AssimpLoader::IsFinished()
{
	return resultModel != 0;
}

IAsset * AssimpLoader::GetAsset()
{
	return resultModel;
}

} // end namespace Ingenuity