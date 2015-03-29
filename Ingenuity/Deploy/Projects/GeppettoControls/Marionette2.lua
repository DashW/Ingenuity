
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

BONE_PELVIS = 1;
BONE_SPINE0 = 2;
BONE_RTHIGH = 3;
BONE_RCALF = 4;
BONE_RFOOT = 5;
BONE_LTHIGH = 6;
BONE_LCALF = 7;
BONE_LFOOT = 8;
BONE_HEAD = 9;
BONE_RUPARM = 10;
BONE_RFRARM = 11;
BONE_RHAND = 12;
BONE_LUPARM = 13;
BONE_LFRARM = 14;
BONE_LHAND = 15;

definitions = {
	--                                  local                                relative
	-- 1     2       3     4     5      6     7     8     9     10    11     12    13    14    15    16    17     18    19    20      21    22    23    24    25    26
	-- type, parent, radi, heig, mass,  rotx, roty, rotz, posx, posy, posz,  rotx, roty, rotz, posz, posy, posz,  cone, jmin, jmax,   cx,   cy,   cz,   px,   py,   pz
	{ "capsule", -1, 0.01, 0.04, 30.0,  0.00, 0.00,-PI_2, 0.00, 0.00, 0.01,  PI_2,-PI_2, PI_1,-0.00, 0.10, 0.00,  0.00, 0.00, 0.00, 0.00, 0.00, 0.00, 0.00, 0.00, 0.00,  0 }, --  0 - pelvis
	{ "capsule",  0, 0.01, 0.04, 20.0,  0.00, 0.00,-PI_2, 0.00, 0.00, 0.055, 0.00, 0.00, 0.00, 0.00, 0.00, 0.045, 0.00,-0.20, 0.20, 0.00,-PI_2, 0.00, 0.00,-PI_2, 0.00,  4 }, --  1 - spine0
	
	{ "capsule",  0, 0.01, 0.08,  5.0,  0.00, PI_2, 0.00, 0.00, 0.00, 0.06, -2.63,-0.00, 0.00, 0.00, 0.015,-0.008,1.00, 0.00, PI_6, 0.00,-PI_2, 0.00, PI_2,-PI_6,-PI_2, 12 }, --  2 - rthigh
	{ "capsule",  2, 0.01, 0.09,  2.5,  0.00, PI_2, 0.00, 0.00, 0.00, 0.05,  0.00,-0.00, 0.00, 0.00, 0.00, 0.105, 0.00,-2.62, 0.00, 0.00, 0.00,-PI_2, 0.00, 0.00,-PI_2, 13 }, --  3 - rcalf
	{ "capsule",  3, 0.01, 0.05,  1.5,  PI_2, 0.00, 0.00, 0.02, 0.00, 0.01,  0.00, 0.00, 0.00, 0.00, 0.00, 0.105, 0.00,-PI_4, 0.00, 0.00, 0.00,-PI_2, 0.00, 0.00,-PI_2, 14 }, --  4 - rfoot
	{ "capsule",  0, 0.01, 0.08,  5.0,  0.00, PI_2, 0.00, 0.00, 0.00, 0.06,  2.69,-0.00, 0.00, 0.00,-0.015,-0.008,1.00,-PI_6, 0.00, 0.00,-PI_2, 0.00, PI_2,-PI_6, PI_2,  1 }, --  5 - lthigh
	{ "capsule",  5, 0.01, 0.08,  2.5,  0.00, PI_2, 0.00, 0.00, 0.00, 0.05,  0.00,-0.00, 0.00, 0.00, 0.00, 0.105, 0.00, 0.00, 2.62, 0.00, 0.00, PI_2, 0.00, 0.00, PI_2,  2 }, --  6 - lcalf
	{ "capsule",  6, 0.01, 0.05,  1.5,  PI_2, 0.00, 0.00, 0.02, 0.00, 0.01,  0.00, 0.00, 0.00, 0.00, 0.00, 0.105, 0.00, 0.00, PI_4, 0.00, 0.00, PI_2, 0.00, 0.00, PI_2,  3 }, --  7 - lfoot
	
	{ "capsule",  1, 0.02, 0.02,  5.0,  0.00, PI_2, 0.00, 0.00, 0.00, 0.035, 0.00, 0.00, 0.00, 0.00, 0.00, 0.075, PI_6,-1.05, 1.05, 0.00,-PI_2, 0.00, 0.00,-PI_2, 0.00,  8 }, --  8 - head
	
	{ "capsule",  1, 0.01, 0.08,  5.0,  0.00, PI_2, 0.00, 0.00, 0.00, 0.03,  1.57, 0.00, 3.15, 0.00, 0.03, 0.058, 1.40, PI_6, PI_6, 0.00,-PI_2, 0.00, PI_2,-PI_6, PI_2,  5 }, --  9 - ruparm
	{ "capsule",  9, 0.01, 0.08,  3.5,  0.00, PI_2, 0.00, 0.00, 0.00, 0.03,  0.00,-0.00, 0.00, 0.00, 0.00, 0.079, 0.00, 0.62, 2.62, 0.00, 0.00, PI_2, 0.00, 0.00, PI_2,  6 }, -- 10 - rfrarm
	{ "capsule", 10, 0.01, 0.02,  1.0,  0.00, PI_2, 0.00, 0.00, 0.00, 0.02,  0.00, 0.00,-PI_2, 0.00, 0.00, 0.077, 0.00,-PI_4, PI_4, 0.00, 0.00, PI_2, 0.00, 0.00, PI_2,  7 }, -- 11 - rhand
	{ "capsule",  1, 0.01, 0.08,  5.0,  0.00, PI_2, 0.00, 0.00, 0.00, 0.03, -1.57, 0.00,-3.15, 0.00,-0.03, 0.058, 1.40, PI_6, PI_6, 0.00,-PI_2, 0.00, PI_2,-PI_6,-PI_2,  9 }, -- 12 - luparm
	{ "capsule", 12, 0.01, 0.08,  3.5,  0.00, PI_2, 0.00, 0.00, 0.00, 0.03,  0.00,-0.00, 0.00, 0.00, 0.00, 0.079, 0.00,-2.62,-0.62, 0.00, 0.00,-PI_2, 0.00, 0.00,-PI_2, 10 }, -- 13 - lfrarm
	{ "capsule", 13, 0.01, 0.02,  1.0,  0.00, PI_2, 0.00, 0.00, 0.00, 0.02,  0.00, 0.00, PI_2, 0.00, 0.00, 0.077, 0.00,-PI_4, PI_4, 0.00, 0.00,-PI_2, 0.00, 0.00,-PI_2, 11 }, -- 14 - lhand
};

SPRING_BONE = 1;
SPRING_FINGER = 2;
SPRING_BONE_OFFSET = 3;

SPRING_STIFFNESS = 4;
SPRING_DAMPING = 5;
SPRING_LENGTH = 6;
SPRING_TORQUE_DAMPING = 7;

SPRING_LEAP_FUNCTION = 8;

-- The arms are definitely getting in the way, they have a bad habit of falling asleep.
-- So, the arms! We need to subtract the fingers from the palm and add them to the walk beam.
-- Not something we can do trivially using the spring definitions
-- Maybe a callback function for each spring would help here?

function shoulderClosure(offsetVector)
	return function()
		local leapMatrix   = GetLeapBoneMatrix(leapHelper, 43);
		local leapVector   = leapMatrix * CreateVector( 0.00, 0.00, 0.55, 1.00 );
		local orientVector = leapMatrix * CreateVector(    1,    0,    0,    0 );
		local orientAngle  = math.atan2(orientVector.z,orientVector.x);
		local offsetX      = (offsetVector.x * math.cos(orientAngle)) - (offsetVector.z * math.sin(orientAngle));
		local offsetZ      = (offsetVector.z * math.cos(orientAngle)) + (offsetVector.x * math.sin(orientAngle));
		return leapVector + CreateVector(offsetX, offsetVector.y, offsetZ, 0);
	end
end

function legClosure(preOffsetVector)
	return function()
		return GetLeapBoneMatrix(leapHelper, 43) * preOffsetVector;
	end
end

function armClosure(offsetVector)
	return function()
		-- So, the arms! We need to subtract the fingers from the palm and add them to the walk beam.
		return CreateVector(0,0,0,1);
	end
end

springDefs = {
	{ BONE_SPINE0, 43, CreateVector(   0.05, 0.0, 0.05, 1.0), 1500, 40, 0.24, 0.1, shoulderClosure(CreateVector( -0.1, 0.0,-0.1, 1.0 )) },
	{ BONE_SPINE0, 43, CreateVector(  -0.05, 0.0, 0.05, 1.0), 1500, 40, 0.24, 0.1, shoulderClosure(CreateVector(  0.1, 0.0,-0.1, 1.0 )) },
	{ BONE_LTHIGH, 43, CreateVector(  -0.04, 0.0, 0.05, 1.0), 1500, 40, 0.45, 0.1, legClosure(CreateVector(-0.1, 0.0, 0.55, 1.0 )) },
	{ BONE_RTHIGH, 43, CreateVector(  -0.04, 0.0, 0.05, 1.0), 1500, 40, 0.45, 0.1, legClosure(CreateVector( 0.1, 0.0, 0.55, 1.0 )) }
	--{ BONE_RHAND, 41, CreateVector(  0.01, 0.0, 0.0, 1.0), 1500, 20, 0.24 },
	--{ BONE_LHAND, 25, CreateVector(  0.01, 0.0, 0.0, 1.0), 1500, 20, 0.24 }
};
springBoneAnchors = {};
springLeapAnchors = {};
springObjects = {};
springModels = {};

springs = {};

function CreateMarionetteSpring(boneNumber,controlObject,boneOffset,controlOffset)
	local spring = {};
	spring.boneNumber = boneNumber;
	spring.controlObject = controlObject;
	spring.boneOffset = boneOffset;
	spring.controlOffset = controlOffset;
	spring.object = CreatePhysicsSpring(ragdollBones[boneNumber],controlObject,boneOffset,controlOffset);
	spring.model = CreateModel("PosNor",CreateCylinder(1));
	SetModelScale(spring.model,0.1);
	SetMeshColor(spring.model,0,0,0,0);
	spring.visible = true;
	
	return spring;
end

function CreateMarionette(physicsWorld)
	
	-- BEGIN PHYSICS RAGDOLL --
	
	physicsRagdoll = CreatePhysicsRagdoll(physicsWorld);
	
	local currentBone = nil;
	numRagdollBones = 0;	
	ragdollBones = {};
	boneModels = {};
	boneMatrices = {};
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
	
	walkBeam = CreatePhysicsCuboid(0.2,0.02,0.04,false);
	walkBeamInWorld = false;
	walkBeamModel = CreateModel("PosNor",CreateCube(false));
	SetMeshScale(walkBeamModel,0,0.2,0.02,0.04);
end

function Reload()
	print("Reloading");
	--CreateMarionette(physicsWorld);
end

function table.removeObject(t,o)
	for i,val in pairs(t) do
		if val == o then
			table.remove(t,i);
			i = i - 1;
		end
	end
end

function UpdateMarionette()
	for i,bone in pairs(ragdollBones) do
		boneMatrices[i] = GetPhysicsMatrix(bone);
		jointMatrices[i] = GetPhysicsMatrix(bone,false); -- global!
		for j,springDef in pairs(springDefs) do
			if i == springDef[SPRING_BONE] then
				springBoneAnchors[j] = boneMatrices[i] * springDef[SPRING_BONE_OFFSET];
			end
		end
		SetModelMatrix(boneModels[i],boneMatrices[i]);
		SetModelMatrix(debugModels[i],boneMatrices[i]);
		
		local marionetteMesh = definitions[i][MODEL_MESH];
		if marionetteModel and marionetteMesh > -1 then
			SetModelMatrix(marionetteModels[marionetteMesh],boneMatrices[i] * initialInvMatrices[i]);
		end
	end
	
	down,pressed,released = GetKeyState('g');
	if pressed then
		dragEnabled = not dragEnabled;
	end
	
	springVisible = false;
	
	if leapVisibilities[42] then
		--print("PALM VISIBLE!");
		
		local walkBeamOffset = CreateMatrix();
		walkBeamOffset[3] = CreateVector(0,0,0.55,1);
		local walkBeamMatrix = GetLeapBoneMatrix(leapHelper,43) * walkBeamOffset;
		SetModelMatrix(walkBeamModel,walkBeamMatrix);
		SetPhysicsMatrix(walkBeam,walkBeamMatrix);
		
		if not walkBeamInWorld then
			AddToPhysicsWorld(physicsWorld, walkBeam);
			walkBeamInWorld = true;
		end
		
		if dragEnabled then
			
			for j,springDef in pairs(springDefs) do
				if not springLeapAnchors[j] then
					springLeapAnchors[j] = CreatePhysicsAnchor();
					AddToPhysicsWorld(physicsWorld, springLeapAnchors[j]);
					
					local spring = CreateMarionetteSpring(
						springDef[SPRING_BONE],
						springLeapAnchors[j],
						springDef[SPRING_BONE_OFFSET],
						CreateVector(0,0,0,1))
					
					spring.object.stiffness = springDef[SPRING_STIFFNESS];
					spring.object.damping = springDef[SPRING_DAMPING];
					if springDef[SPRING_TORQUE_DAMPING] ~= nil then
						spring.object.torqueDamping = springDef[SPRING_TORQUE_DAMPING];
					end
					spring.object.length = springDef[SPRING_LENGTH];
					table.insert(springs,spring);
				end
				
				local anchorPosition = springDef[SPRING_LEAP_FUNCTION]();
				-- something going slightly wrong here
				-- there's some extraeneous torque force (eurgh!)
				SetPhysicsPosition(springLeapAnchors[j], anchorPosition.x, anchorPosition.y, anchorPosition.z);
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
	else
		if walkBeamInWorld then
			table.removeObject(springs,leftWalkSpring);
			table.removeObject(springs,rightWalkSpring);
			leftWalkSpring = nil;
			rightWalkSpring = nil;
			
			RemoveFromPhysicsWorld(physicsWorld,walkBeam);
			walkBeamInWorld = false;
			
			print("Removed walk springs");
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
		DrawComplexModel(model,camera,light,surface,nil,effect);
	end
end

function DrawSpring(spring, camera, surface, effect)
	local boneAnchor = GetPhysicsMatrix(ragdollBones[spring.boneNumber]) * spring.boneOffset;
	local controlAnchorX, controlAnchorY, controlAnchorZ = GetPhysicsPosition(spring.controlObject);
	
	StretchModelBetween(spring.model, 0.01,
						boneAnchor.x, boneAnchor.y, boneAnchor.z,
						controlAnchorX, controlAnchorY, controlAnchorZ);
	
	DrawComplexModel(spring.model, camera, surface, nil, effect);
end

function DrawMarionetteWires(camera, surface, effect)	
	for i,spring in pairs(springs) do
		DrawSpring(spring, camera, surface, effect);
	end
	
	if walkBeamInWorld then
		DrawComplexModel(walkBeamModel,camera,light,surface,nil,effect);
	end
end
