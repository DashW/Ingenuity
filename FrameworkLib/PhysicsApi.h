#pragma once

#include "GeoBuilder.h"
#include "HeightParser.h"

namespace Ingenuity {

struct PhysicsWorld
{
	virtual ~PhysicsWorld() {}
protected:
	PhysicsWorld() {}
};

struct PhysicsObject
{
	virtual ~PhysicsObject() {}
protected:
	PhysicsObject() {}
};

struct PhysicsMaterial
{
	struct Properties
	{
		float elasticity;
		float staticFriction;
		float kineticFriction;
		float softness;
		bool collidable;

		Properties() : 
			elasticity(0.63f),
			staticFriction(0.95f),
			kineticFriction(0.7f),
			softness(0.32f),
			collidable(true) {}

		Properties(
			float elasticity,
			float staticFriction,
			float kineticFriction,
			float softness,
			bool collidable = true) :
			elasticity(elasticity),
			staticFriction(staticFriction),
			kineticFriction(kineticFriction),
			softness(softness),
			collidable(collidable) {}
	};

	virtual ~PhysicsMaterial() {}
protected:
	PhysicsMaterial() {}
};

struct PhysicsRagdoll
{
	virtual ~PhysicsRagdoll() {}
protected:
	PhysicsRagdoll() {}
};

class PhysicsApi
{
public:
	PhysicsApi() {}
	virtual ~PhysicsApi() {}

	virtual PhysicsWorld * CreateWorld(float scale, glm::vec3 gravity = glm::vec3(0.0f, -1.0f, 0.0f)) = 0;

	virtual PhysicsMaterial * CreateMaterial(PhysicsMaterial::Properties & properties) = 0;
	virtual void SetMaterial(PhysicsMaterial * material, PhysicsMaterial::Properties & properties) = 0;
	virtual void OverrideMaterialPair(PhysicsMaterial * mat1, PhysicsMaterial * mat2, PhysicsMaterial::Properties & properties) = 0;

	virtual PhysicsObject * CreateCuboid(glm::vec3 size, bool kinematic = false) = 0;
	virtual PhysicsObject * CreateSphere(float radius, bool kinematic = false) = 0;
	virtual PhysicsObject * CreateCapsule(float radius, float length, bool kinematic = false) = 0;
	virtual PhysicsObject * CreateMesh(LocalMesh * mesh, bool kinematic = false, bool deleteLocal = false) = 0;
	virtual PhysicsObject * CreateHeightmap(HeightParser * parser) = 0;
	virtual PhysicsRagdoll * CreateRagdoll(PhysicsWorld * world) = 0;

	virtual void AddToWorld(PhysicsWorld * world, PhysicsObject * object, bool isStatic = false) = 0;
	virtual void RemoveFromWorld(PhysicsWorld * world, PhysicsObject * object) = 0;
	virtual void UpdateWorld(PhysicsWorld * world, float deltaTime) = 0;

	virtual void AddRagdollBone(PhysicsRagdoll * ragdoll, PhysicsObject * object, int parentIndex, glm::vec3 joint, glm::vec3 childRot, glm::vec3 parentRot) = 0;
	virtual void FinalizeRagdoll(PhysicsRagdoll * ragdoll) = 0;

	virtual glm::vec3 GetPosition(PhysicsObject * object) = 0;
	//virtual glm::vec3 GetRotation(PhysicsObject * object) = 0;
	virtual glm::mat4 GetMatrix(PhysicsObject * object) = 0;
	virtual PhysicsObject * GetRagdollObject(PhysicsRagdoll * ragdoll, unsigned index) = 0;

	virtual void SetLocalPosition(PhysicsObject * object, glm::vec3 position) = 0;
	virtual void SetLocalRotation(PhysicsObject * object, glm::vec3 rotation) = 0;
	virtual void SetPosition(PhysicsObject * object, glm::vec3 position) = 0;
	virtual void SetRotation(PhysicsObject * object, glm::vec3 rotation) = 0;
	virtual void SetScale(PhysicsObject * object, glm::vec3 scale) = 0;
	virtual void SetMass(PhysicsObject * object, float mass) = 0;
	virtual void SetMaterial(PhysicsObject * object, PhysicsMaterial * material) = 0;

	virtual LocalMesh * GetDebugMesh(PhysicsObject * object) = 0;
};

} // namespace Ingenuity