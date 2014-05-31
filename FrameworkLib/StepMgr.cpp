#include "StepMgr.h"

namespace Ingenuity {

const float StepMgr::STEPPABLE_FRAME_TIME = 0.012f; // 12ms

void StepMgr::Add(Steppable * steppable)
{
	if(numSteppables < MAX_STEPPABLES)
	{
		steppables[numSteppables] = steppable;
		numSteppables++;
	}
}

void StepMgr::Remove(unsigned index)
{
	if(index < numSteppables)
	{
		for(unsigned i = index; i < numSteppables && i < MAX_STEPPABLES - 1; ++i)
		{
			steppables[i] = steppables[i + 1];
		}
		numSteppables--;
	}
}

void StepMgr::Remove(Steppable * steppable)
{
	bool found = false;
	for(unsigned i = 0; i < numSteppables; ++i)
	{
		if(steppables[i] == steppable)
		{
			found = true;
		}
		if(found && i < MAX_STEPPABLES - 1)
		{
			steppables[i] = steppables[i + 1];
		}
	}
	if(found)
	{
		numSteppables--;
	}
}

bool StepMgr::IsUpdateRequired(float frameTimeElapsed)
{
	return numSteppables > 0 && (STEPPABLE_FRAME_TIME - frameTimeElapsed > 0.0f);
}

void StepMgr::Update()
{
	Steppable * steppable = steppables[currentIndex];
	if(steppable && !steppable->IsFinished())
	{
		steppable->Step();
		currentIndex++;
	}
	else
	{
		Remove(currentIndex);
	}
	if(currentIndex >= numSteppables) currentIndex = 0;
}

Steppable * StepMgr::Get(unsigned index)
{
	if(index >= numSteppables) return 0;
	return steppables[index];
}

} // namespace Ingenuity
