
Require("ProjectDir","../../Common/IngenUtils.lua");
Require("ProjectDir","Marionette2.lua");

function UpdateLeapHand()
	for i = 1,GetLeapNumBones(leapHelper) do
		visible, length, radius = GetLeapBoneDetails(leapHelper, i-1);
		leapVisibilities[i] = visible;
		if visible then
			if leapModels[i] == nil then
				--leapPhysicals[i] = CreatePhysicsCapsule(radius, length, false);
				--AddToPhysicsWorld(physicsWorld,leapPhysicals[i],true);
				
				local vtx, idx = CreateCapsule(length / (radius * 2.0));
				local boneModel = CreateModel("PosNor",vtx,idx);
				SetModelScale(boneModel, radius * 2.0);
				leapModels[i] = boneModel;
				print("Created Leap Model " .. i-1);
			end
			local leapBoneMatrix = GetLeapBoneMatrix(leapHelper, i-1);
			--SetPhysicsMatrix(leapPhysicals[i], leapBoneMatrix);
			SetMeshMatrix(leapModels[i], 0, leapBoneMatrix);
		end
	end
end

function DrawLeapHand()
	for i,leapModel in pairs(leapModels) do
		if leapModel and leapVisibilities[i] then
			DrawComplexModel(leapModel,camera,light);
		end
	end
end

function Begin()
	assetTicket = LoadAssets(
		{"FrameworkDir","SkyShader.xml","Shader","skyshader"},
		{"FrameworkDir","ShadowLit.xml","Shader","shadowShader"},
		{"ProjectDir","Stars2.dds","CubeMap","skymap"},
		{"ProjectDir","stonefloor.bmp","Texture","floortex"},
		{"ProjectDir","Modified Room/ModifiedRoom.obj","ColladaModel","roomModel"},
		{"ProjectDir","Modified Room/ModifiedTools.obj","ColladaModel","toolsModel"}
	);

	camera = CreateCamera();
	SetCameraClipFov(camera,0.01,200,0.78539);
	SetupFlyCamera(camera, 0, 0,-2, 0.01, 1);

	--cubeModel = CreateModel("PosNorTex",CreateCube());
	--SetMeshColor(cubeModel,0,1.0,0.0,0.0);
	--SetModelPosition(cubeModel,0.0,4.0,0.0);
	--SetModelRotation(cubeModel,0.0,0.0,1.0);

	floorModel = CreateModel("PosNorTex",CreateCube());
	SetModelScale(floorModel,5,0.5,5);
	SetModelPosition(floorModel,0,-1,0);
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

	SetPhysicsPosition(physicsFloor,0,-1,0);
	SetPhysicsRotation(physicsFloor,0,0,0);
	
	physicsRagdoll = CreateMarionette(physicsWorld);
	
	leapModels = {};
	leapVisibilities = {};
	--leapPhysicals = {};
	leapHelper = CreateLeapHelper();
	SetLeapPosition(leapHelper,0,-0.5,0);
	SetLeapScale(leapHelper,0.0025);
	
	pickModel = CreateModel("PosNor",CreateSphere());
	SetModelScale(pickModel,0.01);
	SetMeshColor(pickModel,0,1,0,0);

	debugFont = GetFont(40,"Arial");
	SetFontColor(debugFont,1,1,1);
	
	lightPosX = -math.sin(2.5);
	lightPosY = 0.75;
	lightPosZ = -math.cos(2.5);
	
	light = CreateLight("spot");
	SetLightPosition(light,lightPosX,lightPosY,lightPosZ)
	SetLightDirection(light,-lightPosX,-lightPosY,-lightPosZ);
	SetLightSpotPower(light,12);
	
	shadowSurface = CreateSurface(2048,2048,false,"typeless");
	
	shadowCamera = CreateCamera();
	SetCameraPosition(shadowCamera,lightPosX,lightPosY,lightPosZ);
	SetCameraClipFov(shadowCamera,0.01,20,0.785);
	
	performanceWindow = CreateWindow();
	--SetWindowProps(performanceWindow,nil,nil,true); -- Set performance window undecorated!
	
	performanceCamera = CreateCamera();
	SetCameraPosition(performanceCamera,0,-2,4);
	SetCameraTarget(performanceCamera,0,-2.5,0);
	SetCameraClipFov(performanceCamera,0.01,200,0.78539);
	
	spriteCamera = CreateSpriteCamera(true,false,true);
	
	performanceSurface = CreateSurface(1.0,1.0,performanceWindow);
	
	performanceQuad = CreateSpriteModel(GetSurfaceTexture(performanceSurface));
end

function Update(delta)
	if IsLoaded(assetTicket) then
		skyEffect = CreateEffect("skyshader");
		shadowEffect = CreateEffect("shadowShader");
		skyMap = GetAsset("skymap");
		floorTex = GetAsset("floortex");
		roomModel = GetAsset("roomModel");
		toolsModel = GetAsset("toolsModel");

		if skyEffect then
			SetMeshCubeMap(skyModel,0,skyMap);
			SetMeshEffect(skyModel,0,skyEffect);
		end

		SetMeshTexture(floorModel,0,floorTex);

		assetTicket = -1;
	end
	
	UpdateLeapHand();
	
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

	UpdateMarionette();
	
	--down,pressed,released = GetMouseLeft();
	--if pickedObject and down then
	--	DragPhysicsObject(physicsWorld,camera,x/sWidth,y/sHeight);
	--else
		UpdateFlyCamera(delta);
	--end
	
	local sWidth, sHeight = GetBackbufferSize();
	local mouseX, mouseY = GetMousePosition();
	
	--down,pressed,released = GetKeyState('p');
	--if pressed then
	pickedObject, pickedPos = PickPhysicsObject(physicsWorld,
		CreateVector(flyCamX,flyCamY,flyCamZ,1),
		GetCameraRay(camera,mouseX/sWidth,mouseY/sHeight));
	
	if pickedObject then
		SetModelPosition(pickModel,pickedPos.x,pickedPos.y,pickedPos.z);
	end
	--end
	
	UpdateFrameTime(delta);
	
	local fDown, fPressed, fReleased = GetKeyState('f');
	if fPressed then
		if currentlyFullscreen then
			SetWindowProps(performanceWindow, previousWidth, previousHeight, false);
			currentlyFullscreen = false;
		else
			previousWidth, previousHeight = GetBackbufferSize();
			newWidth, newHeight = GetDesktopSize(performanceWindow);
			print("Resizing window to " .. newWidth .. ", " .. newHeight);
			SetWindowProps(performanceWindow, newWidth, newHeight, true);
			currentlyFullscreen = true;
		end
	end
	
	if roomModel and toolsModel then
		SetModelPosition(roomModel,0,-2.04,-1.5);
		SetModelPosition(toolsModel,0,-2.04,-1.5);
		SetModelScale(roomModel,0.4);
		SetModelScale(toolsModel,0.4);
	end
	
	SetCameraPosition(performanceCamera,-0.5,0.5,2);
	SetCameraTarget(performanceCamera,0,-0.5,0);
	SetCameraClipFov(performanceCamera,0.01,200,0.78539);
	
	lightPosX = -math.sin(2.5) * 2.0;
	lightPosY = 1.7;
	lightPosZ = -math.cos(2.5) * 2.0;
	
	SetLightPosition(light,lightPosX,lightPosY,lightPosZ)
	SetLightDirection(light,-lightPosX,-lightPosY,-lightPosZ);
	SetCameraPosition(shadowCamera,lightPosX,lightPosY,lightPosZ);
	SetCameraClipFov(shadowCamera,0.01,20,1.185);
	
	UpdatePixelCamera(spriteCamera,nil,false,true,1,5000);
end

function Draw()
	
	-- Draw all objects to the main window
	
	DrawComplexModel(floorModel,camera,light);
	
	if pickedObject then
		DrawComplexModel(pickModel,camera,light);
	end
	
	DrawLeapHand();
	
	DrawMarionette(camera,nil,nil);
	
	DrawMarionetteWires(camera,nil,nil);
	
	-- This skybox looks absolute crap, I might have to ditch it...
	--DrawComplexModel(skyModel,camera,nil);
	
	-- Draw shadows to the shadow map
	
	ClearSurface(shadowSurface);
	if toolsModel then
		DrawComplexModel(roomModel,shadowCamera,nil,shadowSurface);
		DrawComplexModel(toolsModel,shadowCamera,nil,shadowSurface);
	end
	DrawMarionette(shadowCamera,shadowSurface,nil,false);
	
	-- Draw objects to the performance surface
	
	if performanceWindow then
		ClearSurface(performanceSurface);
		
		if shadowEffect then
			cameraMatrixFloats = CreateFloatArray(GetCameraMatrix(shadowCamera,shadowSurface,true));
			SetEffectParam(shadowEffect,0,cameraMatrixFloats);
			SetEffectParam(shadowEffect,1,GetSurfaceTexture(shadowSurface));
			SetEffectParam(shadowEffect,2,0.00005);
		end
		
		--DrawComplexModel(floorModel,performanceCamera,light,performanceSurface,nil,shadowEffect);
		
		if roomModel and toolsModel then
			DrawComplexModel(roomModel,performanceCamera,light,performanceSurface,nil,shadowEffect);
			DrawComplexModel(toolsModel,performanceCamera,light,performanceSurface,nil,shadowEffect);
		end
		
		DrawMarionette(performanceCamera,performanceSurface,shadowEffect);
	
		if skyEffect then
			DrawComplexModel(skyModel,performanceCamera,nil,performanceSurface);
		end
	end
	
	-- Draw the performance surface to both windows
	
	local performanceWindowSurface = GetWindowSurface(performanceWindow);
	if performanceWindowSurface then
		DrawSurface(performanceSurface,nil,performanceWindowSurface);
	end
	
	local performanceWidth, performanceHeight = GetBackbufferSize(performanceWindow);
	local mainWidth, mainHeight = GetBackbufferSize();
	SetMeshTexture(performanceQuad,0,GetSurfaceTexture(performanceSurface));
	local quadWidth = (performanceWidth/performanceHeight) * 250;
	SetMeshScale(performanceQuad,0, quadWidth, 250, 1);
	SetModelPosition(performanceQuad,mainWidth - quadWidth,0,0);
	DrawComplexModel(performanceQuad,spriteCamera);

	--debugText = string.format("%s Leap: %2.2fms", frameTimeText, leapFrameTime * 1000);
	DrawText(debugFont,frameTimeText,0,0,0);
	
end

function End()
end
