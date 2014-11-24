/* Copyright (c) <2009> <Newton Game Dynamics>
* 
* This software is provided 'as-is', without any express or implied
* warranty. In no event will the authors be held liable for any damages
* arising from the use of this software.
* 
* Permission is granted to anyone to use this software for any purpose,
* including commercial applications, and to alter it and redistribute it
* freely
*/


// NewtonVehicleControllerManager.h: interface for the NewtonVehicleControllerManager class.
//
//////////////////////////////////////////////////////////////////////

#ifndef D_CUSTOM_VEHICLE_CONTROLLER_MANAGER_H_
#define D_CUSTOM_VEHICLE_CONTROLLER_MANAGER_H_

#include <CustomJointLibraryStdAfx.h>
#include <CustomAlloc.h>
#include <CustomControllerManager.h>
#include <CustomVehicleControllerComponent.h>
#include <CustomVehicleControllerBodyState.h>

#define VEHICLE_PLUGIN_NAME			"__vehicleManager__"

class CustomVehicleControllerComponent;
class CustomVehicleControllerComponentBrake;
class CustomVehicleControllerComponentEngine;
class CustomVehicleControllerComponentSteering;


class CustomVehicleController: public CustomControllerBase
{
	public:
	class dTireForceSolverSolver;
	class dWeightDistibutionSolver;
	class dTireList: public dList<CustomVehicleControllerBodyStateTire>
	{
		public:
		dTireList()
			:dList<CustomVehicleControllerBodyStateTire>()
		{
		}
	};


	public:
	CUSTOM_JOINTS_API CustomVehicleControllerBodyStateTire* GetFirstTire () const ;
	CUSTOM_JOINTS_API CustomVehicleControllerBodyStateTire* GetNextTire (CustomVehicleControllerBodyStateTire* const tire) const;

	CUSTOM_JOINTS_API const CustomVehicleControllerBodyStateChassis& GetChassisState () const;

	CUSTOM_JOINTS_API dFloat GetAerodynamicsDowforceCoeficient () const;
	CUSTOM_JOINTS_API void SetAerodynamicsDownforceCoefficient (dFloat maxDownforceInGravities, dFloat topSpeed);

	CUSTOM_JOINTS_API void SetDryRollingFrictionTorque (dFloat torque);
	CUSTOM_JOINTS_API dFloat GetDryRollingFrictionTorque () const;

	CUSTOM_JOINTS_API CustomVehicleControllerComponentBrake* GetBrakes() const;
	CUSTOM_JOINTS_API CustomVehicleControllerComponentEngine* GetEngine() const;
	CUSTOM_JOINTS_API CustomVehicleControllerComponentBrake* GetHandBrakes() const;
	CUSTOM_JOINTS_API CustomVehicleControllerComponentSteering* GetSteering() const;

	CUSTOM_JOINTS_API void SetCenterOfGravity(const dVector& comRelativeToGeomtriCenter);
	CUSTOM_JOINTS_API CustomVehicleControllerBodyStateTire* AddTire (const CustomVehicleControllerBodyStateTire::TireCreationInfo& tireInfo);

	CUSTOM_JOINTS_API void SetBrakes(CustomVehicleControllerComponentBrake* const brakes);
	CUSTOM_JOINTS_API void SetEngine(CustomVehicleControllerComponentEngine* const engine);
	CUSTOM_JOINTS_API void SetHandBrakes(CustomVehicleControllerComponentBrake* const brakes);
	CUSTOM_JOINTS_API void SetSteering(CustomVehicleControllerComponentSteering* const steering);

	CUSTOM_JOINTS_API void LinksTiresKinematically (int count, CustomVehicleControllerBodyStateTire** const tires);
	CUSTOM_JOINTS_API void Finalize();

	protected:
	CUSTOM_JOINTS_API void Cleanup();
	CUSTOM_JOINTS_API CustomVehicleControllerBodyStateContact* GetContactBody (const NewtonBody* const body);
	CUSTOM_JOINTS_API void Init (NewtonCollision* const chassisShape, const dMatrix& vehicleFrame, dFloat mass, const dVector& gravityVector);
	
	CUSTOM_JOINTS_API bool IsSleeping();
	CUSTOM_JOINTS_API virtual void PreUpdate(dFloat timestep, int threadIndex);
	CUSTOM_JOINTS_API virtual void PostUpdate(dFloat timestep, int threadIndex);


	dTireList m_tireList;
	CustomVehicleControllerBodyStateChassis m_chassisState;
	dList<CustomVehicleControllerBodyState*> m_stateList;

	dList<CustomVehicleControllerEngineDifferencialJoint> m_tankTireLinks;
	dList<CustomVehicleControllerBodyStateContact> m_externalContactStatesPoll;
	dList<CustomVehicleControllerBodyStateContact>::dListNode* m_freeContactList;
	NewtonCollision* m_tireCastShape;
	CustomVehicleControllerComponentBrake* m_brakes;
	CustomVehicleControllerComponentEngine* m_engine;
	CustomVehicleControllerComponentBrake* m_handBrakes;
	CustomVehicleControllerComponentSteering* m_steering; 
	CustomVehicleControllerBodyStateContact* m_externalContactStates[16];
	int m_sleepCounter;
	int m_externalContactStatesCount;
	bool m_finalized;

	friend class CustomVehicleControllerManager;
	friend class CustomVehicleControllerTireJoint;
	
	friend class CustomVehicleControllerTireContactJoint;
	friend class CustomVehicleControllerEngineIdleJoint;
	friend class CustomVehicleControllerEngineDifferencialJoint;
	
	friend class CustomVehicleControllerBodyStateTire;
	friend class CustomVehicleControllerBodyStateChassis;
	friend class CustomVehicleControllerComponentBrake;
	friend class CustomVehicleControllerComponentEngine;
	friend class CustomVehicleControllerComponentSteering;
	friend class CustomVehicleControllerComponentTrackSkidSteering;
};


class CustomVehicleControllerManager: public CustomControllerManager<CustomVehicleController> 
{
	public:
	CUSTOM_JOINTS_API CustomVehicleControllerManager(NewtonWorld* const world);
	CUSTOM_JOINTS_API virtual ~CustomVehicleControllerManager();

	CUSTOM_JOINTS_API virtual CustomVehicleController* CreateVehicle (NewtonCollision* const chassisShape, const dMatrix& vehicleFrame, dFloat mass, const dVector& gravityVector);
	CUSTOM_JOINTS_API virtual void DestroyController (CustomVehicleController* const controller);
};


#endif 
