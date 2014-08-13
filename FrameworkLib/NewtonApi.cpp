#ifdef USE_NEWTON_PHYSICSAPI

#include "NewtonApi.h"

#define _NEWTON_STATIC_LIB
#include <Newton.h>
#include <profileapi.h>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/quaternion.hpp>
#include <glm/gtx/euler_angles.hpp>

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

namespace Ingenuity {

NewtonPhysicsWorld::~NewtonPhysicsWorld()
{
	if(newtonWorld)
	{
		NewtonDestroy(newtonWorld);
	}
}

NewtonPhysicsObject::~NewtonPhysicsObject()
{
	RemoveFromWorld();
	if(spec) delete spec;
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

void NewtonApi::SetTransformCallback(const NewtonBody * const body, const float * const matrix, int threadIndex)
{
	NewtonPhysicsObject * physicsObject = (NewtonPhysicsObject*)NewtonBodyGetUserData(body);

	glm::vec3 position(matrix[12], matrix[13], matrix[14]);
	glm::mat4x4 rotationMat4;

	NewtonBodyGetRotation(body, (float*)&rotationMat4[0]);
	rotationMat4 = glm::transpose(rotationMat4);

	glm::quat rotation = glm::quat_cast(rotationMat4);

	physicsObject->prevPosition = physicsObject->curPosition;
	physicsObject->prevRotation = physicsObject->curRotation;
	if(glm::dot(physicsObject->curRotation, rotation) < 0.0f) {
		physicsObject->prevRotation *= -1.0f;
	}

	physicsObject->curPosition = position;
	physicsObject->curRotation = rotation;
}


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
	return new NewtonPhysicsMaterial(materialBank.size()-1);
}

void NewtonApi::SetMaterial(PhysicsMaterial * material, PhysicsMaterial::Properties & properties)
{
	if(!material) return;
	NewtonPhysicsMaterial * physicsMaterial = static_cast<NewtonPhysicsMaterial*>(material);
	if(physicsMaterial->index > -1 && physicsMaterial->index < (int) materialBank.size())
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

void NewtonApi::AddToWorld(PhysicsWorld * world, PhysicsObject * object, bool isStatic)
{
	NewtonPhysicsWorld * physicsWorld = static_cast<NewtonPhysicsWorld*>(world);
	NewtonPhysicsObject * physicsObject = static_cast<NewtonPhysicsObject*>(object);
	
	if(!physicsObject->spec || !physicsWorld->newtonWorld) return;

	NewtonInvalidateCache(physicsWorld->newtonWorld);

	NewtonCollision * collision = 0;

	switch(physicsObject->spec->GetType())
	{
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
				attributes, heightParser->GetScale(), heightParser->GetWidth() / (float) (sideLength - 1), 0);

			glm::mat4 offset = glm::translate(heightParser->GetWidth() / -2.0f, 0.0f, heightParser->GetWidth() / -2.0f);
			NewtonCollisionSetMatrix(collision, (float*)&offset);

			delete[] attributes;
			break;
		}
	}

	NewtonBody * body = 0;

	glm::mat4 matrix;

	if(physicsObject->spec->kinematic)
	{
		body = NewtonCreateKinematicBody(physicsWorld->newtonWorld, collision, (float*) &matrix);
	}
	else
	{
		body = NewtonCreateDynamicBody(physicsWorld->newtonWorld, collision, (float*) &matrix);
	}

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
	NewtonBodySetTransformCallback(body, NewtonApi::SetTransformCallback);
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

	return matrix;
}

void NewtonApi::SetPosition(PhysicsObject * object, glm::vec3 position)
{
	NewtonPhysicsObject * physicsObject = static_cast<NewtonPhysicsObject*>(object);

	if(!physicsObject->newtonBody) return;

	glm::mat4 matrix;
	NewtonBodyGetMatrix(physicsObject->newtonBody, (float*)&matrix);

	matrix[3] = glm::vec4(position,1.0f);

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

	NewtonCollision * newtonCollision = NewtonBodyGetCollision(physicsObject->newtonBody);

	NewtonCollisionSetScale(newtonCollision, scale.x, scale.y, scale.z);

	//NewtonBodySetCollisionScale(physicsObject->newtonBody, scale.x, scale.y, scale.z);

	ApplyMassMatrix(physicsObject);
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

} // namespace Ingenuity

#endif // USE_NEWTON_PHYSICSAPI
