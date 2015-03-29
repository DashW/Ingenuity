#pragma once

#include "../Third Party/glm-0.9.5.4/glm/glm.hpp"

namespace Leap
{
class Controller;
}

namespace Ingenuity {

class LeapMotionHelper
{
	class InternalListener;

public:
	LeapMotionHelper();
	virtual ~LeapMotionHelper();

	float GetFrameDelta();
	unsigned GetNumBones();
	bool IsBoneVisible(unsigned index);
	bool IsFingerVisible(unsigned index);
	float GetBoneLength(unsigned index) const;
	float GetBoneRadius(unsigned index) const;
	//glm::vec3 GetBonePosition(unsigned index);
	//glm::vec3 GetBoneRotation(unsigned index);
	glm::mat4 GetBoneMatrix(unsigned index);
	glm::vec3 GetFingerPosition(unsigned index);
	glm::vec3 GetFingerDirection(unsigned index);

	void SetPosition(glm::vec3 position) { this->position = position; }
	void SetUniformScale(float uniformScale) { this->uniformScale = uniformScale; }

private:
	Leap::Controller * controller;
	InternalListener * listener;
	glm::mat4 transform;
	glm::vec3 position;
	float uniformScale;
};

} // end namespace Ingenuity
