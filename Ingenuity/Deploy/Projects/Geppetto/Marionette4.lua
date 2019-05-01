
-- Noticed that the little finger can't lift as high as the thumb, maybe some more compensation needed

-- TODO:
-- Blinking
-- Correction of movement - centred about the palm
-- Ability to detach the marionette at will
-- Landing face-up
-- Improved arm control

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
	
	{ "capsule",  0, 0.01, 0.08,  5.0,  0.00, PI_2, 0.00, 0.00, 0.00, 0.06, -3.11,-0.00, 0.00, 0.00, 0.015,-0.008,0.00, 0.05, 1.15, 0.00, 0.00,-PI_2, 0.00, 0.00,-PI_2, 12 }, --  2 - rthigh
	{ "capsule",  2, 0.01, 0.09,  2.5,  0.00, PI_2, 0.00, 0.00, 0.00, 0.05,  0.00,-0.00, 0.00, 0.00, 0.00, 0.105, 0.00,-2.62,-0.50, 0.00, 0.00,-PI_2, 0.00, 0.00,-PI_2, 13 }, --  3 - rcalf
	{ "capsule",  3, 0.01, 0.05,  1.5,  PI_2, 0.00, 0.00, 0.02, 0.00, 0.01,  0.00, 0.00, 0.00, 0.00, 0.00, 0.105, 0.00,-PI_4, 0.00, 0.00, 0.00,-PI_2, 0.00, 0.00,-PI_2, 14 }, --  4 - rfoot
	{ "capsule",  0, 0.01, 0.08,  5.0,  0.00, PI_2, 0.00, 0.00, 0.00, 0.06,  3.11,-0.00, 0.00, 0.00,-0.015,-0.008,0.00,-1.15,-0.05, 0.00, 0.00, PI_2, 0.00, 0.00, PI_2,  1 }, --  5 - lthigh
	{ "capsule",  5, 0.01, 0.09,  2.5,  0.00, PI_2, 0.00, 0.00, 0.00, 0.05,  0.00,-0.00, 0.00, 0.00, 0.00, 0.105, 0.00, 0.50, 2.62, 0.00, 0.00, PI_2, 0.00, 0.00, PI_2,  2 }, --  6 - lcalf
	{ "capsule",  6, 0.01, 0.05,  1.5,  PI_2, 0.00, 0.00, 0.02, 0.00, 0.01,  0.00, 0.00, 0.00, 0.00, 0.00, 0.105, 0.00, 0.00, PI_4, 0.00, 0.00, PI_2, 0.00, 0.00, PI_2,  3 }, --  7 - lfoot
	
	{ "capsule",  1, 0.02, 0.02,  5.0,  0.00, PI_2, 0.00, 0.00, 0.00, 0.035, 0.00, 0.00, 0.00, 0.00, 0.00, 0.075, 0.00, 0.00, 0.00, 0.00,-PI_2, 0.00, 0.00,-PI_2, 0.00, 11 }, --  8 - head
	
	{ "capsule",  1, 0.01, 0.07,  5.0,  0.00, PI_2, 0.00, 0.00, 0.00, 0.03,  1.57, 0.00, 3.15, 0.00, 0.03, 0.058, 1.40, 0.00, 0.00, 0.00,-PI_2, 0.00, PI_2,-PI_6, PI_2,  8 }, --  9 - ruparm
	{ "capsule",  9, 0.01, 0.06,  3.5,  0.00, PI_2, 0.00, 0.00, 0.00, 0.03,  0.00,-0.00, 0.00, 0.00, 0.00, 0.079, 0.00, 0.30, 2.62, 0.00, 0.00, PI_2, 0.00, 0.00, PI_2,  9 }, -- 10 - rfrarm
	{ "capsule", 10, 0.01, 0.02,  1.0,  0.00, PI_2, 0.00, 0.00, 0.00, 0.02,  0.00, 0.00,-PI_2, 0.00, 0.00, 0.077, 0.00, 0.00, 0.00, 0.00, 0.00, PI_2, 0.00, 0.00, PI_2, 10 }, -- 11 - rhand
	{ "capsule",  1, 0.01, 0.07,  5.0,  0.00, PI_2, 0.00, 0.00, 0.00, 0.03, -1.57, 0.00,-3.15, 0.00,-0.03, 0.058, 1.40, 0.00, 0.00, 0.00,-PI_2, 0.00, PI_2,-PI_6,-PI_2,  5 }, -- 12 - luparm
	{ "capsule", 12, 0.01, 0.06,  3.5,  0.00, PI_2, 0.00, 0.00, 0.00, 0.03,  0.00,-0.00, 0.00, 0.00, 0.00, 0.079, 0.00,-2.62,-0.30, 0.00, 0.00,-PI_2, 0.00, 0.00,-PI_2,  6 }, -- 13 - lfrarm
	{ "capsule", 13, 0.01, 0.02,  1.0,  0.00, PI_2, 0.00, 0.00, 0.00, 0.02,  0.00, 0.00, PI_2, 0.00, 0.00, 0.077, 0.00, 0.00, 0.00, 0.00, 0.00,-PI_2, 0.00, 0.00,-PI_2,  7 }, -- 14 - lhand
};

SPRING_BONE = 1;
SPRING_FINGER = 2;
SPRING_BONE_OFFSET = 3;

SPRING_STIFFNESS = 4;
SPRING_DAMPING = 5;
SPRING_LENGTH = 6;
SPRING_TORQUE_DAMPING = 7;

SPRING_LEAP_FUNCTION = 8;

function GetArmMatrix()
	if not armLocked then
		armMatrix = armMotionMatrix * GetLeapBoneMatrix(leapHelper, 45);
	end
	return armMatrix;
end

function shoulderClosure(offsetVector)
	return function()
		local leapMatrix   = GetArmMatrix();
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
		return GetArmMatrix() * preOffsetVector;
	end
end

function armClosure(fingerIndex, offsetVector)
	local scaleMatrix = CreateMatrix();
	scaleMatrix[0] = CreateVector(1.5, 0,   0,   0); -- Scale by 1.5 in the X axis
	scaleMatrix[1] = CreateVector(0,   4.5, 0,   0); -- Scale by 4.5 in the Y axis
	scaleMatrix[2] = CreateVector(0,   0,   2.0, 0); -- Scale by 2.0 in the Z axis
	return function()
		local fX,fY,fZ = GetLeapBonePosition(leapHelper,fingerIndex);
		return (GetArmMatrix() * CreateVector( 0.00, 0.00, 0.55, 1.00 )) + (scaleMatrix * (InvMatrix(GetLeapBoneMatrix(leapHelper,43)) * CreateVector(fX,fY,fZ,1)) + offsetVector);
	end
end

springDefs = {
	{ BONE_SPINE0, 45, CreateVector(   0.05, 0.0, 0.05, 1.0), 1000, 40, 0.24, 0.1, shoulderClosure(CreateVector( -0.1, 0.0,-0.1, 1.0 )) },
	{ BONE_SPINE0, 45, CreateVector(  -0.05, 0.0, 0.05, 1.0), 1000, 40, 0.24, 0.1, shoulderClosure(CreateVector(  0.1, 0.0,-0.1, 1.0 )) },
	{ BONE_LTHIGH, 43, CreateVector(  -0.04, 0.0, 0.05, 1.0),  500, 1, 0.43, 0.1, legClosure(CreateVector(-0.05, 0.0, 0.55, 1.0 )) },
	{ BONE_RTHIGH, 43, CreateVector(  -0.04, 0.0, 0.05, 1.0),  500, 1, 0.43, 0.1, legClosure(CreateVector( 0.05, 0.0, 0.55, 1.0 )) },
	--{ BONE_LCALF, 43, CreateVector(  -0.04, 0.0, 0.05, 1.0),  1000, 100, 0.50, 0.1, legClosure(CreateVector(-0.1, 0.0, 0.6, 1.0 )) },
	--{ BONE_RCALF, 43, CreateVector(  -0.04, 0.0, 0.05, 1.0),  1000, 100, 0.50, 0.1, legClosure(CreateVector( 0.1, 0.0, 0.6, 1.0 )) },
	{ BONE_RHAND,  42, CreateVector(   0.01, 0.0, 0.00, 1.0),  500, 40, 0.24, 0.1, armClosure(41, CreateVector( 0.0, -0.2, -0.05, 1.0 )) },
	{ BONE_LHAND,  25, CreateVector(   0.01, 0.0, 0.00, 1.0),  500, 40, 0.24, 0.1, armClosure(25, CreateVector( 0.0, 0.0, -0.03, 1.0 )) }
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
	
	---------------------------
	-- BEGIN PHYSICS RAGDOLL --
	---------------------------
	
	physicsRagdoll = CreatePhysicsRagdoll(physicsWorld);
	
	local currentBone = nil;
	numRagdollBones = 0;	
	ragdollBones = {};
	boneModels = {};
	boneMatrices = {};
	jointMatrices = {};
	initialInvMatrices = {};
	debugModels = {};
	prevArmMatrix = CreateMatrix();
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
		
		-- Set local position/rotation
		SetPhysicsPosition(currentBone,definition[BONE_X],definition[BONE_Y],definition[BONE_Z],true);
		SetPhysicsRotation(currentBone,definition[BONE_PITCH],definition[BONE_YAW],definition[BONE_ROLL],true);
		
		-- Set world position/rotation
		SetPhysicsPosition(currentBone,definition[BODY_X],definition[BODY_Y],definition[BODY_Z],false);
		SetPhysicsRotation(currentBone,definition[BODY_PITCH],definition[BODY_YAW],definition[BODY_ROLL],false);
		
		debugModels[i] = GetPhysicsDebugModel(currentBone);
		
		local treeDepth = 0;
		local parent = definition[BONE_PARENT];
		while parent ~= -1 do
			treeDepth = treeDepth + 1;
			parent = definitions[parent+1][BONE_PARENT];
		end
		
		local friction = math.pow(0.80,treeDepth);
		
		--if i == BONE_LCALF or i == BONE_RCALF then
		--	friction = 0.95
		--end
		
		if i == BONE_LTHIGH or i == BONE_RTHIGH then
			friction = 0.0;
		end
		
		AddPhysicsRagdollBone(physicsRagdoll,currentBone,definition[BONE_PARENT],
			definition[JOINT_CONE],definition[JOINT_MIN],definition[JOINT_MAX],
			definition[CHILD_PITCH],definition[CHILD_YAW],definition[CHILD_ROLL],
			definition[PAREN_PITCH],definition[PAREN_YAW],definition[PAREN_ROLL],
			0.0);
		numRagdollBones = numRagdollBones + 1;
		ragdollBones[i] = currentBone;
		
	end
	
	FinalizePhysicsRagdoll(physicsRagdoll);
	
	-------------------------
	-- END PHYSICS RAGDOLL --
	-------------------------
	
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
	armLocked = false;
	
	marionetteTicket = LoadAssets(
		{"ProjectDir","Mannaquin2.dae","ColladaModel","Marionette"},
		{"ProjectDir","MarionetteFace.dae","ColladaModel","MarionetteFace"},
		{"ProjectDir","MarionetteBlink.dae","ColladaModel","MarionetteBlink"}
	);
	
	jointModel = CreateModel("PosNor",CreateSphere());
	SetModelScale(jointModel,0.03);
	SetMeshColor(jointModel,0,0,1,0,0.5);
	
	walkBeam = CreatePhysicsCuboid(0.2,0.02,0.04,false);
	walkBeamInWorld = false;
	walkBeamModel = CreateModel("PosNor",CreateCube(false));
	SetMeshScale(walkBeamModel,0,0.2,0.02,0.04);
	
	-- This is where a state machine might come in handy...
	blinkMinInterval = 0.8;
	blinkMaxInterval = 6;
	blinkDuration = 0.14;
	timeToNextBlink = 6;
	blinking = false;
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

function UpdateRagdoll()
	for i,bone in pairs(ragdollBones) do
		boneMatrices[i] = GetPhysicsMatrix(bone);
		jointMatrices[i] = GetPhysicsMatrix(bone,false); -- global!
		SetModelMatrix(boneModels[i],boneMatrices[i]);
		SetModelMatrix(debugModels[i],boneMatrices[i]);
		
		if i == BONE_HEAD then
			if leapVisibilities[43] and armLocked then
				local palmRotMatrix = GetLeapBoneMatrix(leapHelper, 43);
				
				local preMatrix = RotMatrix(0,0,-PI_2);
				preMatrix[3] = CreateVector(0,definitions[i][BONE_Z],0,1)
				
				local postMatrix = RotMatrix(0,0,PI_2);
				palmRotMatrix[3] = CreateVector(0,-definitions[i][BONE_Z],0,1); -- Shouldn't this be postMatrix and palmRotMatrix be 0?
				
				palmRotMatrix = postMatrix * palmRotMatrix * RotMatrix(0.1,-0.5,0.1) * preMatrix;
				boneMatrices[i] = boneMatrices[i] * palmRotMatrix;
			end
			
			if marionetteFaceModel then
				SetModelMatrix(marionetteFaceModel, boneMatrices[i] * initialInvMatrices[i]);
			end
		end
		
		local modelIndex = definitions[i][MODEL_MESH];
		if marionetteModel and modelIndex > -1 then
			SetModelMatrix(marionetteModels[modelIndex], boneMatrices[i] * initialInvMatrices[i]);
		end
	end
end

function UpdateSprings()
	springVisible = false;
	
	if leapVisibilities[43] then -- PALM
		
		local walkBeamOffset = CreateMatrix();
		walkBeamOffset[3] = CreateVector(0,0.2,0.55,1);
		local walkBeamMatrix = GetLeapBoneMatrix(leapHelper,45) * walkBeamOffset;
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
					--spring.object.compresses = false;
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
			RemoveFromPhysicsWorld(physicsWorld,walkBeam);
			walkBeamInWorld = false;
			print("Removed walk beam");
		end
	end
end

function UpdateMarionette(delta)
	
	if marionetteBlinkModel and leapVisibilities[43] then
		timeToNextBlink = timeToNextBlink - delta;
		if timeToNextBlink <= 0 then
			if blinking then
				marionetteFaceModel = marionetteBlinkModel;
				timeToNextBlink = blinkDuration;
				blinking = false;
			else
				marionetteFaceModel = marionetteNeutralModel
				timeToNextBlink = blinkMinInterval + (math.random() * (blinkMaxInterval - blinkMinInterval));
				blinking = true;
			end
		end
	end
	
	UpdateRagdoll();
	
	UpdateSprings();
	
	down,pressed,released = GetKeyState('g');
	if pressed then
		dragEnabled = not dragEnabled;
	end
	down,pressed,released = GetKeyState('h');
	if pressed then
		armLocked = not armLocked;
		--if armLocked then
		--	prevArmMatrix = armMatrix;
		--end
	end
	
	if marionetteTicket and IsLoaded(marionetteTicket) then
		print("Marionette Loaded");
		marionetteModel = GetAsset("Marionette");
		marionetteNeutralModel = GetAsset("MarionetteFace");
		marionetteBlinkModel = GetAsset("MarionetteBlink");
		
		SetModelScale(marionetteModel,0.03);
		
		for i=1,GetNumMeshes(marionetteNeutralModel) do SetMeshScale(marionetteNeutralModel,i-1,0.03) end
		for i=1,GetNumMeshes(marionetteBlinkModel) do SetMeshScale(marionetteBlinkModel,i-1,0.03) end
		
		marionetteModels = DecomposeModel(marionetteModel);
		marionetteFaceModel = marionetteNeutralModel
		
		marionetteTicket = nil;
	end
end

DEBUGMODEL = 1

function DrawMarionette(camera, lights, surface, effect)
	if marionetteModel then
		for i,model in pairs(marionetteModels) do
			DrawComplexModel(model, camera, lights, surface, nil, effect);
			--if i == definitions[DEBUGMODEL][MODEL_MESH] then DrawComplexModel(model, camera, lights, surface, nil, effect); end
		end
	end
	if marionetteFaceModel then
		DrawComplexModel(marionetteFaceModel, camera, lights, surface, nil, effect);
	end
end

function DrawSpring(spring, camera, lights, surface, effect)
	local boneAnchor = GetPhysicsMatrix(ragdollBones[spring.boneNumber]) * spring.boneOffset;
	local controlAnchorX, controlAnchorY, controlAnchorZ = GetPhysicsPosition(spring.controlObject);
	
	StretchModelBetween(spring.model, 0.005,
						boneAnchor.x, boneAnchor.y, boneAnchor.z,
						controlAnchorX, controlAnchorY, controlAnchorZ);
	
	DrawComplexModel(spring.model, camera, lights, surface, nil, effect);
end

function DrawMarionetteWires(camera, lights, surface, effect)	
	for i,spring in pairs(springs) do
		DrawSpring(spring, camera, lights, surface, effect);
	end
	if walkBeamInWorld then
		DrawComplexModel(walkBeamModel, camera, lights, surface, nil, effect);
	end
	for i,matrix in pairs(jointMatrices) do
		SetModelMatrix(jointModel, matrix);
		--DrawComplexModel(jointModel,camera,light,surface,nil,effect);
	end
	for i,model in pairs(boneModels) do
		SetMeshColor(model,0,1,1,1,0.5);
		--DrawComplexModel(debugModels[i],camera,nil,surface);
		--DrawComplexModel(model,camera,light,surface,nil,effect)
		--if i == DEBUGMODEL then DrawComplexModel(model,camera,light,surface,nil,effect) end
	end
end
