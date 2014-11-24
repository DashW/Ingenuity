
Require("ProjectDir","../../Common/IngenUtils.lua");

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
	--SetPhysicsRotation(physicsCube,0,0,1);
	
	SetPhysicsPosition(physicsCapsule, 2, 4, 2);
	SetPhysicsRotation(physicsCapsule, 0, 0, 1);
	
	physicsAnchor = CreatePhysicsAnchor();
	AddToPhysicsWorld(physicsWorld,physicsAnchor);
	SetPhysicsPosition(physicsAnchor, 0, 10, 0);
	physicsSpring = CreatePhysicsSpring(physicsAnchor, physicsCube, CreateVector(0,0,0,1), CreateVector(1,-1,0,1));
	physicsSpring.stiffness = 10;
	physicsSpring.damping = 0;
	physicsSpring.length = 2;
	
	springModel = CreateModel("PosNor",CreateCylinder(1));
	SetModelScale(springModel,0.1);
	SetMeshColor(springModel,0,0,0,0);
		
	dragModel = CreateModel("PosNor",CreateCylinder(1));
	SetModelScale(dragModel,0.1);
	SetMeshColor(dragModel,0,0,0,0);

	pickModel = CreateModel("PosNor",CreateSphere());
	SetModelScale(pickModel,0.1);
	SetMeshColor(pickModel,0,1,0,0);
	
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
	
	cubeMatrix = GetPhysicsMatrix(physicsCube);
	SetMeshMatrix(cubeModel,0,cubeMatrix);
	
	springCubePoint = cubeMatrix * CreateVector(1,-1,0,1);
	StretchModelBetween(springModel, 0.1, springCubePoint.x, springCubePoint.y, springCubePoint.z, 0, 10, 0);
	
	--SyncPhysicsMatrix(physicsCapsule,capsuleModel);
	if assetTicket == -1 then
		SetMeshMatrix(vaseModel,0,GetPhysicsMatrix(physicsVase));
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
	
	UpdateFlyCamera(delta, dragSpring);
	UpdateFrameTime(delta);
	
	local sWidth, sHeight = GetScreenSize();
	local mouseX, mouseY = GetMousePosition();
	local leftDown, leftPressed, leftReleased = GetMouseLeft();
	
	pickRay = GetCameraRay(camera,mouseX/sWidth,mouseY/sHeight);
	cameraPos = CreateVector(flyCamX,flyCamY,flyCamZ,1);
	
	pickedObject, pickedPos, pickedNormal, pickedDistance = PickPhysicsObject(physicsWorld,
		CreateVector(flyCamX,flyCamY,flyCamZ,1), pickRay);
	
	if pickedObject then
		SetModelPosition(pickModel,pickedPos.x,pickedPos.y,pickedPos.z);
		
		if leftPressed then
			dragObject = pickedObject;
			dragDistance = pickedDistance * 1.05;
			dragPoint = InvMatrix(GetPhysicsMatrix(dragObject)) * pickedPos;
			
			if not dragAnchor then
				dragAnchor = CreatePhysicsAnchor();
			end
			
			AddToPhysicsWorld(physicsWorld, dragAnchor);
			SetPhysicsPosition(dragAnchor, pickedPos.x, pickedPos.y, pickedPos.z);
			
			dragSpring = CreatePhysicsSpring(dragObject, dragAnchor, dragPoint, CreateVector(0,0,0,0));
			dragSpring.stiffness = 100;
			dragSpring.damping = 50;
		end
	end
	
	if dragSpring then
		local dragAnchorPos = cameraPos + (pickRay * dragDistance);
		SetPhysicsPosition(dragAnchor, dragAnchorPos.x, dragAnchorPos.y, dragAnchorPos.z);
		
		local dragAttachPos = GetPhysicsMatrix(dragObject) * dragPoint;
		
		StretchModelBetween(dragModel, 0.1,
			dragAttachPos.x, dragAttachPos.y, dragAttachPos.z,
			dragAnchorPos.x, dragAnchorPos.y, dragAnchorPos.z);
		
		if leftReleased then
			dragSpring = nil;
			dragObject = nil
		end
	end
end

function Draw()
	DrawComplexModel(floorModel,camera);
	DrawComplexModel(cubeModel,camera);
	DrawComplexModel(springModel,camera);
	--DrawComplexModel(capsuleModel,camera);
	
	if assetTicket == -1 then
		DrawComplexModel(vaseModel,camera);
		--DrawComplexModel(landModel,camera);
		DrawComplexModel(skyModel,camera);
	end
	
	if pickedObject then
		DrawComplexModel(pickModel,camera);
	end
	
	if dragSpring then
		DrawComplexModel(dragModel,camera);
	end
	
	DrawText(debugFont,frameTimeText,0,0,0);
end

function End()
end
