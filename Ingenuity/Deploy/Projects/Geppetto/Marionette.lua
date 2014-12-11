
BONE_TYPE   = 1;
BONE_PARENT = 2;
BONE_RADIUS = 3;
BONE_HEIGHT = 4;
BONE_MASS   = 5;

BONE_PITCH  = 6;
BONE_YAW    = 7;
BONE_ROLL   = 8;
BONE_X      = 9;
BONE_Y      = 10;
BONE_Z      = 11;

BODY_PITCH  = 12;
BODY_YAW    = 13;
BODY_ROLL   = 14;
BODY_X      = 15;
BODY_Y      = 16;
BODY_Z      = 17;

JOINT_CONE  = 18;
JOINT_MIN   = 19;
JOINT_MAX   = 20;
CHILD_PITCH = 21;
CHILD_YAW   = 22;
CHILD_ROLL  = 23;
PAREN_PITCH = 24;
PAREN_YAW   = 25;
PAREN_ROLL  = 26;

PI_1 = math.pi;
PI_2 = math.pi / 2;
PI_4 = math.pi / 4;
PI_6 = math.pi / 6;
PI_8 = math.pi / 8;

definitions = {
	-- 1     2       3     4     5      6     7     8     9     10    11     12    13    14    15    16    17     18    19    20      21    22    23    24    25    26
	-- type, parent, radi, heig, mass,  rotx, roty, rotz, posx, posy, posz,  rotx, roty, rotz, posz, posy, posz,  cone, jmin, jmax,   cx,   cy,   cz,   px,   py,   pz
	{ "capsule", -1, 0.07, 0.16, 30.0,  0.00, 0.00,-PI_2, 0.00, 0.00, 0.01,  PI_2, 0.00, PI_1,-0.02, 0.89, 0.00,  0.00, 0.00, 0.00, 0.00, 0.00, 0.00, 0.00, 0.00, 0.00 }, --pelvis
	{ "capsule",  0, 0.07, 0.14, 20.0,  0.00, 0.00,-PI_2, 0.00, 0.00, 0.06,  0.00, 0.00, 0.00, 0.00, 0.00, 0.10,  PI_6,-0.20, 0.20, 0.00,-PI_2, 0.00, 0.00,-PI_2, 0.00 }, --spine0
	{ "capsule",  1, 0.07, 0.12, 20.0,  0.00, 0.00,-PI_2, 0.00, 0.00, 0.06,  0.00, 0.00, 0.00, 0.00, 0.00, 0.14,  PI_6,-0.20, 0.20, 0.00,-PI_2, 0.00, 0.00,-PI_2, 0.00 }, --spine1
	{ "capsule",  2, 0.07, 0.08, 20.0,  0.00, 0.00,-PI_2, 0.00, 0.00, 0.06,  0.00, 0.00, 0.00, 0.00, 0.00, 0.14,  PI_6,-0.20, 0.20, 0.00,-PI_2, 0.00, 0.00,-PI_2, 0.00 }, --spine2
	{ "capsule",  0, 0.05, 0.34, 10.0,  0.00, PI_2, 0.00, 0.00, 0.00, 0.19,  PI_1,-0.10, 0.00, 0.00, 0.10, 0.00,  1.40,-PI_6, PI_6, 0.00,-PI_2, 0.00, PI_2,-PI_6,-PI_2 }, --lthigh
	{ "capsule",  4, 0.05, 0.34,  5.0,  0.00, PI_2, 0.00, 0.00, 0.00, 0.19,  0.00,-0.20, 0.00, 0.00, 0.00, 0.40,  0.00,-2.62, 0.00, 0.00, 0.00,-PI_2, 0.00, 0.00,-PI_2 }, --lcalf
	{ "capsule",  5, 0.05, 0.13,  3.0,  PI_2, 0.00, 0.00, 0.05, 0.00, 0.05,  0.00, 0.10, 0.00, 0.00, 0.00, 0.40,  0.00,-PI_4, PI_4, 0.00, 0.00,-PI_2, 0.00, 0.00,-PI_2 }, --lfoot
	{ "capsule",  0, 0.05, 0.34, 10.0,  0.00, PI_2, 0.00, 0.00, 0.00, 0.19,  PI_1,-0.10, 0.00, 0.00,-0.10, 0.00,  1.40,-PI_6, PI_6, 0.00,-PI_2, 0.00, PI_2,-PI_6, PI_2 }, --rthigh
	{ "capsule",  7, 0.05, 0.34,  5.0,  0.00, PI_2, 0.00, 0.00, 0.00, 0.19,  0.00,-0.20, 0.00, 0.00, 0.00, 0.40,  0.00, 0.00, 2.62, 0.00, 0.00, PI_2, 0.00, 0.00, PI_2 }, --rcalf
	{ "capsule",  8, 0.05, 0.13,  3.0,  PI_2, 0.00, 0.00, 0.05, 0.00, 0.05,  0.00, 0.10, 0.00, 0.00, 0.00, 0.40,  0.00,-PI_4, PI_4, 0.00, 0.00, PI_2, 0.00, 0.00, PI_2 }, --rfoot
	{ "capsule",  3, 0.03, 0.04,  5.0,  0.00, PI_2, 0.00, 0.00, 0.00, 0.05,  0.00, 0.00, 0.00, 0.00, 0.00, 0.14,  PI_6,-PI_6, PI_6, 0.00,-PI_2, 0.00, 0.00,-PI_2, 0.00 }, --neck
	{ "sphere",  10, 0.09, 0.00,  5.0,  0.00, PI_2, 0.00, 0.00, 0.00, 0.09,  0.00, 0.00, 0.00, 0.00, 0.00, 0.06,  PI_6,-1.05, 1.05, 0.00,-PI_2, 0.00, 0.00,-PI_2, 0.00 }, -- head
	{ "capsule",  3, 0.03, 0.23, 10.0,  0.00, PI_2, 0.00, 0.00, 0.00, 0.12,  3.04, 0.00, 3.24, 0.00, 0.14, 0.10,  1.40, PI_6, PI_6, 0.00,-PI_2, 0.00, PI_2,-PI_6, PI_2 }, --luparm
	{ "capsule", 12, 0.03, 0.23,  7.0,  0.00, PI_2, 0.00, 0.00, 0.00, 0.12,  0.00,-0.20, 0.00, 0.00, 0.00, 0.24,  0.00, 0.00, 2.62, 0.00, 0.00, PI_2, 0.00, 0.00, PI_2 }, --lfrarm
	{ "capsule", 13, 0.05, 0.05,  2.0,  0.00, PI_2, 0.00, 0.00, 0.00, 0.05,  0.00, 0.00,-PI_2, 0.00, 0.00, 0.24,  0.00,-PI_4, PI_4, 0.00, 0.00, PI_2, 0.00, 0.00, PI_2 }, --lhand
	{ "capsule",  3, 0.03, 0.23, 10.0,  0.00, PI_2, 0.00, 0.00, 0.00, 0.12, -3.04, 0.00,-3.24, 0.00,-0.14, 0.10,  1.40, PI_6, PI_6, 0.00,-PI_2, 0.00, PI_2,-PI_6,-PI_2 }, --ruparm
	{ "capsule", 15, 0.03, 0.23,  7.0,  0.00, PI_2, 0.00, 0.00, 0.00, 0.12,  0.00,-0.20, 0.00, 0.00, 0.00, 0.24,  0.00,-2.62, 0.00, 0.00, 0.00,-PI_2, 0.00, 0.00,-PI_2 }, --rfrarm
	{ "capsule", 16, 0.05, 0.05,  2.0,  0.00, PI_2, 0.00, 0.00, 0.00, 0.05,  0.00, 0.00, PI_2, 0.00, 0.00, 0.24,  0.00,-PI_4, PI_4, 0.00, 0.00,-PI_2, 0.00, 0.00,-PI_2 }, --rhand
};

function CreateMarionette(physicsWorld)
	
	-- BEGIN PHYSICS RAGDOLL --
	
	physicsRagdoll = CreatePhysicsRagdoll(physicsWorld);
	
	local currentBone = nil;
	numRagdollBones = 0;	
	ragdollBones = {};
	boneModels = {};
	debugModels = {};
	local sphVtx, sphIdx = CreateSphere();
	for i,definition in pairs(definitions) do
		if definition[BONE_TYPE] == "capsule" then
			currentBone = CreatePhysicsCapsule(definition[BONE_RADIUS],definition[BONE_HEIGHT],false);
			
			local length = definition[BONE_HEIGHT] * (0.5/definition[BONE_RADIUS]);
			local capVtx, capIdx = CreateCapsule(length);
			local capsuleModel = CreateModel("PosNor",capVtx,capIdx);
			SetModelScale(capsuleModel,definition[BONE_RADIUS] * 2);
			boneModels[i] = capsuleModel;
		else
			currentBone = CreatePhysicsSphere(definition[BONE_RADIUS],false);
			
			local sphereModel = CreateModel("PosNor",sphVtx,sphIdx);
			SetModelScale(sphereModel,definition[BONE_RADIUS] * 2);
			boneModels[i] = sphereModel;
		end
		SetPhysicsMass(currentBone,definition[BONE_MASS]);
		AddToPhysicsWorld(physicsWorld,currentBone);
		SetPhysicsPosition(currentBone,definition[BONE_X],definition[BONE_Y],definition[BONE_Z],true);
		SetPhysicsRotation(currentBone,definition[BONE_PITCH],definition[BONE_YAW],definition[BONE_ROLL],true);
		SetPhysicsPosition(currentBone,definition[BODY_X],definition[BODY_Y],definition[BODY_Z],false);
		SetPhysicsRotation(currentBone,definition[BODY_PITCH],definition[BODY_YAW],definition[BODY_ROLL],false);
		debugModels[i] = GetPhysicsDebugModel(currentBone);
		
		--print("Adding Ragdoll Bone Number "..numRagdollBones);
		AddPhysicsRagdollBone(physicsRagdoll,currentBone,definition[BONE_PARENT],
			definition[JOINT_CONE],definition[JOINT_MIN],definition[JOINT_MAX],
			definition[CHILD_PITCH],definition[CHILD_YAW],definition[CHILD_ROLL],
			definition[PAREN_PITCH],definition[PAREN_YAW],definition[PAREN_ROLL]);
		numRagdollBones = numRagdollBones + 1;
		ragdollBones[i] = currentBone;
	end
	
	FinalizePhysicsRagdoll(physicsRagdoll);
	
	-- END PHYSICS RAGDOLL --
	
	springModel = CreateModel("PosNor",CreateCylinder(1));
	SetModelScale(springModel,0.1);
	SetMeshColor(springModel,0,0,0,0);
	
	arm1SpringModel = CreateModel("PosNor",CreateCylinder(1));
	SetModelScale(arm1SpringModel,0.1);
	SetMeshColor(arm1SpringModel,0,0,0,0);
	
	arm2SpringModel = CreateModel("PosNor",CreateCylinder(1));
	SetModelScale(arm2SpringModel,0.1);
	SetMeshColor(arm2SpringModel,0,0,0,0);
	
	springVisible = false;
	
	dragEnabled = false;
	
	headAnchor = CreateVector(0,0,0,1);
	arm1Anchor = CreateVector(0,0,0,1);
	arm2Anchor = CreateVector(0,0,0,1);
	
	return physicsRagdoll;
end

function UpdateMarionette()
	for i,bone in pairs(ragdollBones) do
		local boneMatrix = GetPhysicsMatrix(bone);
		if i == 12 then
			headAnchor = boneMatrix * CreateVector(-0.09,0.0,0.0,1.0);
		end
		if i == 14 then
			arm1Anchor = boneMatrix * CreateVector(0.030, 0.0, -0.035, 1.0);
		end
		if i == 17 then
			arm2Anchor = boneMatrix * CreateVector(0.030, 0.0, -0.035, 1.0);
		end
		SetMeshMatrix(boneModels[i],0,boneMatrix);
		SetMeshMatrix(debugModels[i],0,boneMatrix);
	end
	
	down,pressed,released = GetKeyState('g');
	if pressed then
		dragEnabled = not dragEnabled;
	end
	
	springVisible = false;
	
	if leapVisibilities[42] then
		--print("PALM VISIBLE!");
		local palmX, palmY, palmZ = GetLeapBonePosition(leapHelper, 41);
		local thumbX, thumbY, thumbZ = GetLeapBonePosition(leapHelper, 24);
		local pinkyX, pinkyY, pinkyZ = GetLeapBonePosition(leapHelper, 40);
		local palmPoint = CreateVector(palmX, palmY, palmZ, 1.0);
		local headMatrix = GetPhysicsMatrix(ragdollBones[12]);
		
		if dragEnabled then
			if not physicsAnchor then
				physicsAnchor = CreatePhysicsAnchor();
				AddToPhysicsWorld(physicsWorld, physicsAnchor);
				--dragStart = InvMatrix(headMatrix) * palmPoint;
				
				physicsSpring = CreatePhysicsSpring(ragdollBones[12],physicsAnchor,CreateVector(-0.09,0.0,0.0,1.0),CreateVector(0,0,0,0));
				physicsSpring.stiffness = 1500;
				physicsSpring.damping = 20;
				physicsSpring.length = 1;
				
				-- 24 - leap thumb tip
				-- 40 - leap pinky tip
				-- 14 - ragdoll right arm
				-- 17 - ragdoll left arm
				
				arm1Spring = CreatePhysicsSpring(ragdollBones[17],leapPhysicals[24],CreateVector(0.030, 0.0, -0.035, 1.0),CreateVector(0,0,0,0));
				arm1Spring.stiffness = 1500;
				arm1Spring.damping = 20;
				arm1Spring.length = 1.4;
				
				arm2Spring = CreatePhysicsSpring(ragdollBones[14],leapPhysicals[40],CreateVector(0.030, 0.0, -0.035, 1.0),CreateVector(0,0,0,0));
				arm2Spring.stiffness = 1500;
				arm2Spring.damping = 20;
				arm2Spring.length = 1.4;
			end
			
			SetPhysicsPosition(physicsAnchor, palmPoint.x, palmPoint.y, palmPoint.z);
			
			StretchModelBetween(springModel, 0.05, headAnchor.x, headAnchor.y, headAnchor.z, palmPoint.x, palmPoint.y, palmPoint.z);
			StretchModelBetween(arm1SpringModel, 0.05, arm1Anchor.x, arm1Anchor.y, arm1Anchor.z, pinkyX, pinkyY, pinkyZ);
			StretchModelBetween(arm2SpringModel, 0.05, arm2Anchor.x, arm2Anchor.y, arm2Anchor.z, thumbX, thumbY, thumbZ);
			springVisible = true;
		else
			if physicsSpring then
				physicsSpring = nil;
				arm1Spring = nil;
				arm2Spring = nil;
				RemoveFromPhysicsWorld(physicsWorld,physicsAnchor);
				physicsAnchor = nil;
			end
		end		
	end
end

function DrawMarionette()
	for i,model in pairs(boneModels) do
		DrawComplexModel(model,camera,light);
		--DrawComplexModel(debugModels[i],camera);
	end
	
	SetModelPosition(pickModel,headAnchor.x,headAnchor.y,headAnchor.z);
	DrawComplexModel(pickModel,camera,light);
	SetModelPosition(pickModel,arm1Anchor.x,arm1Anchor.y,arm1Anchor.z);
	DrawComplexModel(pickModel,camera,light);
	SetModelPosition(pickModel,arm2Anchor.x,arm2Anchor.y,arm2Anchor.z);
	DrawComplexModel(pickModel,camera,light);
	
	if springVisible then
		DrawComplexModel(springModel,camera,light);
		DrawComplexModel(arm1SpringModel,camera,light);
		DrawComplexModel(arm2SpringModel,camera,light);
	end
end
