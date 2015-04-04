#ifdef USE_LEAPMOTION_HELPER

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
	static const unsigned BONES_PER_HAND = (FINGERS_PER_HAND * BONES_PER_FINGER) + 2;
	static const unsigned MAX_BONES = BONES_PER_HAND * 2;
	static const unsigned MAX_FINGERS = FINGERS_PER_HAND * 2;
	static const unsigned VIS_BUFFER_FRAMES = 2;

	struct BoneData
	{
		unsigned visTimeout = 0;
		float length = 0.0f;
		float width = 0.0f;
		glm::mat4 matrix;
	};

	struct FingerData
	{
		unsigned visTimeout = 0;
		glm::vec3 position;
		glm::vec3 direction;
	};

	InternalListener() : frameDelta(0.0f), prevTimeStamp(0)
	{
		__int64 countsPerSec = 0;
		QueryPerformanceFrequency((LARGE_INTEGER*)&countsPerSec);
		secsPerCount = 1.0f / (float)countsPerSec;
	}

	glm::mat4 ConstructMatrix(Leap::FloatArray& leapBasis, Leap::Vector& leapPosition, bool isLeft)
	{
		glm::mat4 boneMatrix;
		memcpy(&boneMatrix, leapBasis.m_array, sizeof(glm::mat4));

		glm::vec3 position;
		memcpy(&position, leapPosition.toFloatPointer(), sizeof(glm::vec3));

		boneMatrix[3] = glm::vec4(position, 1.0f);

		// http://stackoverflow.com/questions/1263072/changing-a-matrix-from-right-handed-to-left-handed-coordinate-system
		// Rigid reflection: reflect * transform * reflect

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

		for(unsigned i = 0; i < MAX_BONES; ++i)
		{
			bones[i].visTimeout -= bones[i].visTimeout > 0 ? 1 : 0;
		}

		for(unsigned i = 0; i < MAX_FINGERS; ++i)
		{
			fingers[i].visTimeout -= fingers[i].visTimeout > 0 ? 1 : 0;
		}

		for(Leap::HandList::const_iterator hl = hands.begin(); hl != hands.end(); ++hl) 
		{
			const Leap::Hand & hand = *hl;
			const Leap::Vector normal = hand.palmNormal();
			const Leap::Vector direction = hand.direction();

			{
				glm::mat4 boneMatrix = ConstructMatrix(hand.basis().toArray4x4(), hand.palmPosition(), hand.isLeft());
				unsigned boneIndex = (hand.isLeft() ? 0 : BONES_PER_HAND) + BONES_PER_HAND - 2;

				bones[boneIndex].visTimeout = VIS_BUFFER_FRAMES;
				bones[boneIndex].length = 0.0f;
				bones[boneIndex].width = hand.palmWidth() * 0.5f;
				bones[boneIndex].matrix = boneMatrix;
			}

			// Get the Arm bone
			Leap::Arm arm = hand.arm();
			//const Leap::Vector direction = arm.direction();

			{
				glm::mat4 boneMatrix = ConstructMatrix(arm.basis().toArray4x4(), arm.center(), hand.isLeft());
				unsigned boneIndex = (hand.isLeft() ? 0 : BONES_PER_HAND) + BONES_PER_HAND - 1;

				bones[boneIndex].visTimeout = VIS_BUFFER_FRAMES;
				bones[boneIndex].length = arm.wristPosition().distanceTo(arm.elbowPosition());
				bones[boneIndex].width = arm.width() * 0.5f;
				bones[boneIndex].matrix = boneMatrix;
			}

			// Get fingers
			const Leap::FingerList fingerList = hand.fingers();
			for(Leap::FingerList::const_iterator fl = fingerList.begin(); fl != fingerList.end(); ++fl) {
				const Leap::Finger & finger = *fl;

				// Get finger bones
				for(int b = 0; b < 4; ++b) {
					Leap::Bone::Type boneType = static_cast<Leap::Bone::Type>(b);
					Leap::Bone bone = finger.bone(boneType);
					glm::mat4 boneMatrix = ConstructMatrix(bone.basis().toArray4x4(), bone.center(), hand.isLeft());
					unsigned boneIndex = (hand.isLeft() ? 0 : BONES_PER_HAND) + (finger.type() * BONES_PER_FINGER) + b;

					bones[boneIndex].visTimeout = VIS_BUFFER_FRAMES;
					bones[boneIndex].length = bone.length();
					bones[boneIndex].width = bone.width();
					bones[boneIndex].matrix = boneMatrix;
				}

				unsigned fingerIndex = (hand.isLeft() ? 0 : FINGERS_PER_HAND) + finger.type();
				fingers[fingerIndex].visTimeout = VIS_BUFFER_FRAMES;

				memcpy(&fingers[fingerIndex].position, &finger.tipPosition(), sizeof(glm::vec3));
				memcpy(&fingers[fingerIndex].direction, &finger.direction(), sizeof(glm::vec3));
			}
		}
	}

	virtual void onFocusGained(const Leap::Controller&) {}
	virtual void onFocusLost(const Leap::Controller&) {}
	virtual void onDeviceChange(const Leap::Controller&) {}
	virtual void onServiceConnect(const Leap::Controller&) {}
	virtual void onServiceDisconnect(const Leap::Controller&) {}

	BoneData bones[MAX_BONES];
	FingerData fingers[MAX_FINGERS];
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
	if(index >= InternalListener::MAX_BONES) return false;
	return listener->bones[index].visTimeout > 0;
}

bool LeapMotionHelper::IsFingerVisible(unsigned index)
{
	if(index >= InternalListener::MAX_FINGERS) return false;
	return listener->fingers[index].visTimeout > 0;
}

float LeapMotionHelper::GetBoneLength(unsigned index) const
{
	if(index >= InternalListener::MAX_BONES) return 0.0f;
	return listener->bones[index].length * uniformScale;
}

float LeapMotionHelper::GetBoneRadius(unsigned index) const
{
	if(index >= InternalListener::MAX_BONES) return 0.0f;
	return listener->bones[index].width * 0.5f * uniformScale;
}

glm::mat4 LeapMotionHelper::GetBoneMatrix(unsigned index)
{
	if(index >= InternalListener::MAX_BONES) return glm::mat4();
	glm::mat4 boneMatrix = listener->bones[index].matrix;
	boneMatrix[3].x *= uniformScale;
	boneMatrix[3].y *= uniformScale;
	boneMatrix[3].z *= uniformScale;
	boneMatrix[3].x += position.x;
	boneMatrix[3].y += position.y;
	boneMatrix[3].z += position.z;
	return transform * boneMatrix;
}

glm::vec3 LeapMotionHelper::GetFingerPosition(unsigned index)
{
	if(index >= InternalListener::MAX_FINGERS) return glm::vec3();
	glm::vec3 fingerPosition = listener->fingers[index].position;
	return (fingerPosition * uniformScale) + position;
}

glm::vec3 LeapMotionHelper::GetFingerDirection(unsigned index)
{
	if(index >= InternalListener::MAX_FINGERS) return glm::vec3();
	return listener->fingers[index].direction;
}

} // end namespace Ingenuity

#endif // USE_LEAPMOTION_HELPER
