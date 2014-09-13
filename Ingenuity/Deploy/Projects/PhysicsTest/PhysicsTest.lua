
Require("ProjectDir","../../Common/IngenUtils.lua");

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
		{"FrameworkDir","MultiTextureAnimY.xml","Shader","landshader"},
		{"ProjectDir","grassCubeMap.dds","CubeMap","skymap"},
		{"ProjectDir","crate.jpg","Texture","cratetex"},
		{"ProjectDir","stonefloor.bmp","Texture","floortex"},
		{"ProjectDir","vase.obj","WavefrontModel","vasemodel"},
		{"ProjectDir","hmblend.dds","Tex2D","blendtex"},
		{"ProjectDir","dirt.dds","Tex2D","dirttex"},
		{"ProjectDir","stone.dds","Tex2D","stonetex"},
		{"ProjectDir","grass.dds","Tex2D","grasstex"},
		{"ProjectDir","hmheight.raw","RawHeightMap","hmheight"}
	);
	
	camera = CreateCamera();
	SetCameraClipFov(camera,0.01,200,0.78539);
	SetupFlyCamera(camera, 0,2,-10, 0.01,10);
	
	cubeModel = CreateModel("PosNorTex",CreateCube());
	--SetMeshColor(cubeModel,0,1.0,0.0,0.0);
	--SetModelPosition(cubeModel,0.0,4.0,0.0);
	--SetModelRotation(cubeModel,0.0,0.0,1.0);
	
	floorModel = CreateModel("PosNorTex",CreateCube());
	SetModelScale(floorModel,5,0.5,5);
	--SetMeshColor(floorModel,0,0.0,1.0,0.0);
	
	capsuleModel = CreateModel("PosNor",CreateCapsule(2.0));
	SetModelScale(capsuleModel, 1.0);
	--SetModelRotation(capsuleModel, 0, math.pi/2, 0);
	
	skyModel = CreateSkyCube();
	
	physicsWorld = CreatePhysicsWorld();
	
	physicsCube = CreatePhysicsCuboid(2,2,2,false);
	physicsFloor = CreatePhysicsCuboid(10,1,10,false);
	
	woodMaterial = CreatePhysicsMaterial(0.63,0.95,0.7,0.32);
	SetPhysicsMaterial(physicsCube, woodMaterial);
	
	AddToPhysicsWorld(physicsWorld,physicsCube,false);
	AddToPhysicsWorld(physicsWorld,physicsFloor,true);
	
	physicsCapsule = CreatePhysicsCapsule(0.5, 2.0, false);
	AddToPhysicsWorld(physicsWorld,physicsCapsule,false);
	
	SetPhysicsPosition(physicsCube,0,4,0);
	SetPhysicsRotation(physicsCube,0,0,1);
	
	SetPhysicsPosition(physicsCapsule, 2, 4, 2);
	SetPhysicsRotation(physicsCapsule, 0, 0, 1);
	
	debugFont = GetFont(40,"Arial");
end

function Update(delta)
	if IsLoaded(assetTicket) then
		skyEffect = CreateEffect("skyshader");
		skyMap = GetAsset("skymap");
		crateTex = GetAsset("cratetex");
		floorTex = GetAsset("floortex");
		
		vaseModel = GetAsset("vasemodel");
		SetModelScale(vaseModel,0.02);
		
		physicsVase = CreatePhysicsMesh(GetWavefrontMesh("vasemodel",0));
		AddToPhysicsWorld(physicsWorld,physicsVase,false);
		SetPhysicsScale(physicsVase,0.02);
		SetPhysicsPosition(physicsVase,2,2,0);
		
		landEffect = CreateEffect("landshader");
		SetEffectParam(landEffect,0,GetAsset("grasstex"));  --tex1
		SetEffectParam(landEffect,1,GetAsset("dirttex")); --tex2
		SetEffectParam(landEffect,2,GetAsset("stonetex")); --tex3
		SetEffectParam(landEffect,3,0.0);               --yStart
		SetEffectParam(landEffect,4,1.0);               --yProgress
		
		heightmap = GetAsset("hmheight");
		SetHeightmapScale(heightmap,30,0.04,30);
		
		landModel = GetHeightmapModel(heightmap);
		SetMeshTexture(landModel,0,GetAsset("blendtex"));
		SetMeshEffect(landModel,0,landEffect);
		SetModelPosition(landModel,0,-8,0);
		
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
	
	SyncPhysicsMatrix(physicsCube,cubeModel);
	--SyncPhysicsMatrix(physicsCapsule,capsuleModel);
	if assetTicket == -1 then
		SyncPhysicsMatrix(physicsVase,vaseModel);
	end
	
	local down,pressed,released = GetKeyState('r');
	if pressed then
		SetPhysicsPosition(physicsCube,0,4,0);
		SetPhysicsRotation(physicsCube,0,0,1);
		
		SetPhysicsPosition(physicsVase,0,8,0);
		SetPhysicsRotation(physicsVase,0,0,0);
	
		--SetPhysicsPosition(physicsCapsule, 2, 4, 2);
		--SetPhysicsRotation(physicsCapsule, 0, 0, 0);
	end
	
	UpdateFlyCamera(delta);
	UpdateFrameTime(delta);
end

function Draw()
	DrawComplexModel(floorModel,camera);
	DrawComplexModel(cubeModel,camera);
	--DrawComplexModel(capsuleModel,camera);
	
	if assetTicket == -1 then
		DrawComplexModel(vaseModel,camera);
		--DrawComplexModel(landModel,camera);
		DrawComplexModel(skyModel,camera);
	end
	
	DrawText(debugFont,frameTimeText,0,0,0);
end

function End()
end
