
Require("ProjectDir","../../Common/IngenUtils.lua");
Require("ProjectDir","../../Common/Easing.lua","Easing");
Require("ProjectDir","../../Common/LuaGen.lua","LG");
Require("ProjectDir","Marionette3.lua");
Require("ProjectDir","GpuParticleSystemCurl.lua","GPUPS");

-- Okay, how to do the choreography...
-- 1. More than one tweened camera movement should be possible per shot
-- 2. Should be able to jump to any specific shot through the console
-- 3. Should be able to cue actions within a shot using one or more 'magic buttons'
-- 4. Should be able to trigger restrings, special effects, physics properties etc.

-- Functions?

function ResetPerformanceCamera()
	SetCameraPosition(performanceCamera,-0.5,0.5,2);
    SetCameraTarget(performanceCamera,0,-0.5,0);
    SetCameraClipFov(performanceCamera,0.01,200,0.78539);
end

setupCS1 = true;
function CS1(progress)
	if setupCS1 then
		PlaySound(waterLily);
		SetLightColor(light,0,0,0);
		setupCS1 = false;
	end
	
	local cameraPos1 = CreateVector(-0.21, 0.05,-0.55, 1);
	local cameraTgt1 = CreateVector(-0.46, 0.14,-1.48, 1);
	local cameraPos2 = CreateVector( 0.47,-0.09, 0.95, 1);
	local cameraTgt2 = CreateVector(-0.32,-0.32,-0.69, 1);
	
	if progress >= 12.5 then Cutscene(csPanAcrossTable) end
	
	local cameraPos = Easing.linear(progress, cameraPos1, cameraPos2 - cameraPos1, 12.2);
	local cameraTgt = Easing.linear(progress, cameraTgt1, cameraTgt2 - cameraTgt1, 12.2);
	
	SetCameraPosition(performanceCamera, cameraPos.x, cameraPos.y, cameraPos.z);
	SetCameraTarget(performanceCamera, cameraTgt.x, cameraTgt.y, cameraTgt.z);
	
	if progress > 4 then
		local lightColor = Easing.linear(progress-4, 0, 1, 8.2);
		SetLightColor(light, lightColor,lightColor,lightColor);
	end
end

function csPanAcrossTable(progress)
	cameraPos1 = CreateVector(0.911497669614,-0.087275455240438,0.83127248782394, 1);
	cameraTgt1 = CreateVector(0.70341969155019,-0.364396330297,-0.12952861900218, 1);
	cameraPos2 = CreateVector(-2.2794403279456,-0.21917106436887,0.62660954712759, 1);   
	cameraTgt2 = CreateVector(-1.8509603484754,-0.4088118956667,-0.25681631976669, 1);   

	if progress >= 10 then Cutscene(csDownFromSky) end
	
	local cameraPos = Easing.linear(progress, cameraPos1, cameraPos2 - cameraPos1, 10);
	local cameraTgt = Easing.linear(progress, cameraTgt1, cameraTgt2 - cameraTgt1, 10);
	
	SetCameraPosition(performanceCamera, cameraPos.x, cameraPos.y, cameraPos.z);
	SetCameraTarget(performanceCamera, cameraTgt.x, cameraTgt.y, cameraTgt.z);
end

function csDownFromSky(progress)
	cameraPos1 = CreateVector(1.5827534542447,1.214914791749,0.84395068446955,1)   
	cameraTgt1 = CreateVector(1.2603276908486,0.43734207299804,0.30411525199706,1)   
	cameraPos2 = CreateVector(-1.5789526447216,-0.89242552215229,0.72446486428968,1)   
	cameraTgt2 = CreateVector(-1.37401389805484,-1.0228492308904,-0.21987705600876,1)

	if progress >= 12 then progress = 12 end
	
	local cameraPos = Easing.linear(progress, cameraPos1, cameraPos2 - cameraPos1, 12);
	local cameraTgt = Easing.linear(progress, cameraTgt1, cameraTgt2 - cameraTgt1, 12);
	
	SetCameraPosition(performanceCamera, cameraPos.x, cameraPos.y, cameraPos.z);
	SetCameraTarget(performanceCamera, cameraTgt.x, cameraTgt.y, cameraTgt.z);
	SetCameraClipFov(performanceCamera, 0.01, 20, 0.985);
end

function csLookAroundRoom(progress)
	
end

function csRevealCar(progress)
	drawCar = true;
end

function Cutscene(cutsceneFunc)
	cutsceneTimer = 0;
	cutsceneFunction = cutsceneFunc;
end

function UpdateLeapHand()
	for i = 1,GetLeapNumBones(leapHelper) do
		visible, length, radius = GetLeapBoneDetails(leapHelper, i-1);
		leapVisibilities[i] = visible;
		if visible then
			if leapModels[i] == nil then
				local vtx, idx = CreateCapsule(length / (radius * 2.0));
				local boneModel = CreateModel("PosNor",vtx,idx);
				SetMeshScale(boneModel, 0, radius * 2.0);
				leapModels[i] = boneModel;
				print("Created Leap Model " .. i-1);
			end
			local leapBoneMatrix = GetLeapBoneMatrix(leapHelper, i-1);
			SetModelMatrix(leapModels[i], leapBoneMatrix);
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
		{"FrameworkDir","NoiseReveal.xml","Shader","noiseReveal"},
		
		{"ProjectDir","Stars2.dds","CubeMap","skymap"},
		
		{"ProjectDir","stonefloor.bmp","Texture","floortex"},
		{"ProjectDir","Noise2D.dds","Texture","noiseTex"},
		
		{"ProjectDir","Models/room/roomModel.inm","IngenuityModel","roomModel"},
		{"ProjectDir","Models/tools/toolsModel.inm","IngenuityModel","toolsModel"},
		{"ProjectDir","Models/car/carModel2.inm","IngenuityModel","carModel"},
		
		{"ProjectDir","Music/Water Lily.wav","WavAudio","waterLily"}
	);

	camera = CreateCamera();
	SetCameraClipFov(camera,0.01,200,0.78539);
	SetupFlyCamera(camera, 0, 0,-2, 0.01, 1);

	floorModel = CreateModel("PosNorTex",CreateCube());
	SetModelScale(floorModel,1.0,0.5,1.0);
	SetModelPosition(floorModel,0,-1,-0.5);
	--SetMeshColor(floorModel,0,0.0,1.0,0.0);

	skyModel = CreateSkyCube();

	physicsWorld = CreatePhysicsWorld();

	physicsFloor = CreatePhysicsCuboid(2,1,2,false);

	AddToPhysicsWorld(physicsWorld,physicsFloor,true);

	SetPhysicsPosition(physicsFloor,0,-1,-0.5);
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
	ResetPerformanceCamera();
	
	spriteCamera = CreateSpriteCamera(true,false,true);
	
	performanceSurface = CreateSurface(1.0,1.0,performanceWindow);
	
	performanceQuad = CreateSpriteModel(GetSurfaceTexture(performanceSurface));
	
	cutsceneTimer = 0;
	
	drawCar = false;
end

function Update(delta)
	if IsLoaded(assetTicket) then
		skyEffect = CreateEffect("skyshader");
		shadowEffect = CreateEffect("shadowShader");
		revealEffect = CreateEffect("noiseReveal");
		skyMap = GetAsset("skymap");
		floorTex = GetAsset("floortex");
		noiseTex = GetAsset("noiseTex");
		roomModel = GetAsset("roomModel");
		toolsModel = GetAsset("toolsModel");
		carModel = GetAsset("carModel");
		waterLily = GetAsset("waterLily");

		if skyEffect then
			SetMeshCubeMap(skyModel,0,skyMap);
			SetMeshEffect(skyModel,0,skyEffect);
		end
		
		if carModel then
			SetModelScale(carModel,0.3)
			SetModelPosition(carModel,0,-0.6,0)
			SetModelRotation(carModel,0,-PI_2,0)
		end
		
		if revealEffect then
			SetEffectParam(revealEffect,0,noiseTex);
		end

		SetMeshTexture(floorModel,0,floorTex);

		assetTicket = -1;
	end
	
	UpdateLeapHand();
	
	UpdatePhysicsWorld(physicsWorld,delta);

	UpdateMarionette(delta);
	
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
			newWidth, newHeight = GetMonitorSize(performanceWindow);
			print("Resizing window to " .. newWidth .. ", " .. newHeight);
			SetWindowProps(performanceWindow, newWidth, newHeight, true);
			currentlyFullscreen = true;
		end
	end
	
	if roomModel and toolsModel then
		SetModelPosition(roomModel,0,-1.59,-0.95);
		SetModelPosition(toolsModel,0,-1.59,-0.95);
		SetModelScale(roomModel,0.285);
		SetModelScale(toolsModel,0.285);
	end
	
	ResetPerformanceCamera();
	
	lightPosX = -math.sin(2.5) * 1.2;
	lightPosY = 1.0;
	lightPosZ = -math.cos(2.5) * 1.2;
	
	SetLightPosition(light,lightPosX,lightPosY,lightPosZ)
	SetLightDirection(light,-lightPosX,-lightPosY,-lightPosZ);
	SetCameraPosition(shadowCamera,lightPosX,lightPosY,lightPosZ);
	SetCameraClipFov(shadowCamera,0.01,20,1.185);
	
	UpdatePixelCamera(spriteCamera,nil,false,true,1,5000);
	
	cutsceneTimer = cutsceneTimer + delta;
	if cutsceneFunction then
		cutsceneFunction(cutsceneTimer);
	end
end

function PrintCamera()
	local dirX = (math.sin(flyCamYAngle) * math.cos(flyCamUpAngle));
	local dirY = (math.sin(flyCamUpAngle));
	local dirZ = (math.cos(flyCamYAngle) * math.cos(flyCamUpAngle));
	
	print(LG.Assign(LG.Call("CreateVector",flyCamX,flyCamY,flyCamZ,1),"cameraPos"));
	print(LG.Assign(LG.Call("CreateVector",flyCamX+dirX,flyCamY+dirY,flyCamZ+dirZ,1),"cameraTgt"));
end

function SyncCameras()
	local dirX = (math.sin(flyCamYAngle) * math.cos(flyCamUpAngle));
	local dirY = (math.sin(flyCamUpAngle));
	local dirZ = (math.cos(flyCamYAngle) * math.cos(flyCamUpAngle));
	
	SetCameraPosition(performanceCamera,flyCamX,flyCamY,flyCamZ);
	SetCameraTarget(performanceCamera,flyCamX+dirX,flyCamY+dirY,flyCamZ+dirZ);
end

function Draw()
	
	-- Draw all objects to the main window
	
	DrawComplexModel(floorModel,camera,light);
	
	if roomModel and toolsModel then
		--DrawComplexModel(roomModel,camera,light,nil,nil,shadowEffect);
		--DrawComplexModel(toolsModel,camera,light,nil,nil,shadowEffect);
	end
	
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
			local cameraMatrix = GetCameraProjMatrix(shadowCamera,shadowSurface,true) * GetCameraViewMatrix(shadowCamera);
			cameraMatrixFloats = CreateFloatArray(cameraMatrix);
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
		
		if drawCar then
			DrawComplexModel(carModel,performanceCamera,light,performanceSurface,nil);
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
