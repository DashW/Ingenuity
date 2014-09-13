
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

function CreateSkyCube()
	local vtx = {
		{-1,-1,-1}, {-1, 1,-1}, { 1,-1,-1}, { 1, 1,-1},
		{-1,-1, 1}, {-1,-1,-1}, { 1,-1, 1}, { 1,-1,-1},
		{-1, 1,-1}, {-1, 1, 1}, { 1, 1,-1}, { 1, 1, 1},
		{ 1,-1, 1}, { 1, 1, 1}, {-1,-1, 1}, {-1, 1, 1},
		{-1,-1, 1}, {-1, 1, 1}, {-1,-1,-1}, {-1, 1,-1},
		{ 1,-1,-1}, { 1, 1,-1}, { 1,-1, 1}, { 1, 1, 1}
	};
	local idx = {
		 0, 1, 2,   1, 3, 2,   4, 5, 6,   5, 7, 6,
		 8, 9,10,   9,11,10,  12,13,14,  13,15,14,
		16,17,18,  17,19,18,  20,21,22,  21,23,22
	};
	local skyCube = CreateModel("Pos",vtx,idx);
	SetModelScale(skyCube,50);
	return skyCube;
end

function Begin()
	assetTicket = LoadAssets(
		{"FrameworkDir","SkyShader.xml","Shader","skyshader"},
		--{"FrameworkDir","MultiTextureAnimY.xml","Shader","landshader"},
		{"ProjectDir","grassCubeMap.dds","CubeMap","skymap"},
		{"ProjectDir","crate.jpg","Texture","cratetex"},
		{"ProjectDir","stonefloor.bmp","Texture","floortex"}
		--{"ProjectDir","vase.obj","WavefrontModel","vasemodel"}
		--{"ProjectDir","hmblend.dds","Tex2D","blendtex"},
		--{"ProjectDir","dirt.dds","Tex2D","dirttex"},
		--{"ProjectDir","stone.dds","Tex2D","stonetex"},
		--{"ProjectDir","grass.dds","Tex2D","grasstex"},
		--{"ProjectDir","hmheight.raw","RawHeightMap","hmheight"}
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
		
		print("Adding Ragdoll Bone Number "..numRagdollBones);
		AddPhysicsRagdollBone(physicsRagdoll,currentBone,definition[BONE_PARENT],
			definition[JOINT_CONE],definition[JOINT_MIN],definition[JOINT_MAX],
			definition[CHILD_PITCH],definition[CHILD_YAW],definition[CHILD_ROLL],
			definition[PAREN_PITCH],definition[PAREN_YAW],definition[PAREN_ROLL]);
		numRagdollBones = numRagdollBones + 1;
		ragdollBones[i] = currentBone;
	end
	
	FinalizePhysicsRagdoll(physicsRagdoll);
	
	-- END PHYSICS RAGDOLL --

	debugFont = GetFont(40,"Arial");
	
	light = CreateLight("directional");
	SetLightDirection(light,math.sin(2.5),0.75,math.cos(2.5));
end

function Update(delta)
	if IsLoaded(assetTicket) then
		skyEffect = CreateEffect("skyshader");
		skyMap = GetAsset("skymap");
		crateTex = GetAsset("cratetex");
		floorTex = GetAsset("floortex");

		--vaseModel = GetAsset("vasemodel");
		--SetModelScale(vaseModel,0.02);

		--physicsVase = CreatePhysicsMesh(GetWavefrontMesh("vasemodel",0));
		--AddToPhysicsWorld(physicsWorld,physicsVase,false);
		--SetPhysicsScale(physicsVase,0.02);
		--SetPhysicsPosition(physicsVase,2,2,0);

		--landEffect = CreateEffect("landshader");
		--SetEffectParam(landEffect,0,GetAsset("grasstex"));  --tex1
		--SetEffectParam(landEffect,1,GetAsset("dirttex")); --tex2
		--SetEffectParam(landEffect,2,GetAsset("stonetex")); --tex3
		--SetEffectParam(landEffect,3,0.0);               --yStart
		--SetEffectParam(landEffect,4,1.0);               --yProgress

		--heightmap = GetAsset("hmheight");
		--SetHeightmapScale(heightmap,30,0.04,30);

		--landModel = GetHeightmapModel(heightmap);
		--SetMeshTexture(landModel,0,GetAsset("blendtex"));
		--SetMeshEffect(landModel,0,landEffect);
		--SetModelPosition(landModel,0,-8,0);

		--physicsLand = CreatePhysicsHeightmap(heightmap);
		--AddToPhysicsWorld(physicsWorld,physicsLand,true);
		--SetPhysicsPosition(physicsLand,0,-8,0);

		if skyEffect then
			SetMeshCubeMap(skyModel,0,skyMap);
			SetMeshEffect(skyModel,0,skyEffect);
		end

		SetMeshTexture(cubeModel,0,crateTex);
		SetMeshTexture(floorModel,0,floorTex);

		assetTicket = -1;
	end

	UpdatePhysicsWorld(physicsWorld,delta);
	for i,bone in pairs(ragdollBones) do
		SyncPhysicsMatrix(bone,boneModels[i]);
		SyncPhysicsMatrix(bone,debugModels[i]);
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

	UpdateFlyCamera(delta);
	UpdateFrameTime(delta);
end

function Draw()
	DrawComplexModel(floorModel,camera);
	--DrawComplexModel(cubeModel,camera);

	for i,model in pairs(boneModels) do
		DrawComplexModel(model,camera,light);
		--DrawComplexModel(debugModels[i],camera);
	end

	if assetTicket == -1 then
		--DrawComplexModel(vaseModel,camera);
		--DrawComplexModel(landModel,camera);
		DrawComplexModel(skyModel,camera);
	end

	--DrawText(debugFont,frameTimeText,0,0,0);
end

function End()
end
