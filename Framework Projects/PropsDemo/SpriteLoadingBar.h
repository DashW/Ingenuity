#pragma once

#include <RealtimeApp.h>

using namespace Ingenuity;

class SpriteLoadingBar
{
	StepMgr * steppables;
	Gpu::Sprite * loadingSprite, *backgroundSprite;

	float maximum, progress;

public:
	SpriteLoadingBar(RealtimeApp * app) : 
		steppables(app->steppables), 
		loadingSprite(0),
		backgroundSprite(0),
		maximum(0.0f), 
		progress(0.0f)
	{
		loadingSprite = new Gpu::Sprite();
		loadingSprite->center.x = 100.0f;
		loadingSprite->center.y = 15.0f;
		loadingSprite->color.g = 0.0f;
		loadingSprite->color.b = 0.0f;

		backgroundSprite = new Gpu::Sprite(*loadingSprite);
		backgroundSprite->color.r = 0.0f;

		Begin();
	}
	virtual ~SpriteLoadingBar()
	{
		//delete loadingSprite->texture;
		delete loadingSprite;
		delete backgroundSprite;
	}

	void Begin()
	{
		maximum = 1.0f * (float) steppables->Count();
	}

	void SetTexture(Gpu::Texture * texture)
	{
		backgroundSprite->texture = texture;
		loadingSprite->texture = texture;
	}

	void Update()
	{
		if(maximum > 0.0f)
		{
			float remaining = maximum;

			for(unsigned i = 0; i < steppables->Count(); i++)
			{
				remaining -= (1.0f - steppables->Get(i)->GetProgress());
			}

			progress = remaining / maximum;

			loadingSprite->clipRect.right = progress;
		}
	}
	
	void BeDrawn(Gpu::Api * gpu)
	{
		if(loadingSprite->texture && progress < 1.0f)
		{
			backgroundSprite->BeDrawn(gpu);
			loadingSprite->BeDrawn(gpu);
		}
	}
};