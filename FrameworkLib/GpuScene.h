#pragma once

#include "GpuApi.h"
#include <vector>
#include <list>
#include <map>
#include <queue>
#include <functional>

namespace Ingenuity {
namespace Gpu {

class Scene : public Drawable
{
public:
	virtual ~Scene() {}

	virtual unsigned Add(Model * model) = 0;
	virtual unsigned Add(Sprite * sprite) = 0;
	virtual unsigned Add(Light * light) = 0;

	virtual void SetCamera(Camera * camera) = 0;

	virtual unsigned GetNumModels() = 0;
	virtual unsigned GetNumSprites() = 0;
	virtual unsigned GetNumLights() = 0;

	virtual void ClearModels() = 0;
	virtual void ClearSprites() = 0;
	virtual void ClearLights() = 0;

	virtual void SetDirty() {}
};

class LinearScene : public Scene
{
	std::list<Model*> models;
	std::vector<Sprite*> sprites;
	std::vector<Light*> lights;
	Camera * camera;

public:
	LinearScene()
		: camera(0)
	{
	}

	virtual unsigned Add(Model * model) override
	{
		models.push_back(model);
		return models.size() - 1;
	}
	virtual unsigned Add(Sprite * sprite) override
	{
		sprites.push_back(sprite);
		return sprites.size() - 1;
	}
	virtual unsigned Add(Light * light) override
	{
		lights.push_back(light);
		return lights.size() - 1;
	}

	Model* GetModel(unsigned index)
	{
		std::list<Model*>::iterator it;
		int i = 0;
		for(it = models.begin(); it != models.end(); it++)
		{
			if(i == index) return *it;
			i++;
		}
		return 0;
	}
	Sprite* GetSprite(unsigned index)
	{
		if(index >= sprites.size()) return 0;
		return sprites[index];
	}
	Light* GetLight(unsigned index)
	{
		if(index >= lights.size()) return 0;
		return lights[index];
	}

	virtual unsigned GetNumModels() override
	{
		return models.size();
	}
	virtual unsigned GetNumSprites() override
	{
		return sprites.size();
	}
	virtual unsigned GetNumLights() override
	{
		return lights.size();
	}

	virtual void SetCamera(Camera* cam) override
	{
		camera = cam;
	}

	virtual void ClearModels() override
	{
		models.clear();
	}
	virtual void ClearSprites() override
	{
		sprites.clear();
	}
	virtual void ClearLights() override
	{
		lights.clear();
	}

	virtual void BeDrawn(Api * gpu, DrawSurface * surface) override
	{
		// First models
		std::list<Model*>::iterator it;
		for(it = models.begin(); it != models.end(); it++)
		{
			gpu->DrawGpuModel(*it, camera, lights.data(), lights.size(), surface);
		}
		// Then sprites
		for(unsigned i = 0; i < sprites.size(); i++)
		{
			gpu->DrawGpuSprite(sprites[i], surface);
		}
	}
};

template <class INSTANCE_TYPE>
class InstancedScene : public Scene
{
	//struct SortByMesh : std::binary_function<GpuModel*,GpuModel*,bool>
	//{
	//	bool operator() (GpuModel*& x, GpuModel*& y) const {return x && y ? x->mesh < y->mesh : x < y;}
	//};
	//SortByMesh sortPred;

	std::vector<Sprite*> sprites;
	std::vector<Light*> lights;

	typedef std::map<Mesh*, Model> ModelMap;
	typedef std::map<Mesh*, std::vector<INSTANCE_TYPE>> VectorMap;
	typedef std::map<Mesh*, InstanceBuffer*> BufferMap;

	ModelMap instanceModels;
	VectorMap instanceVectors;
	BufferMap instanceBuffers;

	Api * gpu;
	Camera * camera;

	bool dirty;

public:
	InstancedScene(Api * gpu)
		: gpu(gpu)
		, camera(0)
		, dirty(false)
	{
	}
	virtual ~InstancedScene()
	{
		ClearModels();
		ClearSprites();
		ClearLights();

		BufferMap::iterator instanceBufferIt = instanceBuffers.begin();
		for(; instanceBufferIt != instanceBuffers.end(); ++instanceBufferIt)
		{
			delete instanceBufferIt->second;
		}
		instanceBuffers.clear();
	}

	virtual unsigned Add(Model * model) override
	{
		if(!model) return -1;

		Mesh * mesh = model->mesh;
		std::map<Mesh*, std::vector<INSTANCE_TYPE>>::iterator instanceBufferIt = instanceVectors.find(mesh);
		if(instanceBufferIt == instanceVectors.end())
		{
			instanceVectors[mesh] = std::vector<INSTANCE_TYPE>();

			Model tempModel = *model;
			tempModel.destructMesh = false;
			tempModel.position = glm::vec4(0.0f);
			instanceModels[mesh] = tempModel;
		}

		INSTANCE_TYPE instance;
		instance.UpdateFromModel(model);

		instanceVectors[mesh].push_back(instance);

		dirty = true;

		return instanceVectors[mesh].size() - 1;
	}
	virtual unsigned Add(Sprite * sprite) override
	{
		sprites.push_back(sprite);
		return sprites.size() - 1;
	}
	virtual unsigned Add(Light * light) override
	{
		lights.push_back(light);
		return lights.size() - 1;
	}

	Gpu::Model * GetModel(Mesh * mesh)
	{
		ModelMap::iterator instanceModelIt = instanceModels.find(mesh);
		if(instanceModelIt != instanceModels.end()) return &instanceModelIt->second;
		return 0;
	}
	INSTANCE_TYPE * GetInstance(Mesh * mesh, unsigned instanceNum)
	{
		VectorMap::iterator instanceVectorIt = instanceVectors.find(mesh);
		if(instanceVectorIt != instanceVectors.end())
		{
			std::vector<INSTANCE_TYPE> & instanceVector = instanceVectorIt->second;
			if(instanceNum < instanceVector.size()) return &instanceVector[instanceNum];
		}
		return 0;
	}
	Gpu::Sprite * GetSprite(unsigned index)
	{
		if(index >= sprites.size()) return 0;
		return sprites[index];
	}
	Gpu::Light * GetLight(unsigned index)
	{
		if(index >= lights.size()) return 0;
		return lights[index];
	}

	virtual unsigned GetNumModels() override
	{
		return instanceModels.size();
	}
	unsigned GetNumInstances(Gpu::Mesh * mesh)
	{
		VectorMap::iterator instanceVectorIt = instanceVectors.find(mesh);
		if(instanceVectorIt != instanceVectors.end())
		{
			return instanceVectorIt->second->size();
		}
		return 0;
	}
	virtual unsigned GetNumSprites() override
	{
		return sprites.size();
	}
	virtual unsigned GetNumLights() override
	{
		return lights.size();
	}

	virtual void SetCamera(Camera * cam) override
	{
		camera = cam;
	}

	virtual void ClearModels() override
	{
		instanceModels.clear();
		instanceVectors.clear();

		dirty = true;
	}
	virtual void ClearSprites() override
	{
		sprites.clear();
	}
	virtual void ClearLights() override
	{
		lights.clear();
	}

	virtual void SetDirty() override
	{
		dirty = true;
	}

	virtual void BeDrawn(Api * gpu, DrawSurface * surface) override
	{
		// First models
		VectorMap::iterator instanceVectorIt = instanceVectors.begin();
		for(; instanceVectorIt != instanceVectors.end(); ++instanceVectorIt)
		{
			Mesh * mesh = instanceVectorIt->first;
			std::vector<INSTANCE_TYPE> & instanceVector = instanceVectorIt->second;

			BufferMap::iterator instanceBufferIt = instanceBuffers.find(mesh);
			if(instanceBufferIt == instanceBuffers.end())
			{
				instanceBuffers[mesh] = gpu->CreateInstanceBuffer(instanceVector.size(), instanceVector.data(), INSTANCE_TYPE::type);
			}
			else if(dirty)
			{
				InstanceBuffer * buffer = instanceBufferIt->second;
				if(buffer->GetCapacity() < instanceVector.size())
				{
					delete buffer;
					instanceBufferIt->second = gpu->CreateInstanceBuffer(instanceVector.size(), instanceVector.data(), INSTANCE_TYPE::type);
				}
				else
				{
					gpu->UpdateInstanceBuffer(buffer, instanceVector.size(), instanceVector.data());
				}
				dirty = false;
			}

			Model * model = &instanceModels[mesh];
			InstanceBuffer * buffer = instanceBuffers[mesh];

			gpu->DrawGpuModel(model, camera, lights.data(), lights.size(), surface, buffer);
		}

		// Then sprites // Perhaps we can take advantage of, or emulate the D3DXSprite's batching support here?
		std::vector<Sprite*>::const_iterator spriteIt;
		for(spriteIt = sprites.begin(); spriteIt != sprites.end(); ++spriteIt)
		{
			gpu->DrawGpuSprite(*spriteIt, surface);
		}
	}
};

} // namespace Gpu
} // namespace Ingenuity
