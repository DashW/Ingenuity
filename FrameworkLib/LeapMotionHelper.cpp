#include "LeapMotionHelper.h"

#include <Leap.h>
#include <glm/mat4x4.hpp>
#include <glm/gtx/euler_angles.hpp>
#include <glm/gtx/transform.hpp>

#define _X86_
#include <profileapi.h>
typedef long long unsigned64;

namespace Ingenuity {

class LeapMotionHelper::InternalListener : public Leap::Listener 
{
public:
	static const unsigned FINGERS_PER_HAND = 5;
	static const unsigned BONES_PER_FINGER = 4;
	static const unsigned BONES_PER_HAND = (FINGERS_PER_HAND * BONES_PER_FINGER) + 1;
	static const unsigned MAX_BONES = BONES_PER_HAND * 2;

	InternalListener() : frameDelta(0.0f), prevTimeStamp(0)
	{
		__int64 countsPerSec = 0;
		QueryPerformanceFrequency((LARGE_INTEGER*)&countsPerSec);
		secsPerCount = 1.0f / (float)countsPerSec;

		for(unsigned i = 0; i < MAX_BONES; i++)
		{
			boneVisibilities[i] = false;
			boneLengths[i] = 0.0f;
			boneWidths[i] = 0.0f;
		}
	}

	glm::mat4 ConstructMatrix(Leap::FloatArray& leapBasis, Leap::Vector& leapPosition, bool isLeft)
	{
		glm::mat4 boneMatrix;
		memcpy(&boneMatrix, leapBasis.m_array, sizeof(glm::mat4));

		glm::vec3 position;
		memcpy(&position, leapPosition.toFloatPointer(), sizeof(glm::vec3));

		boneMatrix[3] = glm::vec4(position, 1.0f);

		// http://stackoverflow.com/questions/1263072/changing-a-matrix-from-right-handed-to-left-handed-coordinate-system

		static glm::mat4 flipZ(
			1, 0, 0, 0,
			0, 1, 0, 0,
			0, 0, -1, 0,
			0, 0, 0, 1);

		boneMatrix = flipZ * boneMatrix;

		if(!isLeft)
		{
			boneMatrix = boneMatrix * flipZ;
		}

		// Ensure that the matrix handedness is correct
		glm::vec3 tmp = glm::cross(glm::vec3(boneMatrix[1]), glm::vec3(boneMatrix[2]));
		float val = glm::dot(tmp, glm::vec3(boneMatrix[0]));
		assert(glm::abs(val - 1.0f) < 1.0e-4f);

		return boneMatrix;
	}

	virtual void onInit(const Leap::Controller&) {}
	virtual void onConnect(const Leap::Controller&) {}
	virtual void onDisconnect(const Leap::Controller&) {}
	virtual void onExit(const Leap::Controller&) {}

	virtual void onFrame(const Leap::Controller& controller)
	{
		__int64 newTimeStamp = 0;
		QueryPerformanceCounter((LARGE_INTEGER*)&newTimeStamp);
		if(prevTimeStamp != 0)
		{
			frameDelta = float(newTimeStamp - prevTimeStamp) * secsPerCount;
		}
		prevTimeStamp = newTimeStamp;

		const Leap::Frame frame = controller.frame();
		Leap::HandList hands = frame.hands();

		memset(boneVisibilities, 0, sizeof(bool) * MAX_BONES);

		for(Leap::HandList::const_iterator hl = hands.begin(); hl != hands.end(); ++hl) 
		{
			const Leap::Hand & hand = *hl;
			const Leap::Vector normal = hand.palmNormal();
			const Leap::Vector direction = hand.direction();

			{
				glm::mat4 boneMatrix = ConstructMatrix(hand.basis().toArray4x4(), hand.palmPosition(), hand.isLeft());
				unsigned boneIndex = (hand.isLeft() ? 0 : BONES_PER_HAND) + BONES_PER_HAND - 1;

				boneVisibilities[boneIndex] = true;
				boneLengths[boneIndex] = 0.0f;
				boneWidths[boneIndex] = hand.palmWidth() * 0.5f;
				boneMatrices[boneIndex] = boneMatrix;
			}

			// Get the Arm bone
			//Leap::Arm arm = hand.arm();

			// Get fingers
			const Leap::FingerList fingers = hand.fingers();
			for(Leap::FingerList::const_iterator fl = fingers.begin(); fl != fingers.end(); ++fl) {
				const Leap::Finger & finger = *fl;

				// Get finger bones
				for(int b = 0; b < 4; ++b) {
					Leap::Bone::Type boneType = static_cast<Leap::Bone::Type>(b);
					Leap::Bone bone = finger.bone(boneType);
					glm::mat4 boneMatrix = ConstructMatrix(bone.basis().toArray4x4(), bone.center(), hand.isLeft());
					unsigned boneIndex = (hand.isLeft() ? 0 : BONES_PER_HAND) + (finger.type() * BONES_PER_FINGER) + b;

					boneVisibilities[boneIndex] = true;
					boneLengths[boneIndex] = bone.length();
					boneWidths[boneIndex] = bone.width();
					boneMatrices[boneIndex] = boneMatrix;
				}
			}
		}
	}

	virtual void onFocusGained(const Leap::Controller&) {}
	virtual void onFocusLost(const Leap::Controller&) {}
	virtual void onDeviceChange(const Leap::Controller&) {}
	virtual void onServiceConnect(const Leap::Controller&) {}
	virtual void onServiceDisconnect(const Leap::Controller&) {}

	bool boneVisibilities[MAX_BONES];
	float boneLengths[MAX_BONES];
	float boneWidths[MAX_BONES];
	glm::mat4 boneMatrices[MAX_BONES];
	float frameDelta;

	float secsPerCount;
	__int64 prevTimeStamp;
};

LeapMotionHelper::LeapMotionHelper() : uniformScale(1.0f)
{
	listener = new InternalListener();
	controller = new Leap::Controller(*listener);
	controller->setPolicyFlags(Leap::Controller::POLICY_IMAGES);
}

LeapMotionHelper::~LeapMotionHelper()
{
	controller->removeListener(*listener);
	delete listener;
	//delete controller;
}

float LeapMotionHelper::GetFrameDelta()
{
	return listener->frameDelta;
}

unsigned LeapMotionHelper::GetNumBones()
{
	return InternalListener::MAX_BONES;
}

bool LeapMotionHelper::IsBoneVisible(unsigned index)
{
	return listener->boneVisibilities[index];
}

float LeapMotionHelper::GetBoneLength(unsigned index) const
{
	return listener->boneLengths[index] * uniformScale;
}

float LeapMotionHelper::GetBoneRadius(unsigned index) const
{
	return listener->boneWidths[index] * 0.5f * uniformScale;
}

glm::mat4 LeapMotionHelper::GetBoneMatrix(unsigned index)
{
	glm::mat4 boneMatrix = listener->boneMatrices[index];
	boneMatrix[3].x *= uniformScale;
	boneMatrix[3].y *= uniformScale;
	boneMatrix[3].z *= uniformScale;
	boneMatrix[3].x += position.x;
	boneMatrix[3].y += position.y;
	boneMatrix[3].z += position.z;
	return transform * boneMatrix;
}

} // end namespace Ingenuity
