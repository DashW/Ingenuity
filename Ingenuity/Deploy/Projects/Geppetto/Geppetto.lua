
Require("ProjectDir","../../Common/IngenUtils.lua");
Require("ProjectDir","../../Common/Easing.lua","Easing");
Require("ProjectDir","../../Common/LuaGen.lua","LG");
Require("ProjectDir","Marionette4.lua");
Require("ProjectDir","Cutscenes.lua");
Require("ProjectDir","GpuParticleSystemCurl.lua","GPUPS");

PROFILING = false;

function store(name)
	return function(value)
		_G[name] = value;
	end
end

function ResetPerformanceCamera()
	SetCameraPosition(performanceCamera,-0.5,0.5,2);
    SetCameraTarget(performanceCamera,0,-0.5,0);
    SetCameraClipFov(performanceCamera,0.01,200,0.78539);
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
			local offsetMatrix = CreateMatrix();
			offsetMatrix[3] = CreateVector(0,0.3,0,1);
			local leapBoneMatrix = armMotionMatrix * offsetMatrix * GetLeapBoneMatrix(leapHelper, i-1);
			SetModelMatrix(leapModels[i], leapBoneMatrix);
		end
	end
end

function DrawLeapHand(camera,lights,surface)
	for i,leapModel in pairs(leapModels) do
		if leapModel and leapVisibilities[i] then
			DrawComplexModel(leapModel,camera,lights,surface);
		end
	end
end

function AddDominoes()
	for i=1,8 do
		physicsDominoes[i] = CreatePhysicsCuboid(0.04,0.1,0.01,false);
		SetPhysicsMass(physicsDominoes[i],0.04);
		AddToPhysicsWorld(physicsWorld,physicsDominoes[i],false);
		SetPhysicsPosition(physicsDominoes[i], 0.3 + (0.35 * math.cos(PI_2 * ((i-1)/7))), -0.45, (0.35 * math.sin(PI_2 * ((i-1)/7))));
		SetPhysicsRotation(physicsDominoes[i], 0, -PI_2 * ((i-1)/7), 0);
		--SetPhysicsRotation(physicsBridgePieces[i], -math.sin(2 * PI_1 * ((i-0.5)/8)) * 0.53, PI_4, 0);
		--SetPhysicsPosition(physicsBridgePieces[i], 0.35 + (0.1 * i), -0.36 + (0.1 * (-math.sin(PI_2 + (2 * PI_1 * ((i-0.5)/8))))), -0.75 + (0.1 * i))
	end
end

function RemoveDominoes()
	for i=1,8 do
		RemoveFromPhysicsWorld(physicsWorld,physicsDominoes[i]);
	end
	physicsDominoes = {};
end

function AddBall()
	AddToPhysicsWorld(physicsWorld,physicsBall,false);
	SetPhysicsPosition(physicsBall, 0.18, -0.46, 0.35);
	drawBall = true;
end

function RemoveBall()
	RemoveFromPhysicsWorld(physicsWorld,physicsBall);
	drawBall = false;
end

function Begin()
	
	SetClearColor(0.2,0.2,0.2);
	
	assetTicket = LoadAssets(
		{"FrameworkDir","SkyShader.xml","Shader","skyshader"},
		{"FrameworkDir","ShadowLit.xml","Shader","shadowShader"},
		{"FrameworkDir","NoiseReveal.xml","Shader","noiseReveal"},
		
		{"ProjectDir","Stars2.dds","CubeMap","skymap"},
		
		{"ProjectDir","stonefloor.bmp","Texture","floortex"},
		{"ProjectDir","Noise2D.dds","Texture","noiseTex"},
		
		{"ProjectDir","Models/room/roomModel2.inm","IngenuityModel","roomModel"},
		{"ProjectDir","Models/tools/toolsModel3.inm","IngenuityModel","toolsModel"},
		{"ProjectDir","Models/car/carModel2.inm","IngenuityModel","carModel"},
		{"ProjectDir","Models/domino/dominoModel.dae","ColladaModel","dominoModel"},
		{"ProjectDir","Models/ball/ballModel.dae","ColladaModel","ballModel"},
		
		{"ProjectDir","Music/Water Lily.wav","WavAudio","waterLily"},
		{"ProjectDir","Music/Dreams Become Real.wav","WavAudio","dreamsBecome"}
	);

	camera = CreateCamera();
	SetCameraClipFov(camera,0.01,200,0.78539);
	SetupFlyCamera(camera, 0, 0, 2, 0.01, 1);
	flyCamYAngle = math.pi;

	floorModel = CreateModel("PosNorTex",CreateCube(true));
	SetModelScale(floorModel, 2, 0.5, 0.75);
	SetModelPosition(floorModel,0,-1,-0.25);
	--SetMeshColor(floorModel,0,0.0,1.0,0.0);

	skyModel = CreateSkyCube();

	physicsWorld = CreatePhysicsWorld();
	SetPhysicsConstants(physicsWorld, 0, -10, 0, 0.05);
	SetPhysicsMaterialDefaults(physicsWorld, 0.7, 0.7);

	physicsFloor = CreatePhysicsCuboid(4, 1, 1.5, false);

	AddToPhysicsWorld(physicsWorld,physicsFloor,true);

	SetPhysicsPosition(physicsFloor,0,-1,-0.25);
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
	
	lights = {};
	
	lightPosX = -math.sin(2.5);
	lightPosY = 0.75;
	lightPosZ = -math.cos(2.5);
	
	spot = CreateLight("spot");
	SetLightPosition(spot,lightPosX,lightPosY,lightPosZ)
	SetLightDirection(spot,-lightPosX,-lightPosY,-lightPosZ);
	SetLightSpotPower(spot,12);
	table.insert(lights,spot);
	
	ambient = CreateLight("point");
	SetLightPosition(ambient,0,0,5);
	SetLightColor(ambient,0.04,0.04,0.06);
	--table.insert(lights,ambient);
	
	shadowSurface = CreateSurface(2048,2048,false,"typeless");
	
	shadowCamera = CreateCamera();
	SetCameraPosition(shadowCamera,lightPosX,lightPosY,lightPosZ);
	SetCameraClipFov(shadowCamera,0.01,20,0.785);
	
	performanceCamera = CreateCamera();
	ResetPerformanceCamera();
	
	spriteCamera = CreateSpriteCamera(true,false,true);
	
	performanceWidth, performanceHeight = 1280,720;
	
	performanceSurface = GetWindowSurface(GetMainWindow());
	controlSurface = CreateSurface(performanceWidth,performanceHeight,nil,"4x8iBGRA");
	spareSurface1 = CreateSurface(performanceWidth,performanceHeight);
	spareSurface2 = CreateSurface(performanceWidth,performanceHeight);
	
	performanceQuad = CreateSpriteModel(GetSurfaceTexture(controlSurface));

	quadHeight = 250;
	local mainWidth, mainHeight = GetBackbufferSize();
	SetMeshTexture(performanceQuad,0,GetSurfaceTexture(controlSurface));
	local quadWidth = (performanceWidth/performanceHeight) * quadHeight;
	SetMeshScale(performanceQuad,0, quadWidth, quadHeight, 1);
	SetModelPosition(performanceQuad,mainWidth - quadWidth,0,0);
	
	performanceSpout = CreateSpoutSender("GeppettoPerformance",performanceWidth,performanceHeight);
	
	cutsceneTimer = 0;
	
	drawMarionette = true;
	drawRoom = true;
	drawTools = true;
	drawCar = false;
	drawBall = false;
	
	armMotionMatrix = CreateMatrix();
	
	-- Car Seat
	
	physicsSeatBase = CreatePhysicsCuboid(0.4, 0.12, 0.3, false);
	physicsSeatBack = CreatePhysicsCuboid(0.4, 0.1, 0.4, false);
	
	baseDebugModel = CreateModel("PosNor",CreateCube(false));
	SetModelScale(baseDebugModel,0.2,0.06,0.15);
	SetModelPosition(baseDebugModel,0,-0.44,-0.22);
	
	backDebugModel = CreateModel("PosNor",CreateCube(false));
	SetModelRotation(backDebugModel,1.4,0,0);
	SetModelScale(backDebugModel,0.2,0.05,0.2);
	SetModelPosition(backDebugModel,0,-0.3,-0.4);
	
	-- Dominoes
	
	dominoDebugModel = CreateModel("PosNor",CreateCube(false));
	SetMeshScale(dominoDebugModel,0,0.02,0.05,0.005);
	
	physicsDominoes = {};
	
	-- Ball
	
	ballDebugModel = CreateModel("PosNor",CreateSphere());
	SetMeshScale(ballDebugModel,0,0.08);
	
	physicsBall = CreatePhysicsSphere(0.04,false);
	SetPhysicsMass(physicsBall, 0.01);
	
end -- Begin()

function Update(delta)
	
	if PROFILING then BeginTimestamp("Update",true,false) end
	
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
		dominoModel = GetAsset("dominoModel");
		ballModel = GetAsset("ballModel");
		waterLily = GetAsset("waterLily");
		
		if toolsModel then
			for i=1,GetNumMeshes(toolsModel) do
				SetMeshSpecular(toolsModel,i-1,12);
				SetMeshFactors(toolsModel,i-1,1,0.7);
			end
		end

		if skyEffect then
			SetMeshCubeMap(skyModel,0,skyMap);
			SetMeshEffect(skyModel,0,skyEffect);
		end
		
		if carModel then
			SetModelScale(carModel,0.37)
			SetModelPosition(carModel,0.4,-0.9,-0.7)
			SetModelRotation(carModel,0,-PI_2,0)
		end
		
		if dominoModel then
			for i=1,GetNumMeshes(dominoModel) do
				SetMeshScale(dominoModel,i-1,0.025);
				SetMeshRotation(dominoModel,i-1,PI_2,0,0);
			end
		end
		
		if ballModel then
			SetMeshScale(ballModel,0,0.04);
		end
		
		if revealEffect then
			SetEffectParam(revealEffect,0,noiseTex);
			SetEffectParam(revealEffect,5,-2);
			SetEffectParam(revealEffect,6,0);
			SetEffectParam(revealEffect,8,-1);
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
	
	down,pressed,released = GetKeyState(72); -- UP
	if down then
		local translateMatrix = CreateMatrix();
		translateMatrix[3] = CreateVector(0,0,0.15 * delta,1);
		armMotionMatrix = armMotionMatrix * translateMatrix;
	end
	down,pressed,released = GetKeyState(80); -- DOWN
	if down then
		local translateMatrix = CreateMatrix();
		translateMatrix[3] = CreateVector(0,0,-0.15 * delta,1);
		armMotionMatrix = armMotionMatrix * translateMatrix;
	end
	down,pressed,released = GetKeyState(75); -- LEFT
	if down then
		local palmX,palmY,palmZ = GetLeapBonePosition(leapHelper, 43);
		local offMatrix = CreateMatrix();
		offMatrix[3] = CreateVector(palmX, 0, palmZ, 1);
		local rotMatrix = RotMatrix(0,-delta,0);
		armMotionMatrix = armMotionMatrix * (offMatrix * rotMatrix * InvMatrix(offMatrix));
	end
	down,pressed,released = GetKeyState(77); -- RIGHT
	if down then
		local palmX,palmY,palmZ = GetLeapBonePosition(leapHelper, 43);
		local offMatrix = CreateMatrix();
		offMatrix[3] = CreateVector(palmX, 0, palmZ, 1);
		local rotMatrix = RotMatrix(0,delta,0);
		armMotionMatrix = armMotionMatrix * (offMatrix * rotMatrix * InvMatrix(offMatrix));
	end
	down,pressed,released = GetKeyState(' ');
	if down then
		triggered = true;
	end
	triggeredThisFrame = pressed;
	
	UpdateFrameTime(delta);
	
	if roomModel and toolsModel then
		SetModelPosition(roomModel,0,-1.59,-0.95);
		SetModelPosition(toolsModel,0,-1.59,-0.95);
		SetModelScale(roomModel,0.285);
		SetModelScale(toolsModel,0.285);
	end
	
	--ResetPerformanceCamera();
	
	lightPosX = -math.sin(2.5) * 1.2;
	lightPosY = 1.0;
	lightPosZ = -math.cos(2.5) * 1.2;
	
	SetLightPosition(spot,lightPosX,lightPosY,lightPosZ)
	SetLightDirection(spot,-lightPosX,-lightPosY,-lightPosZ);
	SetCameraPosition(shadowCamera,lightPosX,lightPosY,lightPosZ);
	SetCameraClipFov(shadowCamera,0.01,20,1.185);
	
	UpdatePixelCamera(spriteCamera,nil,false,true,1,5000);
	
	cutsceneTimer = cutsceneTimer + delta;
	if cutsceneFunction then
		cutsceneFunction(cutsceneTimer);
	end
	
	if PROFILING then EndTimestamp("Update",true,false) end
	
end -- Update()

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

-- Draw shadows to the shadow map
function DrawShadows()
	if PROFILING then BeginTimestamp("DrawShadows",false,true) end
	
	ClearSurface(shadowSurface);
	if toolsModel then
		if drawRoom then
			DrawComplexModel(roomModel,shadowCamera,nil,shadowSurface);
		end
		
		if drawTools then
			DrawComplexModel(toolsModel,shadowCamera,nil,shadowSurface);
		end
		
		if dominoModel then
			for i,domino in pairs(physicsDominoes) do
				SetModelMatrix(dominoModel,GetPhysicsMatrix(domino));
				DrawComplexModel(dominoModel,shadowCamera,nil,shadowSurface);
			end
		end
		
		if drawBall then
			SetModelMatrix(ballModel,GetPhysicsMatrix(physicsBall));
			DrawComplexModel(ballModel,shadowCamera,nil,shadowSurface);
		end
	end
	DrawMarionette(shadowCamera,nil,shadowSurface,nil);
	
	if PROFILING then EndTimestamp("DrawShadows",false,true) end
end

-- Draw objects to the performance surface
function DrawPerformance()
	if PROFILING then BeginTimestamp("DrawPerformance",false,true) end
	
	if performanceSurface then
		ClearSurface(performanceSurface,0,0,0);
		
		if shadowEffect then
			local cameraMatrix = GetCameraProjMatrix(shadowCamera,shadowSurface,true) * GetCameraViewMatrix(shadowCamera);
			cameraMatrixFloats = CreateFloatArray(cameraMatrix);
			SetEffectParam(shadowEffect,0,cameraMatrixFloats);
			SetEffectParam(shadowEffect,1,GetSurfaceTexture(shadowSurface));
			SetEffectParam(shadowEffect,2,0.00005);
		end
		
		--DrawComplexModel(floorModel,performanceCamera,lights,performanceSurface,nil,shadowEffect);
		
		if drawRoom and roomModel then
			DrawComplexModel(roomModel,performanceCamera,lights,performanceSurface,nil,shadowEffect);
		end
		
		if drawTools and toolsModel then
			DrawComplexModel(toolsModel,performanceCamera,lights,performanceSurface,nil,shadowEffect);
		end
		
		if dominoModel then
			for i,domino in pairs(physicsDominoes) do
				SetModelMatrix(dominoModel,GetPhysicsMatrix(domino));
				DrawComplexModel(dominoModel,performanceCamera,lights,performanceSurface,nil,shadowEffect);
			end
		end
		
		if drawBall then
			SetModelMatrix(ballModel,GetPhysicsMatrix(physicsBall));
			DrawComplexModel(ballModel,performanceCamera,lights,performanceSurface,nil,shadowEffect);
		end
		
		if drawMarionette then
			DrawMarionette(performanceCamera,lights,performanceSurface,shadowEffect);
		end
		
		if skyEffect then
			DrawComplexModel(skyModel,performanceCamera,nil,performanceSurface);
		end
		
		if drawCar and carModel then
			DrawComplexModel(carModel,performanceCamera,lights,performanceSurface,nil,revealEffect);
		end
	
		--if performanceSpout then
		--	SpoutSendTexture(performanceSpout,GetSurfaceTexture(performanceSurface));
		--end
	end
	
	if PROFILING then EndTimestamp("DrawPerformance",false,true) end
end

-- Draw objects to the main window
function DrawControls()
	if PROFILING then BeginTimestamp("DrawControl",false,true) end
	
	ClearSurface(controlSurface,0.2,0.2,0.2);
	
	-- Then, draw the control view
	
	--if roomModel and toolsModel then
		--DrawComplexModel(roomModel,camera,light,nil,nil,shadowEffect);
		--DrawComplexModel(toolsModel,camera,spot);
	--end
	
	if pickedObject then
		DrawComplexModel(pickModel,camera,spot,controlSurface);
	end
	
	DrawLeapHand(camera,spot,controlSurface);
	
	DrawMarionette(camera,spot,controlSurface);
	
	DrawMarionetteWires(camera,spot,controlSurface);
	
	if drawDebugSeat then
		DrawComplexModel(baseDebugModel,camera,spot,controlSurface);
		DrawComplexModel(backDebugModel,camera,spot,controlSurface);
	end
	
	for i,domino in pairs(physicsDominoes) do
		SetModelMatrix(dominoDebugModel,GetPhysicsMatrix(domino));
		--SetModelRotation(slideDebugModel, -math.sin(2 * PI_1 * ((i-0.5)/8)) * 0.53, PI_4, 0);
		--SetModelPosition(slideDebugModel, 0.35 + (0.1 * i), -0.36 + (0.1 * (-math.sin(PI_2 + (2 * PI_1 * ((i-0.5)/8))))), -0.75 + (0.1 * i));
		DrawComplexModel(dominoDebugModel,camera,spot,controlSurface);
	end
	
	if drawBall then
		SetModelMatrix(ballDebugModel,GetPhysicsMatrix(physicsBall));
		DrawComplexModel(ballDebugModel,camera,spot,controlSurface);
	end
	
	DrawComplexModel(floorModel,camera,spot,controlSurface);
	
	-- Draw the overlays
	
	local mainWidth, mainHeight = GetBackbufferSize();
	local quadWidth = (performanceWidth/performanceHeight) * quadHeight;
	
	SetMeshScale(performanceQuad,0, quadWidth, quadHeight, 1);
	
	SetMeshTexture(performanceQuad,0,GetSurfaceTexture(controlSurface));
	SetModelPosition(performanceQuad,mainWidth - quadWidth,0,0);
	DrawComplexModel(performanceQuad,spriteCamera,nil,performanceSurface);
	
	-- leapImage = GetLeapImage(leapHelper);
	-- if leapImage then
	-- 	SetMeshTexture(performanceQuad,0,CreateImageTexture(leapImage));
	-- 	SetModelPosition(performanceQuad, mainWidth - quadWidth, mainHeight - quadHeight, 0);
	-- 	--DrawComplexModel(performanceQuad,spriteCamera,nil,performanceSurface);
	-- end
	
	if PROFILING then EndTimestamp("DrawControl",false,true) end
end

function Draw()
	
	if PROFILING then BeginTimestamp("Draw",true,true) end
	
	-- This skybox looks crap, I might have to ditch it...
	
	DrawShadows();

	DrawPerformance();
	
	DrawControls();
		
	if PROFILING then EndTimestamp("Draw",true,true) end
	
	DrawText(debugFont,frameTimeText,0,0);
	-- What cutscene are we on?
	if armLocked then
		DrawText(debugFont,"Locked",0,150);
		
		SetModelPosition(pickModel,0,0,0);
		DrawComplexModel(pickModel,camera,spot);
	end
		
	if PROFILING then
		timestampText = string.format("Update CPU: %2.2fms, Draw CPU: %2.2fms, Draw Gpu: %2.2fms",
			GetTimestampData("Update",true) * 1000,
			GetTimestampData("Draw",true) * 1000,
			GetTimestampData("Draw",false) * 1000);
		
		drawTimeText = string.format("SHDW: %2.2fms, PRFM: %2.2fms, CTRL: %2.2fms",
			GetTimestampData("DrawShadows",false) * 1000,
			GetTimestampData("DrawPerformance",false) * 1000,
			GetTimestampData("DrawControl",false) * 1000);
		
		DrawText(debugFont,timestampText,0,50);
		DrawText(debugFont,drawTimeText,0,100);
	end
	
end -- Draw()

function End()
end
