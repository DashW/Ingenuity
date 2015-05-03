#include "NewtonJoints.h"

#include <Newton.h>

#define MIN_JOINT_PIN_LENGTH	50.0f

#define DG_MAX_BOUND			( 1.0e15f )
#define DG_MIN_BOUND			( -DG_MAX_BOUND )

void CustomBallAndSocketWithFriction::SubmitConstraints(dFloat timestep, int threadIndex)
{
	CustomBallAndSocket::SubmitConstraints(timestep, threadIndex);

	dVector omega0(0.0f, 0.0f, 0.0f, 0.0f);
	dVector omega1(0.0f, 0.0f, 0.0f, 0.0f);

	// get the omega vector
	NewtonBodyGetOmega(m_body0, &omega0[0]);
	if(m_body1) {
		NewtonBodyGetOmega(m_body1, &omega1[0]);
	}

	dVector relOmega(omega0 - omega1);
	dFloat omegaMag = dSqrt(relOmega % relOmega);
	if(omegaMag > 0.1f) {
		// tell newton to used this the friction of the omega vector to apply the rolling friction
		dMatrix basis(dGrammSchmidt(relOmega));
		NewtonUserJointAddAngularRow(m_joint, 0.0f, &basis[2][0]);
		NewtonUserJointAddAngularRow(m_joint, 0.0f, &basis[1][0]);
		NewtonUserJointAddAngularRow(m_joint, 0.0f, &basis[0][0]);

		// calculate the acceleration to stop the ball in one time step
		dFloat invTimestep = (timestep > 0.0f) ? 1.0f / timestep : 1.0f;

		// override the desired acceleration, with the desired acceleration for full stop. 
		NewtonUserJointSetRowAcceleration(m_joint, -omegaMag * invTimestep);

		// set the friction limit proportional the sphere Inertia
		NewtonUserJointSetRowMinimumFriction(m_joint, -m_dryFriction);
		NewtonUserJointSetRowMaximumFriction(m_joint, m_dryFriction);
	}
	else {
		// when omega is too low this is correct but the small angle approximation theorem.
		dMatrix basis(dGetIdentityMatrix());
		for(int i = 0; i < 3; i++) {
			NewtonUserJointAddAngularRow(m_joint, 0.0f, &basis[i][0]);
			NewtonUserJointSetRowMinimumFriction(m_joint, -m_dryFriction);
			NewtonUserJointSetRowMaximumFriction(m_joint, m_dryFriction);
		}
	}
}

CustomLimitBallAndSocketWithFriction::CustomLimitBallAndSocketWithFriction(const dMatrix& pinAndPivotFrame, NewtonBody* const child, NewtonBody* const parent)
	:CustomBallAndSocket(pinAndPivotFrame, child, parent)
	, m_rotationOffset(dGetIdentityMatrix())
{
	SetConeAngle(0.0f);
	SetTwistAngle(0.0f, 0.0f);
	SetFriction(0.0f);
}


CustomLimitBallAndSocketWithFriction::CustomLimitBallAndSocketWithFriction(const dMatrix& childPinAndPivotFrame, NewtonBody* const child, const dMatrix& parentPinAndPivotFrame, NewtonBody* const parent)
	:CustomBallAndSocket(childPinAndPivotFrame, child, parent)
	, m_rotationOffset(childPinAndPivotFrame * parentPinAndPivotFrame.Inverse())
{
	SetConeAngle(0.0f);
	SetTwistAngle(0.0f, 0.0f);
	SetFriction(0.0f);

	dMatrix matrix;
	CalculateLocalMatrix(parentPinAndPivotFrame, matrix, m_localMatrix1);
}

CustomLimitBallAndSocketWithFriction::~CustomLimitBallAndSocketWithFriction()
{
}

void CustomLimitBallAndSocketWithFriction::SetConeAngle(dFloat angle)
{
	m_coneAngle = angle;
	m_coneAngleCos = dCos(angle);
	m_coneAngleSin = dSin(angle);
	m_coneAngleHalfCos = dCos(angle * 0.5f);
	m_coneAngleHalfSin = dSin(angle * 0.5f);
}


void CustomLimitBallAndSocketWithFriction::SetTwistAngle(dFloat minAngle, dFloat maxAngle)
{
	m_minTwistAngle = minAngle;
	m_maxTwistAngle = maxAngle;
}

void CustomLimitBallAndSocketWithFriction::SetFriction(dFloat friction)
{
	m_friction = friction;
}

dFloat CustomLimitBallAndSocketWithFriction::GetConeAngle() const
{
	return m_coneAngle;
}

void CustomLimitBallAndSocketWithFriction::GetTwistAngle(dFloat& minAngle, dFloat& maxAngle) const
{
	minAngle = m_minTwistAngle;
	maxAngle = m_maxTwistAngle;
}

dFloat CustomLimitBallAndSocketWithFriction::GetFriction() const
{
	return m_friction;
}

void CustomLimitBallAndSocketWithFriction::GetInfo(NewtonJointRecord* const info) const
{
	CustomBallAndSocket::GetInfo(info);

	info->m_minAngularDof[0] = m_minTwistAngle;
	info->m_maxAngularDof[0] = m_maxTwistAngle;

	info->m_minAngularDof[1] = -m_coneAngle;
	info->m_maxAngularDof[1] = m_coneAngle;
	info->m_minAngularDof[2] = -m_coneAngle;
	info->m_maxAngularDof[2] = m_coneAngle;

	strcpy(info->m_descriptionType, "limitballsocket");
}

void CustomLimitBallAndSocketWithFriction::SubmitConstraints(dFloat timestep, int threadIndex)
{
	dMatrix matrix0;
	dMatrix matrix1;

	// calculate the position of the pivot point and the Jacobian direction vectors, in global space. 
	CalculateGlobalMatrix(m_localMatrix0, m_localMatrix1, matrix0, matrix1);

	const dVector& p0 = matrix0.m_posit;
	const dVector& p1 = matrix1.m_posit;

	// So we create a position matrix...
	// and then we restrict MOVEMENT...
	// along all three orthonormal directions???

	// Restrict the movement on the pivot point along all tree orthonormal direction
	NewtonUserJointAddLinearRow(m_joint, &p0[0], &p1[0], &matrix1.m_front[0]);
	NewtonUserJointAddLinearRow(m_joint, &p0[0], &p1[0], &matrix1.m_up[0]);
	NewtonUserJointAddLinearRow(m_joint, &p0[0], &p1[0], &matrix1.m_right[0]);

	matrix1 = m_rotationOffset * matrix1;

	// This submits the twist limit constraints...

	dMatrix localMatrix(matrix0 * matrix1.Inverse());
	dFloat pitchAngle = -dAtan2(localMatrix[1][2], localMatrix[2][2]);

	bool twistConstrained = true;
	bool coneConstrained = true;

	if((m_maxTwistAngle - m_minTwistAngle) < 1.0e-4f) {

		dMatrix base(dPitchMatrix(pitchAngle) * matrix0);
		dVector q0(p1 + matrix0.m_up.Scale(MIN_JOINT_PIN_LENGTH));
		dVector q1(p1 + base.m_up.Scale(MIN_JOINT_PIN_LENGTH));

		NewtonUserJointAddLinearRow(m_joint, &q0[0], &q1[0], &base.m_right[0]);
	}
	else {
		if(pitchAngle > m_maxTwistAngle) {
			pitchAngle -= m_maxTwistAngle;
			dMatrix base(dPitchMatrix(pitchAngle) * matrix0);
			dVector q0(p1 + matrix0.m_up.Scale(MIN_JOINT_PIN_LENGTH));
			dVector q1(p1 + base.m_up.Scale(MIN_JOINT_PIN_LENGTH));
			NewtonUserJointAddLinearRow(m_joint, &q0[0], &q1[0], &base.m_right[0]);
			NewtonUserJointSetRowMinimumFriction(m_joint, -0.0f);
		}
		else if(pitchAngle < m_minTwistAngle) {
			pitchAngle -= m_minTwistAngle;
			dMatrix base(dPitchMatrix(pitchAngle) * matrix0);
			dVector q0(p1 + matrix0.m_up.Scale(MIN_JOINT_PIN_LENGTH));
			dVector q1(p1 + base.m_up.Scale(MIN_JOINT_PIN_LENGTH));
			NewtonUserJointAddLinearRow(m_joint, &q0[0], &q1[0], &base.m_right[0]);
			NewtonUserJointSetRowMaximumFriction(m_joint, 0.0f);
		}
		else
		{
			twistConstrained = false;
		}
	}

	// This submits the cone limit constraints...

	const dVector& coneDir0 = matrix0.m_front;
	const dVector& coneDir1 = matrix1.m_front;
	dVector r0(p0 + coneDir0.Scale(MIN_JOINT_PIN_LENGTH));
	dVector r1(p1 + coneDir1.Scale(MIN_JOINT_PIN_LENGTH));

	// construct an orthogonal coordinate system with these two vectors
	dVector lateralDir(coneDir0 * coneDir1);
	dFloat mag2;
	mag2 = lateralDir % lateralDir;
	if(dAbs(mag2) <  1.0e-4f) {
		if(m_coneAngleSin < 1.0e-4f) {
			NewtonUserJointAddLinearRow(m_joint, &r0[0], &r1[0], &matrix0.m_up[0]);
			NewtonUserJointAddLinearRow(m_joint, &r0[0], &r1[0], &matrix0.m_right[0]);
		}
	}
	else {
		dFloat cosAngle;
		cosAngle = coneDir0 % coneDir1;
		if(cosAngle < m_coneAngleCos) {
			lateralDir = lateralDir.Scale(1.0f / dSqrt(mag2));
			dQuaternion rot(m_coneAngleHalfCos, lateralDir.m_x * m_coneAngleHalfSin, lateralDir.m_y * m_coneAngleHalfSin, lateralDir.m_z * m_coneAngleHalfSin);
			r1 = p1 + rot.UnrotateVector(r1 - p1);

			NewtonUserJointAddLinearRow(m_joint, &r0[0], &r1[0], &lateralDir[0]);

			dVector longitudinalDir(lateralDir * matrix0.m_front);
			NewtonUserJointAddLinearRow(m_joint, &r0[0], &r1[0], &longitudinalDir[0]);
			NewtonUserJointSetRowMinimumFriction(m_joint, -0.0f);
		}
		else
		{
			coneConstrained = false;
		}
	}

	if(!twistConstrained && !coneConstrained && m_friction > 0.01f)
	{
		dVector omega0(0.0f, 0.0f, 0.0f, 0.0f);
		dVector omega1(0.0f, 0.0f, 0.0f, 0.0f);

		// get the omega vector
		NewtonBodyGetOmega(m_body0, &omega0[0]);
		if(m_body1) {
			NewtonBodyGetOmega(m_body1, &omega1[0]);
		}

		dVector relOmega(omega0 - omega1);
		dFloat omegaMag = dSqrt(relOmega % relOmega);
		if(omegaMag > 0.1f) {
			// tell newton to used this the friction of the omega vector to apply the rolling friction
			dMatrix basis(dGrammSchmidt(relOmega));
			for(int i = 0; i < 3; i++) {
				NewtonUserJointAddAngularRow(m_joint, 0.0f, &basis[i][0]);
				NewtonUserJointSetRowMinimumFriction(m_joint, -m_friction);
				NewtonUserJointSetRowMaximumFriction(m_joint, m_friction);
			}
		}
		else 
		{
			// when omega is too low this is correct but the small angle approximation theorem.
			dMatrix basis(dGetIdentityMatrix());
			for(int i = 0; i < 3; i++) {
				NewtonUserJointAddAngularRow(m_joint, 0.0f, &basis[i][0]);
				NewtonUserJointSetRowMinimumFriction(m_joint, -m_friction);
				NewtonUserJointSetRowMaximumFriction(m_joint, m_friction);
			}
		}
	}
}
