
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

MODEL_MESH  = 27;

PI_1 = math.pi;
PI_2 = math.pi / 2;
PI_4 = math.pi / 4;
PI_6 = math.pi / 6;
PI_8 = math.pi / 8;

definitions = {
	--                                  local                                relative
	-- 1     2       3     4     5      6     7     8     9     10    11     12    13    14    15    16    17     18    19    20      21    22    23    24    25    26
	-- type, parent, radi, heig, mass,  rotx, roty, rotz, posx, posy, posz,  rotx, roty, rotz, posz, posy, posz,  cone, jmin, jmax,   cx,   cy,   cz,   px,   py,   pz
	{ "capsule", -1, 0.01, 0.04, 30.0,  0.00, 0.00,-PI_2, 0.00, 0.00, 0.01,  PI_2,-PI_2, PI_1,-0.00, 0.10, 0.00,  0.00, 0.00, 0.00, 0.00, 0.00, 0.00, 0.00, 0.00, 0.00,  0 }, --  0 - pelvis
	{ "capsule",  0, 0.01, 0.04, 20.0,  0.00, 0.00,-PI_2, 0.00, 0.00, 0.055, 0.00, 0.00, 0.00, 0.00, 0.00, 0.045, PI_6,-0.20, 0.20, 0.00,-PI_2, 0.00, 0.00,-PI_2, 0.00,  4 }, --  1 - spine0
	
	{ "capsule",  0, 0.01, 0.08, 10.0,  0.00, PI_2, 0.00, 0.00, 0.00, 0.06, -2.63,-0.00, 0.00, 0.00, 0.015,-0.008,1.40,-PI_6, PI_6, 0.00,-PI_2, 0.00, PI_2,-PI_6,-PI_2, 12 }, --  2 - rthigh
	{ "capsule",  2, 0.01, 0.09,  5.0,  0.00, PI_2, 0.00, 0.00, 0.00, 0.05,  0.00,-0.00, 0.00, 0.00, 0.00, 0.105, 0.00,-2.62, 0.00, 0.00, 0.00,-PI_2, 0.00, 0.00,-PI_2, 13 }, --  3 - rcalf
	{ "capsule",  3, 0.01, 0.05,  3.0,  PI_2, 0.00, 0.00, 0.02, 0.00, 0.01,  0.00, 0.00, 0.00, 0.00, 0.00, 0.105, 0.00,-PI_4, PI_4, 0.00, 0.00,-PI_2, 0.00, 0.00,-PI_2, 14 }, --  4 - rfoot
	{ "capsule",  0, 0.01, 0.08, 10.0,  0.00, PI_2, 0.00, 0.00, 0.00, 0.06,  2.69,-0.00, 0.00, 0.00,-0.015,-0.008,1.40,-PI_6, PI_6, 0.00,-PI_2, 0.00, PI_2,-PI_6, PI_2,  1 }, --  5 - lthigh
	{ "capsule",  5, 0.01, 0.08,  5.0,  0.00, PI_2, 0.00, 0.00, 0.00, 0.05,  0.00,-0.00, 0.00, 0.00, 0.00, 0.105, 0.00, 0.00, 2.62, 0.00, 0.00, PI_2, 0.00, 0.00, PI_2,  2 }, --  6 - lcalf
	{ "capsule",  6, 0.01, 0.05,  3.0,  PI_2, 0.00, 0.00, 0.02, 0.00, 0.01,  0.00, 0.00, 0.00, 0.00, 0.00, 0.105, 0.00,-PI_4, PI_4, 0.00, 0.00, PI_2, 0.00, 0.00, PI_2,  3 }, --  7 - lfoot
	
	{ "capsule",  1, 0.02, 0.02,  5.0,  0.00, PI_2, 0.00, 0.00, 0.00, 0.035, 0.00, 0.00, 0.00, 0.00, 0.00, 0.075, PI_6,-1.05, 1.05, 0.00,-PI_2, 0.00, 0.00,-PI_2, 0.00,  8 }, --  8 - head
	
	{ "capsule",  1, 0.01, 0.08, 10.0,  0.00, PI_2, 0.00, 0.00, 0.00, 0.03,  1.57, 0.00, 3.15, 0.00, 0.03, 0.058, 1.40, PI_6, PI_6, 0.00,-PI_2, 0.00, PI_2,-PI_6, PI_2,  5 }, --  9 - ruparm
	{ "capsule",  9, 0.01, 0.08,  7.0,  0.00, PI_2, 0.00, 0.00, 0.00, 0.03,  0.00,-0.00, 0.00, 0.00, 0.00, 0.079, 0.00, 0.62, 2.62, 0.00, 0.00, PI_2, 0.00, 0.00, PI_2,  6 }, -- 10 - rfrarm
	{ "capsule", 10, 0.01, 0.02,  2.0,  0.00, PI_2, 0.00, 0.00, 0.00, 0.02,  0.00, 0.00,-PI_2, 0.00, 0.00, 0.077, 0.00,-PI_4, PI_4, 0.00, 0.00, PI_2, 0.00, 0.00, PI_2,  7 }, -- 11 - rhand
	{ "capsule",  1, 0.01, 0.08, 10.0,  0.00, PI_2, 0.00, 0.00, 0.00, 0.03, -1.57, 0.00,-3.15, 0.00,-0.03, 0.058, 1.40, PI_6, PI_6, 0.00,-PI_2, 0.00, PI_2,-PI_6,-PI_2,  9 }, -- 12 - luparm
	{ "capsule", 12, 0.01, 0.08,  7.0,  0.00, PI_2, 0.00, 0.00, 0.00, 0.03,  0.00,-0.00, 0.00, 0.00, 0.00, 0.079, 0.00,-2.62,-0.62, 0.00, 0.00,-PI_2, 0.00, 0.00,-PI_2, 10 }, -- 13 - lfrarm
	{ "capsule", 13, 0.01, 0.02,  2.0,  0.00, PI_2, 0.00, 0.00, 0.00, 0.02,  0.00, 0.00, PI_2, 0.00, 0.00, 0.077, 0.00,-PI_4, PI_4, 0.00, 0.00,-PI_2, 0.00, 0.00,-PI_2, 11 }, -- 14 - lhand
};

SPRING_BONE = 1;
SPRING_FINGER = 2;
SPRING_OFFSET = 3;

SPRING_STIFFNESS = 4;
SPRING_DAMPING = 5;
SPRING_LENGTH = 6;

springDefs = {
	{ 9,  42, CreateVector(-0.025, 0.0, 0.0, 1.0), 1500, 20, 0.1  },
	{ 12, 41, CreateVector(  0.01, 0.0, 0.0, 1.0), 1500, 20, 0.24 },
	{ 15, 25, CreateVector(  0.01, 0.0, 0.0, 1.0), 1500, 20, 0.24 }
};
springBoneAnchors = {};
springLeapAnchors = {};
springObjects = {};
springModels = {};

function CreateMarionette(physicsWorld)
	
	-- BEGIN PHYSICS RAGDOLL --
	
	physicsRagdoll = CreatePhysicsRagdoll(physicsWorld);
	
	local currentBone = nil;
	numRagdollBones = 0;	
	ragdollBones = {};
	boneModels = {};
	jointMatrices = {};
	initialInvMatrices = {};
	debugModels = {};
	local sphVtx, sphIdx = CreateSphere();
	local cubVtx, cubIdx = CreateCube(false);
	for i,definition in pairs(definitions) do
		if definition[BONE_TYPE] == "capsule" then
			currentBone = CreatePhysicsCapsule(definition[BONE_RADIUS],definition[BONE_HEIGHT],false);
			
			local length = definition[BONE_HEIGHT] * (0.5/definition[BONE_RADIUS]);
			local capVtx, capIdx = CreateCapsule(length);
			local capsuleModel = CreateModel("PosNor",capVtx,capIdx);
			SetMeshScale(capsuleModel,0,definition[BONE_RADIUS] * 2);
			SetMeshRotation(capsuleModel,0,0,math.pi/2,0);
			boneModels[i] = capsuleModel;
		elseif definition[BONE_TYPE] == "cuboid" then
			currentBone = CreatePhysicsCuboid(definition[BONE_HEIGHT],definition[BONE_HEIGHT],definition[BONE_RADIUS],false);
			
			local cubeModel = CreateModel("PosNor",cubVtx,cubIdx);
			SetMeshScale(cubeModel,0,definition[BONE_HEIGHT],definition[BONE_HEIGHT],definition[BONE_RADIUS],false);
			boneModels[i] = cubeModel;
		else
			currentBone = CreatePhysicsSphere(definition[BONE_RADIUS],false);
			
			local sphereModel = CreateModel("PosNor",sphVtx,sphIdx);
			SetMeshScale(sphereModel,0,definition[BONE_RADIUS] * 2);
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
		--local addBone = true;
		--for j,springDef in pairs(springDefs) do
		--	if i == 8 then
		--		addBone = false;
		--	end
		--end
		
		AddPhysicsRagdollBone(physicsRagdoll,currentBone,definition[BONE_PARENT],
			definition[JOINT_CONE],definition[JOINT_MIN],definition[JOINT_MAX],
			definition[CHILD_PITCH],definition[CHILD_YAW],definition[CHILD_ROLL],
			definition[PAREN_PITCH],definition[PAREN_YAW],definition[PAREN_ROLL]);
		numRagdollBones = numRagdollBones + 1;
		ragdollBones[i] = currentBone;
	end
	
	FinalizePhysicsRagdoll(physicsRagdoll);
	
	-- END PHYSICS RAGDOLL --
	
	for i,bone in pairs(ragdollBones) do
		initialInvMatrices[i] = InvMatrix(GetPhysicsMatrix(bone));
	end
	
	for i,springDef in pairs(springDefs) do
		springModels[i] = CreateModel("PosNor",CreateCylinder(1));
		SetModelScale(springModels[i],0.1);
		SetMeshColor(springModels[i],0,0,0,0);
		
		springBoneAnchors[i] = CreateVector(0,0,0,1);
	end
	
	springVisible = false;
	
	dragEnabled = false;
	
	marionetteTicket = LoadAssets({"ProjectDir","Mannaquin.dae","ColladaModel","Mannequin"});
	
	jointModel = CreateModel("PosNor",CreateSphere());
	SetModelScale(jointModel,0.03);
	SetMeshColor(jointModel,0,0,1,0,0.5);
end

function Reload()
	print("Reloading");
	--CreateMarionette(physicsWorld);
end

function UpdateMarionette()
	for i,bone in pairs(ragdollBones) do
		local boneMatrix = GetPhysicsMatrix(bone);
		jointMatrices[i] = GetPhysicsMatrix(bone,false); -- global!
		for j,springDef in pairs(springDefs) do
			if i == springDef[SPRING_BONE] then
				springBoneAnchors[j] = boneMatrix * springDef[SPRING_OFFSET];
			end
		end
		SetModelMatrix(boneModels[i],boneMatrix);
		SetModelMatrix(debugModels[i],boneMatrix);
		
		local marionetteMesh = definitions[i][MODEL_MESH];
		if marionetteModel and marionetteMesh > -1 then
			SetModelMatrix(marionetteModels[marionetteMesh],boneMatrix * initialInvMatrices[i]);
		end
	end
	
	down,pressed,released = GetKeyState('g');
	if pressed then
		dragEnabled = not dragEnabled;
	end
	
	springVisible = false;
	
	if leapVisibilities[43] then
		--print("PALM VISIBLE!");
		
		if dragEnabled then
			for j,springDef in pairs(springDefs) do
				if not springLeapAnchors[j] then
					springLeapAnchors[j] = CreatePhysicsAnchor();
					AddToPhysicsWorld(physicsWorld, springLeapAnchors[j]);
					
					local springObject = CreatePhysicsSpring(
						ragdollBones[springDef[SPRING_BONE]],
						springLeapAnchors[j],
						springDef[SPRING_OFFSET],
						CreateVector(0,0,0,0));
					springObject.stiffness = springDef[SPRING_STIFFNESS];
					springObject.damping = springDef[SPRING_DAMPING];
					springObject.length = springDef[SPRING_LENGTH];
					springObjects[j] = springObject;
				end
				
				local leapX, leapY, leapZ = GetLeapBonePosition(leapHelper, springDef[SPRING_FINGER]);
				SetPhysicsPosition(springLeapAnchors[j], leapX, leapY, leapZ);
				
				local boneAnchor = springBoneAnchors[j];
				--StretchModelBetween(springModels[j], 0.01, boneAnchor.x, boneAnchor.y, boneAnchor.z, leapX, leapY, leapZ);
			end
			springVisible = true;
		--else
		--	if physicsSpring then
		--		physicsSpring = nil;
		--		arm1Spring = nil;
		--		arm2Spring = nil;
		--		RemoveFromPhysicsWorld(physicsWorld,palmAnchor);
		--		palmAnchor = nil;
		--	end
		end		
	end
	
	if marionetteTicket and IsLoaded(marionetteTicket) then
		marionetteModel = GetAsset("Mannequin");
		
		SetModelScale(marionetteModel,0.03);
		
		marionetteModels = DecomposeModel(marionetteModel);
		
		marionetteTicket = nil;
	end
end

function DrawMarionette(camera, surface, effect)
	if marionetteModel then
		for i,model in pairs(marionetteModels) do
			DrawComplexModel(model,camera,light,surface,nil,effect);
		end
	end
	for i,matrix in pairs(jointMatrices) do
		SetModelMatrix(jointModel, matrix);
		--DrawComplexModel(jointModel,camera,light,surface,nil,effect);
	end
	for i,model in pairs(boneModels) do
		SetMeshColor(model,0,1,1,1,0.5);
		--DrawComplexModel(debugModels[i],camera,nil,surface);
		--DrawComplexModel(model,camera,light,surface,nil,effect);
	end
end

function DrawMarionetteWires(camera, surface, effect)
	for j,springDef in pairs(springDefs) do
		local boneAnchor = springBoneAnchors[j];
		SetModelPosition(pickModel,boneAnchor.x,boneAnchor.y,boneAnchor.z);
		--DrawComplexModel(pickModel,camera,light,surface,nil,effect);
		
		if springVisible then
			--DrawComplexModel(springModels[j],camera,light,surface,nil,effect);
		end
	end
end
