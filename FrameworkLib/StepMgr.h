#pragma once

namespace Ingenuity {

struct Steppable
{
	virtual void Step() = 0;
	virtual float GetProgress() = 0;
	virtual bool IsFinished() = 0;
};

//class DummySteppable : public Steppable
//{
//	virtual void Step() override {}
//	virtual float GetProgress() override { return 0.0f; }
//	virtual bool IsFinished() override { return false; }
//};

class StepMgr
{
	static const float STEPPABLE_FRAME_TIME;
	static const int MAX_STEPPABLES = 64;
	Steppable * steppables[MAX_STEPPABLES];
	unsigned numSteppables;
	unsigned currentIndex;

public:
	StepMgr() : numSteppables(0), currentIndex(0) {}

	void Add(Steppable * steppable);
	void Remove(unsigned index);
	void Remove(Steppable * steppable);
	bool IsUpdateRequired(float frameTimeElapsed);
	void Update();
	Steppable * Get(unsigned index);
	unsigned Count() { return numSteppables; }
};

} // namespace Ingenuity
