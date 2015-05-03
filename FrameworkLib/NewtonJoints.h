#pragma once

#define _NEWTON_STATIC_LIB
#define _CUSTOM_JOINTS_STATIC_LIB
#include <CustomBallAndSocket.h>

#include <dMatrix.h>

class CustomBallAndSocketWithFriction : public CustomBallAndSocket
{
public:
	CustomBallAndSocketWithFriction(const dMatrix& pinAndPivotFrame, NewtonBody* const child, NewtonBody* const parent, dFloat dryFriction)
		:CustomBallAndSocket(pinAndPivotFrame, child, parent)
		, m_dryFriction(dryFriction)
	{}

	void SubmitConstraints(dFloat timestep, int threadIndex);

	dFloat m_dryFriction;
};

// Subclass of ball-and-socket joint with the ability to set limits *and* friction
class CustomLimitBallAndSocketWithFriction : public CustomBallAndSocket
{
public:
	CustomLimitBallAndSocketWithFriction(const dMatrix& pinAndPivotFrame, NewtonBody* const child, NewtonBody* const parent = NULL);
	CustomLimitBallAndSocketWithFriction(const dMatrix& childPinAndPivotFrame, NewtonBody* const child, const dMatrix& parentPinAndPivotFrame, NewtonBody* const parent);
	virtual ~CustomLimitBallAndSocketWithFriction();

	void SetConeAngle(dFloat angle);
	void SetTwistAngle(dFloat minAngle, dFloat maxAngle);
	void SetFriction(dFloat friction);

	dFloat GetConeAngle() const;
	void GetTwistAngle(dFloat& minAngle, dFloat& maxAngle) const;
	dFloat GetFriction() const;

protected:
	virtual void GetInfo(NewtonJointRecord* const info) const;
	virtual void SubmitConstraints(dFloat timestep, int threadIndex);


	dMatrix m_rotationOffset;
	dFloat m_coneAngle;
	dFloat m_minTwistAngle;
	dFloat m_maxTwistAngle;
	dFloat m_coneAngleCos;
	dFloat m_coneAngleSin;
	dFloat m_coneAngleHalfCos;
	dFloat m_coneAngleHalfSin;
	dFloat m_friction;
};
