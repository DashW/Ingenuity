
Require("ProjectDir","../../Common/IngenUtils.lua");

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

definitions = {
	-- 1     2       3     4     5      6     7     8     9     10    11     12    13    14    15    16    17     18    19    20      21    22    23    24    25    26
	-- type, parent, radi, heig, mass,  rotx, roty, rotz, posx, posy, posz,  rotx, roty, rotz, posz, posy, posz,  cone, jmin, jmax,   cx,   cy,   cz,   px,   py,   pz
	{ "capsule", -1, 0.07, 0.16, 30.0,  0.00, 0.00,-PI_2, 0.00, 0.00, 0.01,  PI_2, 0.00, PI_1,-0.02, 0.89, 0.00,  0.00, 0.00, 0.00, 0.00, 0.00, 0.00, 0.00, 0.00, 0.00 }, --pelvis
	{ "capsule",  0, 0.07, 0.14, 20.0,  0.00, 0.00,-PI_2, 0.00, 0.00, 0.06,  0.00, 0.00, 0.00, 0.00, 0.00, 0.10,  PI_6,-PI_6, PI_6, 0.00,-PI_2, 0.00, 0.00,-PI_2, 0.00 }, --spine0
	{ "capsule",  1, 0.07, 0.12, 20.0,  0.00, 0.00,-PI_2, 0.00, 0.00, 0.06,  0.00, 0.00, 0.00, 0.00, 0.00, 0.14,  PI_6,-PI_6, PI_6, 0.00,-PI_2, 0.00, 0.00,-PI_2, 0.00 }, --spine1
	{ "capsule",  2, 0.07, 0.08, 20.0,  0.00, 0.00,-PI_2, 0.00, 0.00, 0.06,  0.00, 0.00, 0.00, 0.00, 0.00, 0.14,  PI_6,-PI_6, PI_6, 0.00,-PI_2, 0.00, 0.00,-PI_2, 0.00 }, --spine2
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

function Begin()
	assetTicket = LoadAssets(
		{"FrameworkDir","SkyShader.xml","Shader","skyshader"},
		{"ProjectDir","grassCubeMap.dds","CubeMap","skymap"},
		{"ProjectDir","crate.jpg","Texture","cratetex"},
		{"ProjectDir","stonefloor.bmp","Texture","floortex"}
	);

	camera = CreateCamera();
	SetCameraClipFov(camera,0.01,200,0.78539);
	SetupFlyCamera(camera, 0,-2,-10, 0.01,10);

	cubeModel = CreateModel("PosNorTex",CreateCube());
	--SetMeshColor(cubeModel,0,1.0,0.0,0.0);
	--SetModelPosition(cubeModel,0.0,4.0,0.0);
	--SetModelRotation(cubeModel,0.0,0.0,1.0);

	floorModel = CreateModel("PosNorTex",CreateCube());
	SetModelScale(floorModel,5,0.5,5);
	SetModelPosition(floorModel,0,-4,0);
	--SetMeshColor(floorModel,0,0.0,1.0,0.0);

	skyModel = CreateSkyCube();

	physicsWorld = CreatePhysicsWorld();

	--physicsCube = CreatePhysicsCuboid(2,2,2,false);
	physicsFloor = CreatePhysicsCuboid(10,1,10,false);

	--woodMaterial = CreatePhysicsMaterial(0.63,0.95,0.7,0.32);
	--SetPhysicsMaterial(physicsCube, woodMaterial);

	--AddToPhysicsWorld(physicsWorld,physicsCube,false);
	AddToPhysicsWorld(physicsWorld,physicsFloor,true);

	--SetPhysicsPosition(physicsCube,0,0,0);
	--SetPhysicsRotation(physicsCube,0,0,1);

	SetPhysicsPosition(physicsFloor,0,-4,0);
	SetPhysicsRotation(physicsFloor,0,0,0);
	
	-- BEGIN DEMO PHYSICS RAGDOLL --

	--physicsRagdoll = CreatePhysicsRagdoll(physicsWorld);
	--FinalizePhysicsRagdoll(physicsRagdoll);
	--
	--ragdollBones = {};
	--local currentBone = nil;
	--numRagdollBones = 0;
	--repeat
	--	currentBone = GetPhysicsRagdollBone(physicsRagdoll, numRagdollBones);
	--	if currentBone then
	--		print("Adding Ragdoll Bone Number "..numRagdollBones);
	--		numRagdollBones = numRagdollBones + 1;
	--		table.insert(ragdollBones, currentBone);
	--	end
	--until currentBone == nil
	--
	--boneModels = {};
	--debugModels = {};
	--local sphVtx, sphIdx = CreateSphere();
	--for i,bone in pairs(ragdollBones) do
	--	if definitions[i][BONE_TYPE] == "capsule" then
	--		local length = definitions[i][BONE_HEIGHT] * (0.5/definitions[i][BONE_RADIUS]);
	--		local capVtx, capIdx = CreateCapsule(length);
	--		local capsuleModel = CreateModel("PosNor",capVtx,capIdx);
	--		SetModelScale(capsuleModel,definitions[i][BONE_RADIUS] * 2);
	--		boneModels[i] = capsuleModel;
	--	else
	--		local sphereModel = CreateModel("PosNor",sphVtx,sphIdx);
	--		SetModelScale(sphereModel,definitions[i][BONE_RADIUS] * 2);
	--		boneModels[i] = sphereModel;
	--	end
	--	debugModels[i] = GetPhysicsDebugModel(bone);
	--end
	
	-- END DEMO PHYSICS RAGDOLL --
	
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
	
	leapModels = {};
	leapVisibilities = {};
	leapPhysicals = {};
	leapHelper = CreateLeapHelper();
	SetLeapPosition(leapHelper,0,-4.5,0);
	SetLeapScale(leapHelper,0.008);
	
	pickModel = CreateModel("PosNor",CreateSphere());
	SetModelScale(pickModel,0.1);
	SetMeshColor(pickModel,0,1,0,0);
	
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

	debugFont = GetFont(40,"Arial");
	
	light = CreateLight("directional");
	SetLightDirection(light,math.sin(2.5),0.75,math.cos(2.5));
	
	dragEnabled = false;
	
	headAnchor = CreateVector(0,0,0,1);
	arm1Anchor = CreateVector(0,0,0,1);
	arm2Anchor = CreateVector(0,0,0,1);
end

function Update(delta)
	if IsLoaded(assetTicket) then
		skyEffect = CreateEffect("skyshader");
		skyMap = GetAsset("skymap");
		crateTex = GetAsset("cratetex");
		floorTex = GetAsset("floortex");

		if skyEffect then
			SetMeshCubeMap(skyModel,0,skyMap);
			SetMeshEffect(skyModel,0,skyEffect);
		end

		SetMeshTexture(cubeModel,0,crateTex);
		SetMeshTexture(floorModel,0,floorTex);

		assetTicket = -1;
	end
	
	for i = 1,GetLeapNumBones(leapHelper) do
		visible, length, radius = GetLeapBoneDetails(leapHelper, i-1);
		leapVisibilities[i] = visible;
		if visible then
			if leapModels[i] == nil then
				leapPhysicals[i] = CreatePhysicsCapsule(radius, length, false);
				AddToPhysicsWorld(physicsWorld,leapPhysicals[i],true);
				
				local vtx, idx = CreateCapsule(length / (radius * 2.0));
				local boneModel = CreateModel("PosNor",vtx,idx);
				SetModelScale(boneModel, radius * 2.0);
				leapModels[i] = boneModel;
				print("Created Leap Model " .. i-1);
			end
			local leapBoneMatrix = GetLeapBoneMatrix(leapHelper, i-1);
			SetPhysicsMatrix(leapPhysicals[i], leapBoneMatrix);
			SetMeshMatrix(leapModels[i], 0, leapBoneMatrix);
		end
	end
	--if not leapAccum then
	--	leapAccum = CreateAccumulator();
	--	leapFrameTime = 0;
	--end
	--leapAccum:Add(GetLeapFrameTime(leapHelper));
	--if leapAccum:Sum() > 0.5 then
	--	leapFrameTime = leapAccum:Average();
	--	leapAccum:Clear(0.5);
	--end

	UpdatePhysicsWorld(physicsWorld,delta);
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

	--SyncPhysicsMatrix(physicsCube,cubeModel);
	--if assetTicket == -1 then
	--	SyncPhysicsMatrix(physicsVase,vaseModel);
	--end

	local down,pressed,released = GetKeyState('r');
	if pressed then
		--SetPhysicsPosition(physicsCube,0,0,0);
		--SetPhysicsRotation(physicsCube,0,0,1);

		--SetPhysicsPosition(physicsVase,0,8,0);
		--SetPhysicsRotation(physicsVase,0,0,0);
	end
	
	local sWidth, sHeight = GetScreenSize();
	local mouseX, mouseY = GetMousePosition();
	
	--down,pressed,released = GetMouseLeft();
	--if pickedObject and down then
	--	DragPhysicsObject(physicsWorld,camera,x/sWidth,y/sHeight);
	--else
		UpdateFlyCamera(delta);
	--end
	
	--down,pressed,released = GetKeyState('p');
	--if pressed then
		pickedObject, pickedPos = PickPhysicsObject(physicsWorld,
			CreateVector(flyCamX,flyCamY,flyCamZ,1),
			GetCameraRay(camera,mouseX/sWidth,mouseY/sHeight));
		
		if pickedObject then
			SetModelPosition(pickModel,pickedPos.x,pickedPos.y,pickedPos.z);
		end
	--end
	
	down,pressed,released = GetKeyState('g');
	if pressed then
		dragEnabled = not dragEnabled;
	end
	
	springVisible = false;
	
	physicsHead = ragdollBones[12];
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
				
				arm1Spring = CreatePhysicsSpring(ragdollBones[14],leapPhysicals[24],CreateVector(0.030, 0.0, -0.035, 1.0),CreateVector(0,0,0,0));
				arm1Spring.stiffness = 1500;
				arm1Spring.damping = 20;
				arm1Spring.length = 1.4;
				
				arm2Spring = CreatePhysicsSpring(ragdollBones[17],leapPhysicals[40],CreateVector(0.030, 0.0, -0.035, 1.0),CreateVector(0,0,0,0));
				arm2Spring.stiffness = 1500;
				arm2Spring.damping = 20;
				arm2Spring.length = 1.4;
			end
			
			SetPhysicsPosition(physicsAnchor, palmPoint.x, palmPoint.y, palmPoint.z);
			
			StretchModelBetween(springModel, 0.05, headAnchor.x, headAnchor.y, headAnchor.z, palmPoint.x, palmPoint.y, palmPoint.z);
			StretchModelBetween(arm1SpringModel, 0.05, arm1Anchor.x, arm1Anchor.y, arm1Anchor.z, thumbX, thumbY, thumbZ);
			StretchModelBetween(arm2SpringModel, 0.05, arm2Anchor.x, arm2Anchor.y, arm2Anchor.z, pinkyX, pinkyY, pinkyZ);
			springVisible = true;
		else
			if physicsSpring then
				physicsSpring = nil;
				arm1Spring = nil;
				arm2Spring = nil;
				RemoveFromPhysicsWorld(physicsAnchor);
				physicsAnchor = nil;
			end
		end		
	end
	
	UpdateFrameTime(delta);
end

function Draw()
	DrawComplexModel(floorModel,camera);
	--DrawComplexModel(cubeModel,camera);

	for i,model in pairs(boneModels) do
		DrawComplexModel(model,camera,light);
		--DrawComplexModel(debugModels[i],camera);
	end
	
	for i,leapModel in pairs(leapModels) do
		if leapModel and leapVisibilities[i] then
			DrawComplexModel(leapModel,camera,light);
		end
	end
	
	if pickedObject then
		DrawComplexModel(pickModel,camera,light);
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

	if assetTicket == -1 then
		DrawComplexModel(skyModel,camera);
	end

	--debugText = string.format("%s Leap: %2.2fms", frameTimeText, leapFrameTime * 1000);
	DrawText(debugFont,frameTimeText,0,0,0);
end

function End()
end
