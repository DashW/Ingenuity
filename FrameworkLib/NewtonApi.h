// API Wrapper for Newton Game Dynamics
// http://newtondynamics.com/forum/newton.php

#pragma once

#include "PhysicsApi.h"
#include <glm/gtc/quaternion.hpp>
#include <map>

class NewtonWorld;
class NewtonBody;
class NewtonJoint;
class NewtonMaterial;

class RagDollManager;

namespace Ingenuity {

struct NewtonPhysicsWorld : public PhysicsWorld
{
	NewtonWorld * newtonWorld;

	NewtonPhysicsWorld(NewtonWorld * newtonWorld) : newtonWorld(newtonWorld) {}
	virtual ~NewtonPhysicsWorld();
};

struct NewtonPhysicsMaterial : public PhysicsMaterial
{
	int index;

	NewtonPhysicsMaterial(int index) : index(index) {}
};

struct NewtonPhysicsSpec
{
	enum Type
	{
		Cuboid,
		Sphere,
		Capsule,
		Mesh,
		Heightmap
	};
	int materialIndex = -1;
	float mass = 1.0f;
	bool kinematic = false;

	virtual Type GetType() = 0;
	virtual ~NewtonPhysicsSpec() {}
};

struct NewtonPhysicsCuboidSpec : public NewtonPhysicsSpec
{
	virtual Type GetType() override { return Cuboid; }
	glm::vec3 dimensions;
};
struct NewtonPhysicsSphereSpec : public NewtonPhysicsSpec
{
	virtual Type GetType() override { return Sphere; }
	float radius = 0.0f;
};
struct NewtonPhysicsCapsuleSpec : public NewtonPhysicsSpec
{
	virtual Type GetType() override { return Capsule; }
	float radius = 0.0f;
	float length = 0.0f;
};
struct NewtonPhysicsMeshSpec : public NewtonPhysicsSpec
{
	virtual Type GetType() override { return Mesh; }
	LocalMesh * mesh = 0;
	bool deleteLocal = false;
	virtual ~NewtonPhysicsMeshSpec()
	{
		if(deleteLocal) delete mesh;
	}
};
struct NewtonPhysicsHeightmapSpec : public NewtonPhysicsSpec
{
	virtual Type GetType() override { return Heightmap; }
	HeightParser * heightParser = 0;
};

struct NewtonPhysicsObject : public PhysicsObject
{
	NewtonPhysicsSpec * spec;
	NewtonBody * newtonBody;

	glm::vec3 curPosition;
	glm::vec3 prevPosition;
	glm::quat curRotation;
	glm::quat prevRotation;
	
	NewtonPhysicsObject(NewtonPhysicsSpec * spec) : spec(spec), newtonBody(0) {}
	virtual ~NewtonPhysicsObject();

	void RemoveFromWorld();
};

struct NewtonPhysicsRagdoll : public PhysicsRagdoll
{
	RagDollManager * manager;

	NewtonPhysicsRagdoll() : manager(0) {}
	virtual ~NewtonPhysicsRagdoll();
};

class NewtonApi : public PhysicsApi
{
	float physicsTime;
	long long microseconds;
	bool reentrantUpdate;

	typedef std::pair<int, int> PairKey;
	typedef std::vector<PhysicsMaterial::Properties> MaterialBank;
	typedef std::map<PairKey, PhysicsMaterial::Properties> MaterialPairBank;
	MaterialBank materialBank;
	MaterialPairBank materialPairBank;

	static void SetTransformCallback(const NewtonBody * const body, const float * const matrix, int threadIndex);
	static int AABBOverlapCallback(const NewtonMaterial* const material, const NewtonBody* const body0, const NewtonBody* const body1, int threadIndex);
	//static int CompoundAABBOverlapCallback(const NewtonMaterial* const material, const NewtonBody* const body0, const void* const collsionNode0, const NewtonBody* const body1, const void* const collsionNode1, int threadIndex);
	static void ContactCollisionCallback(const NewtonJoint* contactJoint, float timestep, int threadIndex);
	static void DebugPolygonCallback(void* userData, int vertexCount, const float* faceVertec, int id);
	
	//int GetNewtonMaterialID(NewtonWorld * newtonWorld, PhysicsMaterial * material);
	static void ApplyMassMatrix(NewtonPhysicsObject * physicsObject);
	 
public:
	static void ApplyForceAndTorqueCallback(const NewtonBody * body, float timestep, int threadIndex);

	NewtonApi();
	virtual ~NewtonApi();

	virtual PhysicsWorld * CreateWorld(float scale, glm::vec3 gravity = glm::vec3(0.0f, -1.0f, 0.0f)) override;

	virtual PhysicsMaterial * CreateMaterial(PhysicsMaterial::Properties & properties) override;
	virtual void SetMaterial(PhysicsMaterial * material, PhysicsMaterial::Properties & properties) override;
	virtual void OverrideMaterialPair(PhysicsMaterial * mat1, PhysicsMaterial * mat2, PhysicsMaterial::Properties & properties) override;

	virtual PhysicsObject * CreateCuboid(glm::vec3 size, bool kinematic = false) override;
	virtual PhysicsObject * CreateSphere(float radius, bool kinematic = false) override;
	virtual PhysicsObject * CreateCapsule(float radius, float length, bool kinematic = false) override;
	virtual PhysicsObject * CreateMesh(LocalMesh * mesh, bool kinematic = false, bool deleteLocal = false) override;
	virtual PhysicsObject * CreateHeightmap(HeightParser * parser) override;
	virtual PhysicsRagdoll * CreateRagdoll(PhysicsWorld * world) override;

	virtual void AddToWorld(PhysicsWorld * world, PhysicsObject * object, bool isStatic = false) override;
	virtual void RemoveFromWorld(PhysicsWorld * world, PhysicsObject * object) override;
	virtual void UpdateWorld(PhysicsWorld * world, float deltaTime) override;

	virtual void AddRagdollBone(PhysicsRagdoll * ragdoll, PhysicsObject * object, unsigned boneIndex) override;

	virtual glm::vec3 GetPosition(PhysicsObject * object) override;
	//virtual glm::vec3 GetRotation(PhysicsObject * object) override;
	virtual glm::mat4 GetMatrix(PhysicsObject * object) override;
	virtual PhysicsObject * GetRagdollObject(PhysicsRagdoll * ragdoll, unsigned index) override;
	
	virtual void SetLocalPosition(PhysicsObject * object, glm::vec3 position) override;
	virtual void SetLocalRotation(PhysicsObject * object, glm::vec3 rotation) override;
	virtual void SetPosition(PhysicsObject * object, glm::vec3 position) override;
	virtual void SetRotation(PhysicsObject * object, glm::vec3 rotation) override;
	virtual void SetScale(PhysicsObject * object, glm::vec3 scale) override;
	virtual void SetMass(PhysicsObject * object, float mass) override;
	virtual void SetMaterial(PhysicsObject * object, PhysicsMaterial * material) override;

	virtual LocalMesh * GetDebugMesh(PhysicsObject * object) override;
};

} // namespace Ingenuity