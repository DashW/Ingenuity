#ifdef USE_NEWTON_PHYSICSAPI

#include "NewtonApi.h"

#define _NEWTON_STATIC_LIB
#include <Newton.h>

#define _CUSTOM_JOINTS_STATIC_LIB
#include <CustomArcticulatedTransformManager.h>
#include <CustomBallAndSocket.h>
#include <CustomKinematicController.h>

#include <dVector.h>
#include <dMatrix.h>
#include <profileapi.h>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/quaternion.hpp>
#include <glm/gtx/euler_angles.hpp>
#include "GpuVertices.h"

#define MAX_PHYSICS_FPS 120.0f
#define MAX_PHYSICS_LOOPS 1

typedef long long unsigned64;

const float TICKS2SEC = 1.0e-6f;

static unsigned64 m_prevTime = 0;

#ifdef _MSC_VER
static LARGE_INTEGER frequency;
static LARGE_INTEGER baseCount;
#else 
static unsigned64 baseCount;
#endif

unsigned64 dGetTimeInMicroseconds()
{
#ifdef _MSC_VER
	LARGE_INTEGER count;
	QueryPerformanceCounter(&count);
	count.QuadPart -= baseCount.QuadPart;
	unsigned64 ticks = unsigned64(count.QuadPart * LONGLONG(1000000) / frequency.QuadPart);
	return ticks;

#endif

#if (defined (_POSIX_VER) || defined (_POSIX_VER_64))
	timespec ts;
	clock_gettime(CLOCK_REALTIME, &ts); // Works on Linux
	//return unsigned64 (ts.tv_nsec / 1000) - baseCount;

	return unsigned64(ts.tv_sec) * 1000000 + ts.tv_nsec / 1000 - baseCount;
#endif


#ifdef _MACOSX_VER
	timeval tp;
	gettimeofday(&tp, NULL);
	unsigned64 microsecunds = unsigned64(tp.tv_sec) * 1000000 + tp.tv_usec;
	return microsecunds - baseCount;
#endif
}

//////////////////////////////////////////////////////////////////////////////
// NEWTON EXAMPLE DATA - REMOVE WHEN CONFIDENT THAT EXTERNAL API IS WORKING //
//////////////////////////////////////////////////////////////////////////////

struct RAGDOLL_BONE_DEFINITION
{
	char m_boneName[32];
	char m_shapeType[32];
	int m_parent;

	dFloat m_shapePitch;
	dFloat m_shapeYaw;
	dFloat m_shapeRoll;

	dFloat m_shape_x;
	dFloat m_shape_y;
	dFloat m_shape_z;

	dFloat m_radius;
	dFloat m_height;
	dFloat m_mass;

	// JOINT PROPERTIES:

	dFloat m_coneAngle;
	dFloat m_minTwistAngle;
	dFloat m_maxTwistAngle;

	dFloat m_childPitch;
	dFloat m_childYaw;
	dFloat m_childRoll;

	dFloat m_parentPitch;
	dFloat m_parentYaw;
	dFloat m_parentRoll;

	// TRANSFORM:

	dFloat m_position_x;
	dFloat m_position_y;
	dFloat m_position_z;

	dFloat m_rotation_x;
	dFloat m_rotation_y;
	dFloat m_rotation_z;
};

static RAGDOLL_BONE_DEFINITION skeletonRagDoll[] =
{
	{ "Bip01_Pelvis", "capsule", -1,
	0.0f, 0.0f, -90.0f,
	0.0f, 0.0f, 0.01f,
	0.07f, 0.16f, 30.0f,
	0.0f, -0.0f, 0.0f,
	0.0f, 0.0f, 0.0f,
	0.0f, 0.0f, 0.0f,
	-0.02f, 0.89f, 0.0f,
	1.57f, 0.0f, 3.14f },
	// -- transform position="-0.020121 0.887429 0.000003 1.000000" eulerAngles="1.570796 -0.000001 3.141593 0.000000" localScale="1.000000 1.000000 1.000000 1.000000"

	{ "Bip01_Spine", "capsule", 0,
	0.0f, 0.0f, -90.0f,
	0.0f, 0.0f, 0.06f,
	0.07f, 0.14f, 20.0f,
	30.0f, -30.0f, 30.0f,
	0.0f, -90.0f, 0.0f,
	0.0f, -90.0f, 0.0f,
	0.0f, 0.0f, 0.1f,
	0.0f, 0.0f, 0.0f },
	// transform position="-0.000110 0.000000 0.091558 1.000000" eulerAngles="-0.000000 0.000796 0.000073 0.000000" localScale="1.000001 1.320000 1.000000 1.000000"

	{ "Bip01_Spine1", "capsule", 1,
	0.0f, 0.0f, -90.0f,
	0.0f, 0.0f, 0.06f,
	0.07f, 0.12f, 20.0f,
	30.0f, -30.0f, 30.0f,
	0.0f, -90.0f, 0.0f,
	0.0f, -90.0f, 0.0f,
	0.0f, 0.0f, 0.14f,
	0.0f, 0.0f, 0.0f },
	// transform position="-0.000110 -0.000000 0.138000 1.000000" eulerAngles="-0.000000 -0.000000 0.000000 0.000000" localScale="1.000000 1.000000 1.000000 1.000000"

	{ "Bip01_Spine2", "capsule", 2,
	0.0f, 0.0f, -90.0f,
	0.0f, 0.0f, 0.06f,
	0.07f, 0.08f, 20.0f,
	30.0f, -30.0f, 30.0f,
	0.0f, -90.0f, 0.0f,
	0.0f, -90.0f, 0.0f,
	0.0f, 0.0f, 0.14f,
	0.0f, 0.0f, 0.0f },
	// transform position="-0.000110 -0.000000 0.138000 1.000000" eulerAngles="-0.000000 -0.000000 -0.000000 0.000000" localScale="1.000000 1.000000 1.000000 1.000000"

	{ "Bip01_L_Thigh", "capsule", 0,
	0.0f, 90.0f, 0.0f,
	0.0f, 0.0f, 0.19f,
	0.05f, 0.34f, 10.0f,
	80.0f, -30.0f, 30.0f,
	0.0f, -90.0f, 0.0f,
	90.0f, -30.0f, -90.0f,
	0.0f, 0.1f, 0.0f,
	3.14f, -0.1f, 0.0f },
	// transform position="-0.000007 0.105093 0.000000 1.000000" eulerAngles="3.141591 -0.100000 0.000076 0.000000" localScale="1.000000 1.320000 1.000000 1.000000"

	{ "Bip01_L_Calf", "capsule", 4,
	0.0f, 90.0f, 0.0f,
	0.0f, 0.0f, 0.19f,
	0.05f, 0.34f, 5.0f,
	0.0f, -150.0f, 0.0f,
	0.0f, 0.0f, -90.0f,
	0.0f, 0.0f, -90.0f,
	0.0f, 0.0f, 0.4f,
	0.0f, -0.2f, 0.0f },
	// transform position="0.000000 0.000000 0.398078 1.000000" eulerAngles="-0.000000 -0.200000 -0.000000 0.000000" localScale="1.000000 1.000000 1.000000 1.000000"

	{ "Bip01_L_Foot", "capsule", 5,
	90.0f, 00.0f, 0.0f,
	0.05f, 0.00f, 0.05f,
	0.05f, 0.13f, 3.0f,
	0.0f, -45.0f, 45.0f,
	0.0f, 0.0f, -90.0f,
	0.0f, 0.0f, -90.0f,
	0.0f, 0.0f, 0.4f,
	0.0f, 0.1f, 0.0f },
	// transform position="-0.000000 -0.000000 0.398078 1.000000" eulerAngles="-0.000000 0.100000 -0.000000 0.000000" localScale="1.000000 1.000000 1.000000 1.000000"

	{ "Bip01_R_Thigh", "capsule", 0,
	0.0f, 90.0f, 0.0f,
	0.0f, 0.0f, 0.19f,
	0.05f, 0.34f, 10.0f,
	80.0f, -30.0f, 30.0f,
	0.0f, -90.0f, 0.0f,
	90.0f, -30.0f, 90.0f,
	0.0f, -0.1f, 0.0f,
	3.14f, -0.1f, 0.0f },
	// transform position="0.000007 -0.105093 0.000000 1.000000" eulerAngles="3.141591 -0.100000 0.000076 0.000000" localScale="1.000000 1.320000 1.000000 1.000000"

	{ "Bip01_R_Calf", "capsule", 7,
	0.0f, 90.0f, 0.0f,
	0.0f, 0.0f, 0.19f,
	0.05f, 0.34f, 5.0f,
	0.0f, 0.0f, 150.0f,
	0.0f, 0.0f, 90.0f,
	0.0f, 0.0f, 90.0f,
	0.0f, 0.0f, 0.4f,
	0.0f, -0.2f, 0.0f },
	// transform position="0.000000 0.000000 0.398078 1.000000" eulerAngles="-0.000000 -0.200000 -0.000000 0.000000" localScale="1.000000 1.000000 1.000000 1.000000"

	{ "Bip01_R_Foot", "capsule", 8,
	90.0f, 00.0f, 0.0f,
	0.05f, 0.00f, 0.05f,
	0.05f, 0.13f, 3.0f,
	0.0f, -45.0f, 45.0f,
	0.0f, 0.0f, 90.0f,
	0.0f, 0.0f, 90.0f,
	0.0f, 0.0f, 0.4f,
	0.0f, 0.1f, 0.0f },
	// transform position="-0.000000 -0.000000 0.398078 1.000000" eulerAngles="-0.000000 0.100000 -0.000000 0.000000" localScale="1.000000 1.000000 1.000000 1.000000"

	{ "Bip01_Neck", "capsule", 3,
	0.0f, 90.0f, 0.0f,
	0.0f, 0.0f, 0.05f,
	0.03f, 0.04f, 5.0f,
	30.0f, -30.0f, 30.0f,
	0.0f, -90.0f, 0.0f,
	0.0f, -90.0f, 0.0f,
	0.0f, 0.0f, 0.14f,
	0.0f, 0.0f, 0.0f },
	// transform position="-0.000048 -0.000000 0.138000 1.000000" eulerAngles="-0.000000 -0.000000 -0.000000 0.000000" localScale="1.000000 1.000000 1.000000 1.000000"

	{ "Bip01_Head", "sphere", 10,
	0.0f, 90.0f, 0.0f,
	0.0f, 0.0f, 0.09f,
	0.09f, 0.0f, 5.0f,
	30.0f, -60.0f, 60.0f,
	0.0f, -90.0f, 0.0f,
	0.0f, -90.0f, 0.0f,
	0.0f, 0.0f, 0.06f,
	0.0f, 0.0f, 0.0f },
	// transform position="0.000000 -0.000000 0.059712 1.000000" eulerAngles="0.000000 -0.000798 0.000000 0.000000" localScale="1.000000 1.000000 1.000000 1.000000"

	{ "Bip01_L_UpperArm", "capsule", 3,
	0.0f, 90.0f, 0.0f,
	0.0f, 0.0f, 0.12f,
	0.03f, 0.23f, 10.0f,
	80.0f, 30.0f, 30.0f,
	0.0f, -90.0f, 0.0f,
	90.0f, -30.0f, 90.0f,
	0.0f, 0.14f, 0.10f,
	3.04f, 0.0f, 3.24f },
	// transform position="0.000000 0.000000 0.103500 1.000000" eulerAngles="1.470797 -0.000003 0.099998 0.000000" localScale="1.000000 1.000000 1.000000 1.000000"
	// CLAVICLE:
	// transform position="-0.000001 0.027865 0.138000 1.000000" eulerAngles="1.570796 0.000796 3.141593 0.000000" localScale="1.000000 1.000000 1.000000 1.000000"

	{ "Bip01_L_Forearm", "capsule", 12,
	0.0f, 90.0f, 0.0f,
	0.0f, 0.0f, 0.12f,
	0.03f, 0.23f, 7.0f,
	0.0f, 0.0f, 150.0f,
	0.0f, 0.0f, 90.0f,
	0.0f, 0.0f, 90.0f,
	0.0f, 0.0f, 0.24f,
	0.0f, -0.2f, 0.0f },
	// transform position="-0.000000 0.000000 0.238847 1.000000" eulerAngles="-0.000001 -0.200000 0.000000 0.000000" localScale="1.000000 1.000000 1.000000 1.000000"

	{ "Bip01_L_Hand", "capsule", 13,
	0.0f, 90.0f, 0.0f,
	0.00f, 0.0f, 0.05f,
	0.05f, 0.05f, 2.0f,
	0.0f, -45.0f, 45.0f,
	0.0f, 0.0f, 90.0f,
	0.0f, 0.0f, 90.0f,
	0.0f, 0.0f, 0.24f,
	0.0f, 0.0f, -1.57f },
	// transform position="-0.000000 -0.000000 0.238847 1.000000" eulerAngles="-0.000000 -0.000000 -1.570000 0.000000" localScale="1.000000 1.000000 1.000000 1.000000"

	{ "Bip01_R_UpperArm", "capsule", 3,
	0.0f, 90.0f, 0.0f,
	0.0f, 0.0f, 0.12f,
	0.03f, 0.23f, 10.0f,
	80.0f, 30.0f, 30.0f,
	0.0f, -90.0f, 0.0f,
	90.0f, -30.0f, -90.0f,
	0.0f, -0.14f, 0.10f,
	-3.04f, 0.0f, -3.24f },
	// transform position="0.000000 -0.000000 0.103500 1.000000" eulerAngles="-1.470796 0.000001 -0.100000 0.000000" localScale="1.000000 1.000000 1.000000 1.000000"
	// CLAVICLE:
	// transform position = "-0.000000 -0.027865 0.138000 1.000000" eulerAngles = "-1.570796 0.000797 -3.141593 0.000000" localScale = "1.000000 1.000000 1.000000 1.000000"

	{ "Bip01_R_Forearm", "capsule", 15,
	0.0f, 90.0f, 0.0f,
	0.0f, 0.0f, 0.12f,
	0.03f, 0.23f, 7.0f,
	0.0f, -150.0f, 0.0f,
	0.0f, 0.0f, -90.0f,
	0.0f, 0.0f, -90.0f,
	0.0f, 0.0f, 0.24f,
	0.0f, -0.2f, 0.0f },
	// transform position="-0.000000 0.000000 0.238847 1.000000" eulerAngles="-0.000002 -0.200000 0.000000 0.000000" localScale="1.000000 1.000000 1.000000 1.000000"

	{ "Bip01_R_Hand", "capsule", 16,
	0.0f, 90.0f, 0.0f,
	0.00f, 0.0f, 0.05f,
	0.05f, 0.05f, 2.0f,
	0.0f, -45.0f, 45.0f,
	0.0f, 0.0f, -90.0f,
	0.0f, 0.0f, -90.0f,
	0.0f, 0.0f, 0.24f,
	0.0f, 0.0f, 1.57f },
	// transform position="-0.000000 0.000000 0.238846 1.000000" eulerAngles="-0.000000 -0.000000 1.570000 0.000000" localScale="1.000000 1.000000 1.000000 1.000000"
};

////////////////////////////////
// END OF NEWTON EXAMPLE DATA //
////////////////////////////////

class RagDollManager : public CustomArticulaledTransformManager
{
public:
	RagDollManager(NewtonWorld* const world) : CustomArticulaledTransformManager(world, true), controller(0)
	{
		// create a material for early collision culling
		numPhysicsObjects = 0;
		m_material = NewtonMaterialCreateGroupID(world);
		NewtonMaterialSetCollisionCallback(world, m_material, m_material, this, OnBoneAABBOverlap, NULL);
	}

	virtual void OnPreUpdate(CustomArticulatedTransformController* const constroller, dFloat timestep, int threadIndex) const
	{
	}

	static int OnBoneAABBOverlap(const NewtonMaterial* const material, const NewtonBody* const body0, const NewtonBody* const body1, int threadIndex)
	{
		NewtonCollision* const collision0 = NewtonBodyGetCollision(body0);
		NewtonCollision* const collision1 = NewtonBodyGetCollision(body1);
		CustomArticulatedTransformController::dSkeletonBone* const bone0 = (CustomArticulatedTransformController::dSkeletonBone*)NewtonCollisionGetUserData(collision0);
		CustomArticulatedTransformController::dSkeletonBone* const bone1 = (CustomArticulatedTransformController::dSkeletonBone*)NewtonCollisionGetUserData(collision1);

		dAssert(bone0);
		dAssert(bone1);
		if(bone0->m_myController && bone1->m_myController) {
			return bone0->m_myController->SelfCollisionTest(bone0, bone1) ? 1 : 0;
		}

		return 1;
	}

	//void GetDimentions(DemoEntity* const bodyPart, dVector& origin, dVector& size) const
	//{
	//	DemoMesh* const mesh = bodyPart->GetMesh();

	//	dFloat* const array = mesh->m_vertex;
	//	dVector pmin(1.0e20f, 1.0e20f, 1.0e20f, 0.0f);
	//	dVector pmax(-1.0e20f, -1.0e20f, -1.0e20f, 0.0f);

	//	for(int i = 0; i < mesh->m_vertexCount; i++) {
	//		dFloat x = array[i * 3 + 0];
	//		dFloat y = array[i * 3 + 1];
	//		dFloat z = array[i * 3 + 2];

	//		pmin.m_x = x < pmin.m_x ? x : pmin.m_x;
	//		pmin.m_y = y < pmin.m_y ? y : pmin.m_y;
	//		pmin.m_z = z < pmin.m_z ? z : pmin.m_z;

	//		pmax.m_x = x > pmax.m_x ? x : pmax.m_x;
	//		pmax.m_y = y > pmax.m_y ? y : pmax.m_y;
	//		pmax.m_z = z > pmax.m_z ? z : pmax.m_z;
	//	}

	//	size = (pmax - pmin).Scale(0.5f);
	//	origin = (pmax + pmin).Scale(0.5f);
	//	origin.m_w = 1.0f;
	//}


	NewtonCollision* MakeSphere(const RAGDOLL_BONE_DEFINITION& definition) const
	{
		dVector size;
		dVector origin;
		dMatrix matrix(dGetIdentityMatrix());

		matrix.m_posit.m_x = definition.m_shape_x;
		matrix.m_posit.m_y = definition.m_shape_y;
		matrix.m_posit.m_z = definition.m_shape_z;
		return NewtonCreateSphere(GetWorld(), definition.m_radius, 0, &matrix[0][0]);

	}

	NewtonCollision* MakeCapsule(const RAGDOLL_BONE_DEFINITION& definition) const
	{
		dVector size;
		dVector origin;
		dMatrix matrix(dPitchMatrix(definition.m_shapePitch * 3.141592f / 180.0f) * dYawMatrix(definition.m_shapeYaw * 3.141592f / 180.0f) * dRollMatrix(definition.m_shapeRoll * 3.141592f / 180.0f));

		matrix.m_posit.m_x = definition.m_shape_x;
		matrix.m_posit.m_y = definition.m_shape_y;
		matrix.m_posit.m_z = definition.m_shape_z;
		return NewtonCreateCapsule(GetWorld(), definition.m_radius, definition.m_height, 0, &matrix[0][0]);
	}

	//NewtonCollision* MakeBox() const
	//{
	//	dAssert(0);
	//	//		dVector size;
	//	//		dVector origin;
	//	//		dMatrix matrix (GetIdentityMatrix());
	//	//		GetDimentions(bone, matrix.m_posit, size);
	//	//		return NewtonCreateBox (nWorld, 2.0f * size.m_x, 2.0f * size.m_y, 2.0f * size.m_z, 0, &matrix[0][0]);
	//	return NULL;
	//}


	virtual void OnUpdateTransform(const CustomArticulatedTransformController::dSkeletonBone* const bone, const dMatrix& localMatrix) const
	{
		//DemoEntity* const ent = (DemoEntity*)NewtonBodyGetUserData(bone->m_body);
		//DemoEntityManager* const scene = (DemoEntityManager*)NewtonWorldGetUserData(NewtonBodyGetWorld(bone->m_body));

		//dQuaternion rot(localMatrix);
		//ent->SetMatrix(*scene, rot, localMatrix.m_posit);
	}

	//NewtonCollision* MakeConvexHull(Ingenuity::IVertexBuffer * vertexBuffer) const
	//{
	//	dVector points[1024 * 16];

	//	dAssert(vertexBuffer->GetLength() && (int(vertexBuffer->GetLength()) < int(sizeof(points) / sizeof(points[0]))));

	//	// go over the vertex array and find and collect all vertices's weighted by this bone.
	//	dFloat* const array = (float*)vertexBuffer->GetData();
	//	for(unsigned i = 0; i < vertexBuffer->GetLength(); i++) {
	//		unsigned offset = i * (vertexBuffer->GetElementSize() / sizeof(float));
	//		points[i].m_x = array[offset + 0];
	//		points[i].m_y = array[offset + 1];
	//		points[i].m_z = array[offset + 2];
	//	}

	//	return NewtonCreateConvexHull(GetWorld(), vertexBuffer->GetLength(), &points[0].m_x, sizeof(dVector), 1.0e-3f, 0, NULL);
	//}


	Ingenuity::NewtonPhysicsObject * CreateRagDollBodyPart(const RAGDOLL_BONE_DEFINITION* definitions, int index)
	{
		const RAGDOLL_BONE_DEFINITION& definition = definitions[index];
		NewtonCollision* shape = NULL;
		Ingenuity::NewtonPhysicsObject * physicsObject = 0;

		if(!strcmp(definition.m_shapeType, "sphere")) {
			Ingenuity::NewtonPhysicsSphereSpec * sphereSpec = new Ingenuity::NewtonPhysicsSphereSpec();
			sphereSpec->mass = definition.m_mass;
			sphereSpec->radius = definition.m_radius;
			physicsObject = new Ingenuity::NewtonPhysicsObject(sphereSpec);
			shape = MakeSphere(definition);
		}
		else if(!strcmp(definition.m_shapeType, "capsule")) {
			Ingenuity::NewtonPhysicsCapsuleSpec * capsuleSpec = new Ingenuity::NewtonPhysicsCapsuleSpec();
			capsuleSpec->mass = definition.m_mass;
			capsuleSpec->radius = definition.m_radius;
			capsuleSpec->length = definition.m_height;
			physicsObject = new Ingenuity::NewtonPhysicsObject(capsuleSpec);
			shape = MakeCapsule(definition);
		}
		else
		{
			return 0;
		}
		//else if(!strcmp(definition.m_shapeType, "box")) {
		//	shape = MakeBox();
		//}
		//else {
		//	shape = MakeConvexHull(vertexBuffer);
		//}

		// calculate the bone matrix
		glm::mat4 matrix = CalculateBoneMatrix(definitions, index);

		NewtonWorld* const world = GetWorld();

		// create the rigid body that will make this bone
		NewtonBody* const bone = NewtonCreateDynamicBody(world, shape, &matrix[0][0]);

		// calculate the moment of inertia and the relative center of mass of the solid
		NewtonBodySetMassProperties(bone, definition.m_mass, shape);

		// save the user data with the bone body (usually the visual geometry)
		//NewtonBodySetUserData(bone, bodyPart);

		// assign the material for early collision culling
		NewtonBodySetMaterialGroupID(bone, m_material);

		// set the bod part force and torque call back to the gravity force, skip the transform callback
		NewtonBodySetForceAndTorqueCallback(bone, Ingenuity::NewtonApi::ApplyForceAndTorqueCallback);

		// destroy the collision helper shape 
		NewtonDestroyCollision(shape);

		physicsObject->newtonBody = bone;

		return physicsObject;
	}


	void ConnectBodyParts(NewtonBody* const bone, NewtonBody* const parent, glm::vec3 jointInfo, glm::vec3 childRot, glm::vec3 parentRot) const
	{
		dMatrix matrix;
		NewtonBodyGetMatrix(bone, &matrix[0][0]);

		dMatrix parentPinAndPivotInGlobalSpace(dPitchMatrix(parentRot.x) * dYawMatrix(parentRot.y) * dRollMatrix(parentRot.z));
		parentPinAndPivotInGlobalSpace = parentPinAndPivotInGlobalSpace * matrix;

		dMatrix childPinAndPivotInGlobalSpace(dPitchMatrix(childRot.x) * dYawMatrix(childRot.y) * dRollMatrix(childRot.z));
		childPinAndPivotInGlobalSpace = childPinAndPivotInGlobalSpace * matrix;

		CustomLimitBallAndSocket* const joint = new CustomLimitBallAndSocket(childPinAndPivotInGlobalSpace, bone, parentPinAndPivotInGlobalSpace, parent);

		joint->SetConeAngle(jointInfo.x);
		joint->SetTwistAngle(jointInfo.y, jointInfo.z);
	}

	glm::mat4 CalculateBoneMatrix(const RAGDOLL_BONE_DEFINITION* const definitions, int index)
	{
		glm::mat4 matrix =
			glm::eulerAngleZ(definitions[index].m_rotation_z) *
			glm::eulerAngleY(definitions[index].m_rotation_y) *
			glm::eulerAngleX(definitions[index].m_rotation_x);

		matrix[3] = glm::vec4(
			definitions[index].m_position_x,
			definitions[index].m_position_y,
			definitions[index].m_position_z, 1.0f);

		int parentIndex = definitions[index].m_parent;
		glm::mat4 parentMatrix(1.0f);
		if(parentIndex > -1)
		{
			parentMatrix = CalculateBoneMatrix(definitions, parentIndex);
		}

		return parentMatrix * matrix;
	}

	void CreateRagDoll()
	{
		NewtonWorld* const world = GetWorld();
		//DemoEntityManager* const scene = (DemoEntityManager*)NewtonWorldGetUserData(world);

		// make a clone of the mesh 
		//DemoEntity* const ragDollEntity = (DemoEntity*)model->CreateClone();
		//scene->Append(ragDollEntity);

		// build the ragdoll with rigid bodies connected by joints
		// create a transform controller
		controller = CreateTransformController(0, false);

		//controller->GetUserData

		if(numPhysicsObjects > 0)
		{
			for(unsigned i = 0; i < numPhysicsObjects; ++i)
			{
				delete physicsObjects[i];
			}
		}

		//numPhysicsObjects = (unsigned)definitionCount;
	}

	void CreateDemoRagDoll(RAGDOLL_BONE_DEFINITION* const definitions, int definitionCount)
	{
		// add the root bone
		physicsObjects[0] = CreateRagDollBodyPart(definitions, 0);

		NewtonBody * rootBone = physicsObjects[0]->newtonBody;

		CustomArticulatedTransformController::dSkeletonBone* const skelBone = controller->AddBone(rootBone, dGetIdentityMatrix());
		// save the controller as the collision user data, for collision culling
		NewtonCollisionSetUserData(NewtonBodyGetCollision(rootBone), skelBone);

		int stackIndex = 0;
		skelBones[0] = skelBone;

		// walk model hierarchic adding all children designed as rigid body bones. 
		for(int j = 1; j < definitionCount; j++) {
			stackIndex--;
			//DemoEntity* const entity = childEntities[stackIndex];

			int parentIndex = definitions[j].m_parent;

			physicsObjects[j] = CreateRagDollBodyPart(definitions, j);

			NewtonBody * bone = physicsObjects[j]->newtonBody;

			CustomArticulatedTransformController::dSkeletonBone* parentBone = skelBones[parentIndex];

			// connect this body part to its parent with a ragdoll joint
			glm::vec3 joint(definitions[j].m_coneAngle * 3.141592f / 180.0f, definitions[j].m_minTwistAngle * 3.141592f / 180.0f, definitions[j].m_maxTwistAngle * 3.141592f / 180.0f);
			glm::vec3 childRot(definitions[j].m_childPitch * 3.141592f / 180.0f, definitions[j].m_childYaw * 3.141592f / 180.0f, definitions[j].m_childRoll * 3.141592f / 180.0f);
			glm::vec3 parentRot(definitions[j].m_parentPitch * 3.141592f / 180.0f, definitions[j].m_parentYaw * 3.141592f / 180.0f, definitions[j].m_parentRoll * 3.141592f / 180.0f);
			ConnectBodyParts(bone, parentBone->m_body, joint, childRot, parentRot);

			// This is used to add extra waypoint matrices for bones that might not be directly parented (e.g. clavicle)
			glm::mat4 bindMatrix(1.0f);
			parentBone = controller->AddBone(bone, (dMatrix&) bindMatrix, parentBone);

			// save the controller as the collision user data, for collision culling
			NewtonCollisionSetUserData(NewtonBodyGetCollision(bone), parentBone);

			skelBones[j] = parentBone;
		}

		numPhysicsObjects = definitionCount;
	}

	void FinalizeRagDoll(const dMatrix& location)
	{
		// set the collision mask
		// note this container work best with a material call back for setting bit field 
		controller->SetDefaultSelfCollisionMask();

		NewtonBody * rootBone = physicsObjects[0]->newtonBody;
		dMatrix rootMatrix;
		NewtonBodyGetMatrix(rootBone, (float*)&rootMatrix);

		// transform the entire contraction to its location
		dMatrix worldMatrix(rootMatrix * location); // FIXME!!!
		NewtonBodySetMatrixRecursive(rootBone, &worldMatrix[0][0]);
	}

	int m_material;

	CustomArticulatedTransformController* controller;
	CustomArticulatedTransformController::dSkeletonBone* skelBones[32];
	Ingenuity::NewtonPhysicsObject * physicsObjects[32];
	unsigned numPhysicsObjects;
};

namespace Ingenuity {

NewtonPhysicsWorld::~NewtonPhysicsWorld()
{
	if(newtonWorld)
	{
		NewtonDestroy(newtonWorld);
	}
}

NewtonPhysicsSpring::~NewtonPhysicsSpring()
{
	NewtonPhysicsObject * physicsObject1 = (NewtonPhysicsObject*)NewtonBodyGetUserData(body1);
	NewtonPhysicsObject * physicsObject2 = (NewtonPhysicsObject*)NewtonBodyGetUserData(body2);

	for(unsigned i = 0; physicsObject1 != 0 && i < physicsObject1->springs.size(); ++i)
	{
		if(physicsObject1->springs[i] == this)
		{
			physicsObject1->springs.erase(physicsObject1->springs.begin() + i);
			break;
		}
	}
	for(unsigned i = 0; physicsObject2 != 0 && i < physicsObject2->springs.size(); ++i)
	{
		if(physicsObject2->springs[i] == this)
		{
			physicsObject2->springs.erase(physicsObject2->springs.begin() + i);
			break;
		}
	}
}

NewtonPhysicsObject::~NewtonPhysicsObject()
{
	RemoveFromWorld();
	if(spec) delete spec;
}

NewtonPhysicsRagdoll::~NewtonPhysicsRagdoll()
{
	//if(manager)
	//{
	//	if(manager->numPhysicsObjects > 0)
	//	{
	//		for(unsigned i = 0; i < manager->numPhysicsObjects; ++i)
	//		{
	//			delete manager->physicsObjects[i];
	//		}
	//	}
	//	//delete manager;
	//}
}

void NewtonPhysicsObject::RemoveFromWorld()
{
	if(newtonBody)
	{
		NewtonDestroyBody(newtonBody);
	}
	newtonBody = 0;
}

NewtonApi::NewtonApi() :
physicsTime(0.0f),
microseconds(0),
reentrantUpdate(false)
//m_pickedBodyParam(0.0f)
{
	QueryPerformanceFrequency(&frequency);
}

NewtonApi::~NewtonApi()
{
	//std::map<PhysicsMaterial*, PhysicsMaterial*>::iterator cachedMaterialIt = cachedMaterials.begin();
	//for(; cachedMaterialIt != cachedMaterials.end(); ++cachedMaterialIt)
	//{
	//	delete cachedMaterialIt->second;
	//}
}

//void NewtonApi::SetTransformCallback(const NewtonBody * const body, const float * const matrix, int threadIndex)
//{
//	NewtonPhysicsObject * physicsObject = (NewtonPhysicsObject*)NewtonBodyGetUserData(body);
//
//	glm::vec3 position(matrix[12], matrix[13], matrix[14]);
//	glm::mat4x4 rotationMat4;
//
//	NewtonBodyGetRotation(body, (float*)&rotationMat4[0]);
//	rotationMat4 = glm::transpose(rotationMat4);
//
//	glm::quat rotation = glm::quat_cast(rotationMat4);
//
//	physicsObject->prevPosition = physicsObject->curPosition;
//	physicsObject->prevRotation = physicsObject->curRotation;
//	if(glm::dot(physicsObject->curRotation, rotation) < 0.0f) {
//		physicsObject->prevRotation *= -1.0f;
//	}
//
//	physicsObject->curPosition = position;
//	physicsObject->curRotation = rotation;
//}


void NewtonApi::ApplyForceAndTorqueCallback(const NewtonBody* body, float timestep, int threadIndex)
{
	float Ixx;
	float Iyy;
	float Izz;
	float mass;

	NewtonBodyGetMassMatrix(body, &mass, &Ixx, &Iyy, &Izz);

#define GRAVITY -10.0f

	glm::vec4 gravityForce(0.0f, mass * GRAVITY, 0.0f, 1.0f);
	NewtonBodySetForce(body, &gravityForce[0]);

	// If we really want to do this properly, the following should be in a CustomJoint.
	NewtonPhysicsObject * physicsObject = (NewtonPhysicsObject*)NewtonBodyGetUserData(body);
	if(physicsObject)
	{
		for(unsigned i = 0; i < physicsObject->springs.size(); ++i)
		{
			NewtonPhysicsSpring * spring = physicsObject->springs[i];

			glm::vec4 position1(spring->attachPoint1, 1.0f);
			if(spring->body1)
			{
				glm::mat4 body1matrix;
				NewtonBodyGetMatrix(spring->body1, &body1matrix[0][0]);
				position1 = body1matrix * glm::vec4(spring->attachPoint1, 1.0f);
			}
			glm::vec4 position2(spring->attachPoint2, 1.0f);
			if(spring->body2)
			{
				glm::mat4 body2matrix;
				NewtonBodyGetMatrix(spring->body2, &body2matrix[0][0]);
				position2 = body2matrix * glm::vec4(spring->attachPoint2, 1.0f);
			}

			glm::vec4 localPosition = (body == spring->body1 ? position1 : position2);
			glm::vec4 otherPosition = (body == spring->body1 ? position2 : position1);

			glm::vec4 displacement = otherPosition - localPosition;
			glm::vec4 offsetDisplacement = displacement - (spring->length * glm::normalize(displacement));
			float mag2 = glm::dot(displacement, displacement);
			if(mag2 > dFloat(20 * 20)) {
				displacement = displacement * (20.0f / sqrtf(mag2));
			}

			// add the mouse pick penalty force and torque
			glm::vec4 com;
			glm::mat4 matrix;
			NewtonBodyGetMatrix(body, &matrix[0][0]);
			NewtonBodyGetCentreOfMass(body, &com[0]);

			if(NewtonBodyGetType(body) != NEWTON_KINEMATIC_BODY)
			{
				// we pick a dynamics body, update by applying forces
				dFloat mass;
				dFloat Ixx;
				dFloat Iyy;
				dFloat Izz;

				glm::vec4 veloc;
				glm::vec4 omega;

				NewtonBodyGetMassMatrix(body, &mass, &Ixx, &Iyy, &Izz);
				NewtonBodyGetVelocity(body, &veloc[0]);
				NewtonBodyGetOmega(body, &omega[0]);

				if((spring->extension && glm::length(displacement) > spring->length) ||
					(spring->compression && glm::length(displacement) < spring->length))
				{
					glm::vec4 force(offsetDisplacement * mass * spring->stiffness);
					glm::vec4 forceDamping(veloc * mass * spring->damping);
					force -= forceDamping;

					//matrix[3] = glm::vec4(0.0f, 0.0f, 0.0f, matrix[3][3]);

					// calculate local point relative to center of mass
					glm::vec3 localAttachPoint = (body == spring->body1 ? spring->attachPoint1 : spring->attachPoint2);
					glm::vec4 point(matrix * (glm::vec4(localAttachPoint,0.0f) - com));
					//glm::vec4 point(glm::inverse(glm::transpose(matrix)) * (m_pickedBodyLocalAtachmentPoint - com));
					glm::vec4 torque(glm::cross(glm::vec3(point),glm::vec3(force)), 1.0f);
					//glm::vec4 torqueDamping(omega * mass * 0.5f);
					//torque -= torqueDamping;

					NewtonBodyAddForce(body, &force.x);
					NewtonBodyAddTorque(body, &torque.x);

					// make sure the body is unfrozen, if it is picked
					//NewtonBodySetFreezeState(body, 0);
				}
			}
		}
	}
}

int NewtonApi::AABBOverlapCallback(const NewtonMaterial* const material, const NewtonBody* const body0, const NewtonBody* const body1, int threadIndex)
{
	const NewtonPhysicsObject * const physicsObject0 = static_cast<NewtonPhysicsObject*>(NewtonBodyGetUserData(body0));
	const NewtonPhysicsObject * const physicsObject1 = static_cast<NewtonPhysicsObject*>(NewtonBodyGetUserData(body1));
	const int material0 = physicsObject0->spec->materialIndex;
	const int material1 = physicsObject1->spec->materialIndex;

	glm::vec3 scale0;
	glm::vec3 scale1;
	const NewtonCollision * const collision0 = NewtonBodyGetCollision(body0);
	const NewtonCollision * const collision1 = NewtonBodyGetCollision(body1);
	NewtonCollisionGetScale(collision0, &scale0.x, &scale0.y, &scale0.z);
	NewtonCollisionGetScale(collision1, &scale1.x, &scale1.y, &scale1.z);

	NewtonApi * const newtonApi = static_cast<NewtonApi*>(NewtonMaterialGetUserData(
		NewtonBodyGetWorld(body0),
		NewtonBodyGetMaterialGroupID(body0),
		NewtonBodyGetMaterialGroupID(body1)));

	bool contactOverridden = false;

	if(physicsObject0->spec->materialIndex > -1 && physicsObject1->spec->materialIndex > -1)
	{
		MaterialPairBank::iterator it = newtonApi->materialPairBank.find(PairKey(material0, material1));
		if(it != newtonApi->materialPairBank.end())
		{
			PhysicsMaterial::Properties & props = it->second;

			// HACK TO (SORT OF) FIX SCALED COLLISIONS
			NewtonMaterialSetContactSoftness(material, (props.softness * scale0.x * scale1.x));
			NewtonMaterialSetContactElasticity(material, props.elasticity);
			NewtonMaterialSetContactFrictionCoef(material, props.staticFriction, props.kineticFriction, 0);
			NewtonMaterialSetContactFrictionCoef(material, props.staticFriction, props.kineticFriction, 1);

			contactOverridden = true;
		}
	}

	if(!contactOverridden)
	{
		PhysicsMaterial::Properties props0;
		PhysicsMaterial::Properties props1;

		if(material0 > -1) props0 = newtonApi->materialBank[material0];
		if(material1 > -1) props1 = newtonApi->materialBank[material1];

		// HACK TO (SORT OF) FIX SCALED COLLISIONS
		NewtonMaterialSetContactSoftness(material, (props0.softness * props1.softness * scale0.x * scale1.x));
		NewtonMaterialSetContactElasticity(material, props0.elasticity * props1.elasticity);
		NewtonMaterialSetContactFrictionCoef(material,
			props0.staticFriction * props1.staticFriction,
			props0.kineticFriction * props1.kineticFriction, 0);
		NewtonMaterialSetContactFrictionCoef(material,
			props0.staticFriction * props1.staticFriction,
			props0.kineticFriction * props1.kineticFriction, 1);
	}

	return 1;
}

//int NewtonApi::CompoundAABBOverlapCallback(const NewtonMaterial* const material, const NewtonBody* const body0, const void* const collsionNode0, const NewtonBody* const body1, const void* const collsionNode1, int threadIndex)
//{
//	return 1;
//}

void NewtonApi::ContactCollisionCallback(const NewtonJoint* contactJoint, float timestep, int threadIndex)
{

}

void NewtonApi::DebugPolygonCallback(void* userData, int vertexCount, const float* faceVertec, int id)
{
	//DEBUG_DRAW_MODE mode = (DEBUG_DRAW_MODE) ((int)userData); //NOTE error: cast from ‘void*’ to ‘int’ loses precision
	//DEBUG_DRAW_MODE mode = (DEBUG_DRAW_MODE)((intptr_t)userData);

	std::vector<Vertex_PosNor> * verts = (std::vector<Vertex_PosNor>*) userData;

	glm::vec3 p0(faceVertec[0 * 3 + 0], faceVertec[0 * 3 + 1], faceVertec[0 * 3 + 2]);
	glm::vec3 p1(faceVertec[1 * 3 + 0], faceVertec[1 * 3 + 1], faceVertec[1 * 3 + 2]);
	glm::vec3 p2(faceVertec[2 * 3 + 0], faceVertec[2 * 3 + 1], faceVertec[2 * 3 + 2]);

	glm::vec3 nor((p1 - p0) * (p2 - p0));
	nor = glm::normalize(nor);

	for(int i = 2; i < vertexCount; i++) {
		glm::vec3 p2(faceVertec[i * 3 + 0], faceVertec[i * 3 + 1], faceVertec[i * 3 + 2]);
		verts->push_back(Vertex_PosNor(p0, nor));
		verts->push_back(Vertex_PosNor(p1, nor));
		verts->push_back(Vertex_PosNor(p2, nor));
		p1 = p2;
	}
}

// Here's the real issue. We need to make sure this code only gets executed when a spring
// is attached to this body. Okay, so how do we know where a spring is attached relative to a parent body,
// and how do we update that every frame? The most 'correct' API would be to introduce a raft of new
// spring specific functions: CreateSpring, AttachSpring, DetachSpring, SetSpringStiffness, SetSpringDamping
// But this is way too complex, and we'd like any new functions to apply to any kind of "joint".

// Attach a spring between point 1, which is relative to a Body, and point 2, which is relative to the world.
// Every frame, update the position of point 2. 
// My idea at one point was to declare a new kind of NewtonBody, a POINT, which could be moved, rotated, just
// like any other NewtonBody, but never as part of the simulation. I tried creating this body with a NullCollision.
// With the understanding that physics bodies cannot be parented to one-another, I have created the Anchor, purely
// as a means to attach joints to a static point of reference which can be moved manually.

PhysicsObject * NewtonApi::PickObject(PhysicsWorld * world, glm::vec3 origin, glm::vec3 dir, float & tOut, glm::vec3 & posOut, glm::vec3 & normalOut)
{
	class dMousePickClass
	{
	public:
		dMousePickClass() : m_param(1.0f), m_body(NULL) {}

		// implement a ray cast pre-filter
		static unsigned RayCastPrefilter(const NewtonBody* body, const NewtonCollision* const collision, void* const userData)
		{
			// ray cannot pick trigger volumes
			//return NewtonCollisionIsTriggerVolume(collision) ? 0 : 1;

			const NewtonCollision* const parent = NewtonCollisionGetParentInstance(collision);
			if(parent) {
				// you can use this to filter sub collision shapes.  
				dAssert(NewtonCollisionGetSubCollisionHandle(collision));
			}

			return (NewtonBodyGetType(body) == NEWTON_DYNAMIC_BODY) ? 1 : 0;
		}

		static dFloat RayCastFilter(const NewtonBody* const body, const NewtonCollision* const collisionHit, const dFloat* const contact, const dFloat* const normal, dLong collisionID, void* const userData, dFloat intersectParam)
		{
			// check if we are hitting a sub shape
			const NewtonCollision* const parent = NewtonCollisionGetParentInstance(collisionHit);
			if(parent) {
				// you can use this to filter sub collision shapes.  
				dAssert(NewtonCollisionGetSubCollisionHandle(collisionHit));
			}

			dMousePickClass* const data = (dMousePickClass*)userData;
			data->m_body = body;

			if(intersectParam < data->m_param) {
				data->m_param = intersectParam;
				data->m_normal = glm::vec3(normal[0], normal[1], normal[2]);
			}
			return intersectParam;
		}

		glm::vec3 m_normal;
		dFloat m_param;
		const NewtonBody* m_body;
	};

	NewtonPhysicsWorld * physicsWorld = static_cast<NewtonPhysicsWorld*>(world);

	glm::vec3 target = origin + dir;

	dMousePickClass rayCast;
	NewtonWorldRayCast(physicsWorld->newtonWorld, &origin[0], &target[0], dMousePickClass::RayCastFilter, &rayCast, dMousePickClass::RayCastPrefilter, 0);

	NewtonPhysicsObject * physicsObject = 0;

	if(rayCast.m_body) {
		physicsObject = (NewtonPhysicsObject*)NewtonBodyGetUserData(rayCast.m_body);
		posOut = origin + ( dir * (rayCast.m_param) );
		normalOut = rayCast.m_normal;
		tOut = rayCast.m_param;
	}

	return physicsObject;
}

//int NewtonApi::GetNewtonMaterialID(NewtonWorld * newtonWorld, PhysicsMaterial * material)
//{
//	int newtonMaterialID = -1;
//	std::map<PhysicsMaterial*, int>::iterator it = newtonMaterialIDs.find(material);
//
//	if(it != newtonMaterialIDs.end())
//		newtonMaterialID = it->second;
//	else
//		newtonMaterialID = NewtonMaterialCreateGroupID(newtonWorld);
//
//	NewtonMaterialSetDefaultElasticity(newtonWorld, newtonMaterialID, newtonMaterialID, material->elasticity);
//	NewtonMaterialSetDefaultFriction(newtonWorld, newtonMaterialID, newtonMaterialID, material->staticFriction, material->kineticFriction);
//	NewtonMaterialSetDefaultSoftness(newtonWorld, newtonMaterialID, newtonMaterialID, material->softness);
//	return newtonMaterialID;
//}

void NewtonApi::ApplyMassMatrix(NewtonPhysicsObject * physicsObject)
{
	NewtonBody * body = physicsObject->newtonBody;
	NewtonCollision * collision = NewtonBodyGetCollision(body);
	float mass = physicsObject->spec->mass;

	glm::vec4 inertia;
	glm::vec4 origin;

	NewtonConvexCollisionCalculateInertialMatrix(collision, &inertia[0], &origin[0]);

	// set the body mass matrix
	NewtonBodySetMassMatrix(body, mass, mass * inertia.x, mass * inertia.y, mass * inertia.z);

	// set the body origin
	NewtonBodySetCentreOfMass(body, &origin[0]);

	//NewtonBodySetMassProperties(body, physicsObject->spec->mass, collision);
}

PhysicsWorld * NewtonApi::CreateWorld(float scale, glm::vec3 gravity)
{
	NewtonPhysicsWorld * physicsWorld = new NewtonPhysicsWorld(NewtonCreate());

	NewtonSetSolverModel(physicsWorld->newtonWorld, 1);

	NewtonInvalidateCache(physicsWorld->newtonWorld);

	microseconds = dGetTimeInMicroseconds();

	int defaultMaterialId = NewtonMaterialGetDefaultGroupID(physicsWorld->newtonWorld);
	NewtonMaterialSetCollisionCallback(physicsWorld->newtonWorld, defaultMaterialId, defaultMaterialId, this, AABBOverlapCallback, ContactCollisionCallback);
	//NewtonMaterialSetCompoundCollisionCallback(physicsWorld->newtonWorld, defaultMaterialId, defaultMaterialId, CompoundAABBOverlapCallback);

	return physicsWorld;
}

PhysicsMaterial * NewtonApi::CreateMaterial(PhysicsMaterial::Properties & properties)
{
	materialBank.push_back(properties);
	return new NewtonPhysicsMaterial(materialBank.size() - 1);
}

void NewtonApi::SetMaterial(PhysicsMaterial * material, PhysicsMaterial::Properties & properties)
{
	if(!material) return;
	NewtonPhysicsMaterial * physicsMaterial = static_cast<NewtonPhysicsMaterial*>(material);
	if(physicsMaterial->index > -1 && physicsMaterial->index < (int)materialBank.size())
	{
		materialBank[physicsMaterial->index] = properties;
	}
}

void NewtonApi::OverrideMaterialPair(PhysicsMaterial * mat1, PhysicsMaterial * mat2, PhysicsMaterial::Properties & properties)
{
	if(!mat1 || !mat2) return;
	NewtonPhysicsMaterial * physicsMat1 = static_cast<NewtonPhysicsMaterial*>(mat1);
	NewtonPhysicsMaterial * physicsMat2 = static_cast<NewtonPhysicsMaterial*>(mat2);
	int index1 = physicsMat1->index;
	int index2 = physicsMat2->index;

	materialPairBank[PairKey(index1, index2)] = properties;
	materialPairBank[PairKey(index2, index1)] = properties;
}

PhysicsObject * NewtonApi::CreateAnchor()
{
	NewtonPhysicsAnchorSpec * spec = new NewtonPhysicsAnchorSpec();
	spec->mass = 0.0f;
	return new NewtonPhysicsObject(spec);
}

PhysicsObject * NewtonApi::CreateCuboid(glm::vec3 size, bool kinematic)
{
	NewtonPhysicsCuboidSpec * spec = new NewtonPhysicsCuboidSpec();
	spec->dimensions = size;
	spec->kinematic = kinematic;
	return new NewtonPhysicsObject(spec);
}

PhysicsObject * NewtonApi::CreateSphere(float radius, bool kinematic)
{
	NewtonPhysicsSphereSpec * spec = new NewtonPhysicsSphereSpec();
	spec->radius = radius;
	spec->kinematic = kinematic;
	return new NewtonPhysicsObject(spec);
}

PhysicsObject * NewtonApi::CreateCapsule(float radius, float length, bool kinematic)
{
	NewtonPhysicsCapsuleSpec * spec = new NewtonPhysicsCapsuleSpec();
	spec->radius = radius;
	spec->length = length;
	spec->kinematic = kinematic;
	return new NewtonPhysicsObject(spec);
}

PhysicsObject * NewtonApi::CreateMesh(LocalMesh * mesh, bool kinematic, bool deleteLocal)
{
	NewtonPhysicsMeshSpec * spec = new NewtonPhysicsMeshSpec();
	spec->mesh = mesh;
	spec->kinematic = kinematic;
	spec->deleteLocal = deleteLocal;
	return new NewtonPhysicsObject(spec);
}

PhysicsObject * NewtonApi::CreateHeightmap(HeightParser * parser)
{
	NewtonPhysicsHeightmapSpec * spec = new NewtonPhysicsHeightmapSpec();
	spec->heightParser = parser;
	spec->kinematic = false;
	return new NewtonPhysicsObject(spec);
}

PhysicsRagdoll * NewtonApi::CreateRagdoll(PhysicsWorld * world)
{
	NewtonPhysicsWorld * physicsWorld = static_cast<NewtonPhysicsWorld*>(world);

	RagDollManager* const manager = new RagDollManager(physicsWorld->newtonWorld);

	manager->CreateRagDoll();

	NewtonPhysicsRagdoll * physicsRagdoll = new NewtonPhysicsRagdoll();
	physicsRagdoll->manager = manager;

	return physicsRagdoll;
}

void NewtonApi::AddToWorld(PhysicsWorld * world, PhysicsObject * object, bool isStatic)
{
	NewtonPhysicsWorld * physicsWorld = static_cast<NewtonPhysicsWorld*>(world);
	NewtonPhysicsObject * physicsObject = static_cast<NewtonPhysicsObject*>(object);

	if(!physicsObject->spec || !physicsWorld->newtonWorld) return;

	NewtonInvalidateCache(physicsWorld->newtonWorld);

	NewtonCollision * collision = 0;

	switch(physicsObject->spec->GetType())
	{
	case NewtonPhysicsSpec::Anchor:
	{
		collision = NewtonCreateNull(physicsWorld->newtonWorld);
		break;
	}
	case NewtonPhysicsSpec::Cuboid:
	{
		NewtonPhysicsCuboidSpec * cubeSpec = static_cast<NewtonPhysicsCuboidSpec*>(physicsObject->spec);
		collision = NewtonCreateBox(physicsWorld->newtonWorld,
			cubeSpec->dimensions.x,
			cubeSpec->dimensions.y,
			cubeSpec->dimensions.z,
			0, 0);
		break;
	}
	case NewtonPhysicsSpec::Sphere:
	{
		NewtonPhysicsSphereSpec * sphereSpec = static_cast<NewtonPhysicsSphereSpec*>(physicsObject->spec);
		collision = NewtonCreateSphere(physicsWorld->newtonWorld, sphereSpec->radius, 0, 0);
		break;
	}
	case NewtonPhysicsSpec::Capsule:
	{
		NewtonPhysicsCapsuleSpec * capsuleSpec = static_cast<NewtonPhysicsCapsuleSpec*>(physicsObject->spec);
		collision = NewtonCreateCapsule(physicsWorld->newtonWorld, capsuleSpec->radius, capsuleSpec->length, 0, 0);
		break;
	}
	case NewtonPhysicsSpec::Mesh:
	{
		NewtonPhysicsMeshSpec * meshSpec = static_cast<NewtonPhysicsMeshSpec*>(physicsObject->spec);
		//LocalMesh * localMesh = meshSpec->mesh;
		//NewtonMesh * newtonMesh = NewtonMeshCreate(physicsWorld->newtonWorld);

		//NewtonMeshBeginFace(newtonMesh);

		//for(unsigned i = 0; i < localMesh->numTriangles; ++i)
		//{
		//	glm::vec3 positions[3];
		//	for(unsigned j = 0; j < 3; j++)
		//	{
		//		unsigned index = localMesh->indexBuffer[(i * 3) + j];
		//		unsigned byteOffset = (localMesh->vertexBuffer->GetElementSize() * index);
		//		char * byteBuffer = (char*) localMesh->vertexBuffer->GetData();
		//		positions[j] = *((glm::vec3*)(&byteBuffer[byteOffset]));
		//	}
		//	//glm::vec3 temp = positions[2];
		//	//positions[2] = positions[1];
		//	//positions[1] = temp;

		//	NewtonMeshAddFace(newtonMesh, 3, (float*)&positions, sizeof(glm::vec3), 0);
		//}

		//NewtonMeshEndFace(newtonMesh);

		glm::mat4 offsetMatrix;
		IVertexBuffer * vertexBuffer = meshSpec->mesh->vertexBuffer;

		//for(unsigned i = 0; i < debugVertexBuffer->GetLength(); ++i)
		//{
		//	debugVertexBuffer->Get(i).position *= 0.02;
		//}

		collision = NewtonCreateConvexHull(
			physicsWorld->newtonWorld,
			vertexBuffer->GetLength(),
			(float*)vertexBuffer->GetData(),
			vertexBuffer->GetElementSize(),
			0.0f, 0, (float*)&offsetMatrix);

		//NewtonMeshFixTJoints(newtonMesh);

		//NewtonMeshCalculateVertexNormals(newtonMesh, 0.0f);

		//NewtonMesh* const convexApproximation = NewtonMeshApproximateConvexDecomposition(newtonMesh, 0.01f, 0.2f, 32, 100, NULL);

		//NewtonMeshSaveOFF(convexApproximation, "debugConvex");

		//collision = NewtonCreateTreeCollisionFromMesh(physicsWorld->newtonWorld, newtonMesh, 0);

		//NewtonMesh * firstSegment = NewtonMeshCreateFirstSingleSegment(newtonMesh);

		//collision = NewtonCreateCompoundCollisionFromMesh(physicsWorld->newtonWorld, newtonMesh, 0.001f, 0, 0);

		//collision = NewtonCreateConvexHullFromMesh(physicsWorld->newtonWorld, firstSegment, 0.001f, 0);

		//NewtonMesh * convexHullMesh = NewtonMeshCreateFromCollision(collision);

		//NewtonMeshSaveOFF(convexHullMesh, "debugHull");

		//NewtonMeshDestroy(convexApproximation);
		//NewtonMeshDestroy(newtonMesh);

		//NewtonCreateConvexHull(physicsWorld->newtonWorld, localMesh->vertexBuffer->GetLength(), )

		//NewtonCreateConvexHullFromMesh()

		break;
	}
	case NewtonPhysicsSpec::Heightmap:
	{
		// Not allowed to create dynamic heightmap bodies, no internal volume
		if(!isStatic) return;

		NewtonPhysicsHeightmapSpec * heightSpec = static_cast<NewtonPhysicsHeightmapSpec*>(physicsObject->spec);
		HeightParser * heightParser = heightSpec->heightParser;
		int sideLength = (int)heightParser->GetSideLength();
		char* const attributes = new char[sideLength * sideLength];
		memset(attributes, 0, sideLength * sideLength * sizeof(char));

		collision = NewtonCreateHeightFieldCollision(physicsWorld->newtonWorld,
			sideLength, sideLength, 1, 0, heightParser->GetData(),
			attributes, heightParser->GetScale(), heightParser->GetWidth() / (float)(sideLength - 1), 0);

		glm::mat4 offset = glm::translate(glm::vec3(heightParser->GetWidth() / -2.0f, 0.0f, heightParser->GetWidth() / -2.0f));
		NewtonCollisionSetMatrix(collision, (float*)&offset);

		delete[] attributes;
		break;
	}
	}

	NewtonBody * body = 0;

	glm::mat4 matrix;

	//if(physicsObject->spec->kinematic)
	//{
	//	body = NewtonCreateKinematicBody(physicsWorld->newtonWorld, collision, (float*)&matrix);
	//}
	//else
	//{
		body = NewtonCreateDynamicBody(physicsWorld->newtonWorld, collision, (float*)&matrix);
	//}

	//PhysicsMaterial * material = cachedMaterials[physicsObject->spec->materialKey];
	//if(material)
	//{
	//	NewtonBodySetMaterialGroupID(body, GetNewtonMaterialID(physicsWorld->newtonWorld, material));
	//}

	physicsObject->newtonBody = body;

	if(!isStatic)
	{
		ApplyMassMatrix(physicsObject);
	}

	NewtonBodySetUserData(body, (void*)physicsObject);
	//NewtonBodySetTransformCallback(body, NewtonApi::SetTransformCallback);
	NewtonBodySetForceAndTorqueCallback(body, NewtonApi::ApplyForceAndTorqueCallback);

	NewtonDestroyCollision(collision);
}

void NewtonApi::RemoveFromWorld(PhysicsWorld * world, PhysicsObject * object)
{
	if(!object) return;

	NewtonPhysicsObject * physicsObject = static_cast<NewtonPhysicsObject*>(object);
	physicsObject->RemoveFromWorld();
}

void NewtonApi::UpdateWorld(PhysicsWorld * world, float deltaTime)
{
	// Based on DemoEntityManager::UpdatePhysics() line 677

	NewtonPhysicsWorld * physicsWorld = static_cast<NewtonPhysicsWorld*>(world);

	// read the controls 
	// update the physics
	if(physicsWorld->newtonWorld) {

		dFloat timestepInSeconds = 1.0f / MAX_PHYSICS_FPS;
		unsigned64 timestepMicroseconds = long long(timestepInSeconds * 1000000.0f);

		unsigned64 currentTime = dGetTimeInMicroseconds();
		unsigned64 nextTime = currentTime - microseconds;
		int loops = 0;

		while((nextTime >= timestepMicroseconds) && (loops < MAX_PHYSICS_LOOPS)) {
			loops++;

			// run the newton update function
			if(!reentrantUpdate) {
				reentrantUpdate = true;

				NewtonUpdate(physicsWorld->newtonWorld, timestepInSeconds);

				reentrantUpdate = false;
			}

			nextTime -= timestepMicroseconds;
			microseconds += timestepMicroseconds;
		}

		if(loops) {
			physicsTime = dFloat(dGetTimeInMicroseconds() - currentTime) / 1000000.0f;

			if(physicsTime >= MAX_PHYSICS_LOOPS * (1.0f / MAX_PHYSICS_FPS)) {
				microseconds = currentTime;
			}
		}
	}
}

void NewtonApi::AddRagdollBone(PhysicsRagdoll * ragdoll, PhysicsObject * object, int parentIndex, glm::vec3 joint, glm::vec3 childRot, glm::vec3 parentRot)
{
	NewtonPhysicsRagdoll * physicsRagdoll = static_cast<NewtonPhysicsRagdoll*>(ragdoll);
	NewtonPhysicsObject * physicsObject = static_cast<NewtonPhysicsObject*>(object);
	RagDollManager * manager = physicsRagdoll->manager;

	NewtonBody * body = physicsObject->newtonBody;
	CustomArticulatedTransformController::dSkeletonBone * parentBone = 0;

	if(parentIndex >= (int)manager->numPhysicsObjects) return;

	if(parentIndex > -1)
	{
		NewtonBody * parentBody = manager->physicsObjects[parentIndex]->newtonBody;
		parentBone = manager->skelBones[parentIndex];

		glm::mat4 bodyMatrix;
		glm::mat4 parentMatrix;
		NewtonBodyGetMatrix(body, (float*)&bodyMatrix);
		NewtonBodyGetMatrix(parentBody, (float*)&parentMatrix);

		bodyMatrix = parentMatrix * bodyMatrix;

		NewtonBodySetMatrix(body, (float*)&bodyMatrix);

		// connect this body part to its parent with a ragdoll joint
		manager->ConnectBodyParts(body, parentBody, joint, childRot, parentRot);
	}

	// This is used to add extra waypoint matrices for bones that might not be directly parented (e.g. clavicle)
	glm::mat4 bindMatrix(1.0f);
	CustomArticulatedTransformController::dSkeletonBone* skelBone = manager->controller->AddBone(body, (dMatrix&)bindMatrix, parentBone);

	// assign the material for early collision culling
	NewtonBodySetMaterialGroupID(body, manager->m_material);

	// save the controller as the collision user data, for collision culling
	NewtonCollisionSetUserData(NewtonBodyGetCollision(body), skelBone);

	manager->skelBones[manager->numPhysicsObjects] = skelBone;
	manager->physicsObjects[manager->numPhysicsObjects] = physicsObject;
	manager->numPhysicsObjects++;
}

void NewtonApi::FinalizeRagdoll(PhysicsRagdoll * ragdoll)
{
	NewtonPhysicsRagdoll * physicsRagdoll = static_cast<NewtonPhysicsRagdoll*>(ragdoll);

	if(physicsRagdoll->manager->numPhysicsObjects < 1)
	{
		physicsRagdoll->manager->CreateDemoRagDoll(skeletonRagDoll, sizeof(skeletonRagDoll) / sizeof(skeletonRagDoll[0]));
	}

	glm::mat4 matrix;
	physicsRagdoll->manager->FinalizeRagDoll((float*)&matrix);
}

PhysicsSpring * NewtonApi::CreateSpring(PhysicsObject * body1, PhysicsObject * body2, glm::vec3 attachPoint1, glm::vec3 attachPoint2)
{
	NewtonPhysicsObject * physicsObject1 = static_cast<NewtonPhysicsObject*>(body1);
	NewtonPhysicsObject * physicsObject2 = static_cast<NewtonPhysicsObject*>(body2);

	if(!physicsObject1 && !physicsObject2) return 0;

	NewtonPhysicsSpring * physicsSpring = new NewtonPhysicsSpring();
	physicsSpring->body1 = physicsObject1 ? physicsObject1->newtonBody : 0;
	physicsSpring->body2 = physicsObject2 ? physicsObject2->newtonBody : 0;
	physicsSpring->attachPoint1 = attachPoint1;
	physicsSpring->attachPoint2 = attachPoint2;

	if(physicsObject1)
	{
		physicsObject1->springs.push_back(physicsSpring);
	}
	if(physicsObject2)
	{
		physicsObject2->springs.push_back(physicsSpring);
	}

	return physicsSpring;
}

glm::vec3 NewtonApi::GetPosition(PhysicsObject * object)
{
	NewtonPhysicsObject * physicsObject = static_cast<NewtonPhysicsObject*>(object);

	if(!physicsObject->newtonBody) return glm::vec3();

	glm::mat4 matrix;
	NewtonBodyGetMatrix(physicsObject->newtonBody, (float*)&matrix);

	return glm::vec3(matrix[3]);
}

//glm::vec3 NewtonApi::GetRotation(PhysicsObject * object)
//{
//	NewtonPhysicsObject * physicsObject = static_cast<NewtonPhysicsObject*>(object);
//
//	if(!physicsObject->newtonBody) return glm::vec3();
//
//	glm::mat4 matrix;
//	NewtonBodyGetMatrix(physicsObject->newtonBody, (float*)&matrix);
//
//	glm::vec3 eulerAngles;
//	glm::vec3 eulerAngles2;
//	NewtonGetEulerAngle((float*)&matrix, (float*)&eulerAngles, (float*)&eulerAngles2);
//
//	return eulerAngles;
//}

glm::mat4 NewtonApi::GetMatrix(PhysicsObject * object)
{
	NewtonPhysicsObject * physicsObject = static_cast<NewtonPhysicsObject*>(object);

	if(!physicsObject->newtonBody) return glm::mat4();

	glm::mat4 matrix;
	NewtonBodyGetMatrix(physicsObject->newtonBody, (float*)&matrix);

	glm::mat4 localMatrix;
	NewtonCollisionGetMatrix(NewtonBodyGetCollision(physicsObject->newtonBody), (float*)&localMatrix);

	// HACK to orientate capsule collisions consistently with their graphics and Leap Motion counterparts
	glm::mat4 correctionMatrix;
	if(physicsObject->spec->GetType() == NewtonPhysicsSpec::Capsule)
	{
		correctionMatrix = glm::eulerAngleY(float(M_PI / 2.0));
	}

	return matrix * localMatrix * correctionMatrix;
}

PhysicsObject * NewtonApi::GetRagdollObject(PhysicsRagdoll * ragdoll, unsigned index)
{
	NewtonPhysicsRagdoll * physicsRagdoll = static_cast<NewtonPhysicsRagdoll*>(ragdoll);

	if(physicsRagdoll->manager && physicsRagdoll->manager->numPhysicsObjects > index)
	{
		return physicsRagdoll->manager->physicsObjects[index];
	}
	return 0;
}

void NewtonApi::SetLocalPosition(PhysicsObject * object, glm::vec3 position)
{
	NewtonPhysicsObject * physicsObject = static_cast<NewtonPhysicsObject*>(object);
	if(!physicsObject->newtonBody) return;

	dMatrix matrix;
	NewtonCollisionGetMatrix(NewtonBodyGetCollision(physicsObject->newtonBody), &matrix[0][0]);
	matrix.m_posit.m_x = position.x;
	matrix.m_posit.m_y = position.y;
	matrix.m_posit.m_z = position.z;

	NewtonCollisionSetMatrix(NewtonBodyGetCollision(physicsObject->newtonBody), (float*)&matrix);
}

void NewtonApi::SetLocalRotation(PhysicsObject * object, glm::vec3 rotation)
{
	NewtonPhysicsObject * physicsObject = static_cast<NewtonPhysicsObject*>(object);
	if(!physicsObject->newtonBody) return;

	dMatrix rotMatrix(dPitchMatrix(rotation.x) * dYawMatrix(rotation.y) * dRollMatrix(rotation.z));

	dMatrix matrix;
	NewtonCollisionGetMatrix(NewtonBodyGetCollision(physicsObject->newtonBody), &matrix[0][0]);

	rotMatrix.m_posit = matrix.m_posit;

	NewtonCollisionSetMatrix(NewtonBodyGetCollision(physicsObject->newtonBody), (float*)&rotMatrix);
}

void NewtonApi::SetPosition(PhysicsObject * object, glm::vec3 position)
{
	NewtonPhysicsObject * physicsObject = static_cast<NewtonPhysicsObject*>(object);

	if(!physicsObject->newtonBody) return;

	glm::mat4 matrix;
	NewtonBodyGetMatrix(physicsObject->newtonBody, (float*)&matrix);

	matrix[3] = glm::vec4(position, 1.0f);

	NewtonBodySetMatrix(physicsObject->newtonBody, (float*)&matrix);
}

void NewtonApi::SetRotation(PhysicsObject * object, glm::vec3 rotation)
{
	NewtonPhysicsObject * physicsObject = static_cast<NewtonPhysicsObject*>(object);

	if(!physicsObject->newtonBody) return;

	glm::mat4 rotMatrix;
	NewtonSetEulerAngle((float*)&rotation, (float*)&rotMatrix);

	glm::mat4 objMatrix;
	NewtonBodyGetMatrix(physicsObject->newtonBody, (float*)&objMatrix);

	objMatrix[0] = rotMatrix[0];
	objMatrix[1] = rotMatrix[1];
	objMatrix[2] = rotMatrix[2];

	NewtonBodySetMatrix(physicsObject->newtonBody, (float*)&objMatrix);
}

void NewtonApi::SetScale(PhysicsObject * object, glm::vec3 scale)
{
	NewtonPhysicsObject * physicsObject = static_cast<NewtonPhysicsObject*>(object);

	if(!physicsObject->newtonBody) return;

	//NewtonCollision * newtonCollision = NewtonBodyGetCollision(physicsObject->newtonBody);

	//NewtonCollisionSetScale(newtonCollision, scale.x, scale.y, scale.z);

	NewtonBodySetCollisionScale(physicsObject->newtonBody, scale.x, scale.y, scale.z);

	//ApplyMassMatrix(physicsObject);
}

void NewtonApi::SetTargetMatrix(PhysicsObject * object, glm::mat4 matrix)
{
	NewtonPhysicsObject * physicsObject = static_cast<NewtonPhysicsObject*>(object);

	if(!physicsObject->newtonBody) return;

	// HACK to orientate capsule collisions consistently with their graphics and Leap Motion counterparts
	if(physicsObject->spec->GetType() == NewtonPhysicsSpec::Capsule)
	{
		glm::mat4 correctionMatrix = glm::eulerAngleY(float(M_PI / -2.0));
		matrix = matrix * correctionMatrix;
	}

	for(unsigned i = 0; i < 3; i++)
	{
		float basisVectorLength = glm::length(matrix[i]);
		if(fabs(basisVectorLength - 1.0f) > 1.0e-4f)
		{
			__debugbreak();
		}
	}

	// Need to check for invalid matrices and discard!
	// But should I do it here, or in the script callback??
	// Do it in the script callback for now. 
	// If I find that it's needed here, I'll move it.

	NewtonBodySetMatrix(physicsObject->newtonBody, (float*)&matrix);
}

void NewtonApi::SetMass(PhysicsObject * object, float mass)
{
	NewtonPhysicsObject * physicsObject = static_cast<NewtonPhysicsObject*>(object);

	physicsObject->spec->mass = mass;
}

void NewtonApi::SetMaterial(PhysicsObject * object, PhysicsMaterial * material)
{
	if(!object || !material) return;
	NewtonPhysicsObject * physicsObject = static_cast<NewtonPhysicsObject*>(object);
	NewtonPhysicsMaterial * physicsMaterial = static_cast<NewtonPhysicsMaterial*>(material);

	physicsObject->spec->materialIndex = physicsMaterial->index;
}


void NewtonApi::SetSpringProperty(PhysicsSpring * spring, PhysicsSpring::Property prop, float value)
{
	NewtonPhysicsSpring * physicsSpring = static_cast<NewtonPhysicsSpring*>(spring);

	switch(prop)
	{
	case PhysicsSpring::Stiffness:
		physicsSpring->stiffness = value;
		break;
	case PhysicsSpring::Damping:
		physicsSpring->damping = value;
		break;
	case PhysicsSpring::Length:
		physicsSpring->length = value;
		break;
	case PhysicsSpring::Extends:
		physicsSpring->extension = value > 0.0f;
		break;
	case PhysicsSpring::Compresses:
		physicsSpring->compression = value > 0.0f;
		break;
	}
}

LocalMesh * NewtonApi::GetDebugMesh(PhysicsObject * object)
{
	NewtonPhysicsObject * physicsObject = static_cast<NewtonPhysicsObject*>(object);
	NewtonCollision * collision = NewtonBodyGetCollision(physicsObject->newtonBody);

	std::vector<Vertex_PosNor> verts;

	glm::mat4 matrix;
	NewtonCollisionGetMatrix(collision, (float*)&matrix);
	matrix = glm::inverse(matrix);
	NewtonCollisionForEachPolygonDo(collision, (float*)&matrix, DebugPolygonCallback, (void*)&verts);

	LocalMesh * localMesh = new LocalMesh();
	localMesh->vertexBuffer = new VertexBuffer<Vertex_PosNor>(verts.size());
	memcpy(localMesh->vertexBuffer->GetData(), verts.data(), verts.size() * sizeof(Vertex_PosNor));

	return localMesh;
}

} // namespace Ingenuity

#endif // USE_NEWTON_PHYSICSAPI
