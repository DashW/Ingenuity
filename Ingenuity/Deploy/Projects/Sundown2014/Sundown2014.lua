
Require("ProjectDir","../../Common/ColorHelper.lua");
Require("ProjectDir","../../Common/IngenHDR.lua");
Require("ProjectDir","../../Common/IngenUtils.lua");
Require("ProjectDir","Metronome.lua");

--function CreateSkyCube()
--	local vtx = {
--		{-1,-1,-1}, {-1, 1,-1}, { 1,-1,-1}, { 1, 1,-1},
--		{-1,-1, 1}, {-1,-1,-1}, { 1,-1, 1}, { 1,-1,-1},
--		{-1, 1,-1}, {-1, 1, 1}, { 1, 1,-1}, { 1, 1, 1},
--		{ 1,-1, 1}, { 1, 1, 1}, {-1,-1, 1}, {-1, 1, 1},
--		{-1,-1, 1}, {-1, 1, 1}, {-1,-1,-1}, {-1, 1,-1},
--		{ 1,-1,-1}, { 1, 1,-1}, { 1,-1, 1}, { 1, 1, 1}
--	};
--	local idx = {
--		 0, 1, 2,   1, 3, 2,   4, 5, 6,   5, 7, 6,
--		 8, 9,10,   9,11,10,  12,13,14,  13,15,14,
--		16,17,18,  17,19,18,  20,21,22,  21,23,22
--	};
--	return CreateModel("Pos",vtx,idx);
--end

function CreateUISquare(posX, posY, width, height, tex)
	local idx = {0,1,2,1,3,2};
	if tex then
		vtx = {
			{posX,       posY,        0,    0,    0}, 
			{posX+width, posY,        0,    1,    0},
			{posX,       posY-height, 0,    0,    1},
			{posX+width, posY-height, 0,    1,    1}
		};
		vtxType = "PosTex";
	else
		vtx = {
			{posX,       posY,        0}, 
			{posX+width, posY,        0},
			{posX,       posY-height, 0},
			{posX+width, posY-height, 0}
		};
		vtxType = "Pos";
	end
	return CreateModel(vtxType,vtx,idx);
end

function lerp (a, b, t)
    return a + (b - a) * t
end

rotSpeed = 0.4;

function Begin()
	camera = CreateCamera(true);
	SetCameraClipHeight(camera,1,5000,10);
	SetCameraPosition(camera,0,0,-1000);
	SetCameraTarget(camera,0,0,0);
	
	projCamera = CreateCamera();
	SetCameraClipFov(projCamera,1,200,0.78539);
	SetCameraPosition(projCamera,0,0,-20);
	SetCameraTarget(projCamera,0,0,0);
	SetupFlyCamera(projCamera, 0, 0.4, -9, 0.01, 10);
	
	ticket = LoadAssets(
		{"FrameworkDir","PathShader.xml","Shader","PathShader"},
		{"FrameworkDir","SkyShader.xml","Shader","SkyShader"},
		{"FrameworkDir","Vignette.xml","Shader","Vignette"},
		{"ProjectDir","grassCubeMap.dds","CubeMap","SkyTexture"},
		{"ProjectDir","Space Fighter Loop.wav","WavAudio","SoundFile"},
		{"ProjectDir","Door.svg","SVGModel","DoorSVG"},
		{"ProjectDir","greet.png","Texture","Greet"}
	);

	rect = CreateRect( -5, -5, 10, 10 );
	progressRect = CreateRect( 0, 0, 1, 1 );
	
	isoSurface = CreateIsoSurface(40);
	
	ballPositions = {
		{ x=0, y=-20, z=0 },
		{ x=0, y=-20, z=0 },
		{ x=0, y=-20, z=0 }
	};
	ballSpeeds = {
		{ x=0, y=0, z=0 },
		{ x=0, y=0, z=0 },
		{ x=0, y=0, z=0 }
	};
	ballDamping = 0.01;
	ballDrag = 0.9;
	ballCenterX = 0;
	ballCenterY = -20;
	ballCenterZ = 0;
	
	divisionProgress = 0;
	divisionDirection = 1;
	
	light = CreateLight("point");
	SetLightColor(light,8,8,8);
	modelx = 0;
	
	sphereVtx, sphereIdx = CreateSphere()
	lightSphere = CreateModel("PosNor",sphereVtx,sphereIdx);
	SetMeshColor(lightSphere, 0, 480, 480, 480);
	SetModelScale(lightSphere,4);
	
	BeginHDR();
	
	sceneSurface = CreateSurface(1, 1, true, "3x10f");
	vignetteSurface = CreateSurface(1, 1, true, "3x10f");
	
	plane = CreateModel("PosNor",CreateGrid(100,100,2,2));
	SetModelPosition(plane,0,-7.7,0);
	SetModelScale(plane,100,1,100);
	SetMeshColor(plane,0,0.1,0.1,0.1);
	
	trailSphere = CreateModel("PosNor",sphereVtx,sphereIdx);
	SetMeshColor(trailSphere, 0, 0, 0, 3);
	trailSphereBuffer = CreateInstanceBuffer("PosSca",100);
	trailSphereFloats = CreateFloatArray(600);
	trailBalls = {};
	ballTrailPosX = 0;
	ballTrailPosY = 0;
	ballTrailPosZ = 0;
	ballTrailProgress = 0;
	ballSpawnPeriod = 0.02;
	ballSpawnProgress = 0;
	
	trailCube = CreateModel("PosNor",CreateCube(false));
	SetMeshColor(trailCube, 0, 3, 0, 0);
	trailCubeBuffer = CreateInstanceBuffer("PosSca",100);
	trailCubeFloats = CreateFloatArray(600);
	trailCubes = {};
	cubeTrailPosX = 0;
	cubeTrailPosY = 0;
	cubeTrailPosZ = 0;
	cubeTrailProgress = 0;
	cubeSpawnPeriod = 0.02;
	cubeSpawnProgress = 0;
	
	surfaceDivideBuffer = CreateInstanceBuffer("Pos",2048);
	surfaceDivideFloats = CreateFloatArray(6144);
	surfaceDivision = 0;
	
	section4gotInstances = false;
	
	isoSurfaceOffsetZ = 0;
	
	prevBallTrailPosZ = 0;
	
	debugFont = GetFont(30,"Arial");
	greetFont = GetFont(150,"VX Rocket");
	SetFontColor(greetFont,1,1,1);
	finaleFont = GetFont(50,"Electronic Highway Sign");
	SetFontColor(finaleFont,1,1,1);
	greetSurface = CreateSurface(1024,64,false,"4x16f");
	greetQuad = CreateUISquare(0,0,1024,64,true);
	SetModelRotation(greetQuad,0,math.pi/2,0);
	SetModelScale(greetQuad,0.05);
end

function Reload()
	Begin();
end

function IsoSurfaceInstances(surfaceDivision,divisionDirection,cosCurve)
	local maxXinst = math.pow(2, math.floor((surfaceDivision+2)/3));
	local maxZinst = math.pow(2, math.floor((surfaceDivision+1)/3));
	local maxYinst = math.pow(2, math.floor((surfaceDivision+0)/3));
	local index,xPos, yPos, zPos;
	local xAnim = 0;
	local yAnim = 0;
	local zAnim = 0;
	if divisionDirection == 1 then xAnim = cosCurve; end
	if divisionDirection == 2 then zAnim = cosCurve; end
	if divisionDirection == 3 then yAnim = cosCurve; end
	
	for xInst=1,maxXinst do
		for zInst = 1,maxZinst do
			for yInst = 1,maxYinst do
				index= (xInst-1)+((zInst-1)*maxXinst)+((yInst-1)*maxXinst*maxZinst);
				xPos = ((xInst-1)-((maxXinst-1)/2))*12;
				yPos = ((yInst-1)-((maxYinst-1)/2))*12;
				zPos = ((zInst-1)-((maxZinst-1)/2))*12;
				--print(string.format("%d: (%d,%d,%d)(%2.2f,%2.2f,%2.2f)",index,xInst,yInst,zInst,xPos,yPos,zPos));
				SetFloatArray(surfaceDivideFloats,index*3,xPos+(xAnim*xPos),yPos+(yAnim*yPos),zPos+(zAnim*zPos));
			end
		end
	end
	
	UpdateInstanceBuffer(surfaceDivideBuffer,surfaceDivideFloats,maxXinst*maxYinst*maxZinst);
end

function DivideIsoSurface(delta)
	divisionProgress = divisionProgress + (delta / GetMetronomeBeatTime(2));
	if divisionProgress > 1 then
		divisionProgress = 0;
		surfaceDivision = surfaceDivision + 1;
		divisionDirection = divisionDirection + 1;
		if divisionDirection > 3 then divisionDirection = 1; end
		if surfaceDivision > 10 then surfaceDivision = 0; divisionDirection = 1; end
	end
	
	local cosCurve = (1-math.cos(divisionProgress * math.pi)) * 0.5;
	
	ballX = 0;
	ballY = 0;
	ballZ = 0;
	if divisionDirection == 1 then ballX = 6 * cosCurve; end
	if divisionDirection == 2 then ballZ = 6 * cosCurve; end
	if divisionDirection == 3 then ballY = 6 * cosCurve; end
	
	AddIsoSurfaceBall(isoSurface, ballX, ballY, ballZ, 2 + (cosCurve * 0.75));
	AddIsoSurfaceBall(isoSurface,-ballX,-ballY,-ballZ, 2 + (cosCurve * 0.75));
	
	IsoSurfaceInstances(surfaceDivision,divisionDirection,cosCurve);
end

function UpdateSphereTrail(delta,period,size,life)
	ballTrailProgress = ballTrailProgress + (delta / 8);
	if ballTrailProgress > 1 then ballTrailProgress = 0; end
	
	--ballTrailPosX = math.sin(ballTrailProgress * math.pi * 2) * 10;
	--ballTrailPosZ = math.cos(ballTrailProgress * math.pi * 2) * 10;
	
	ballSpawnProgress = ballSpawnProgress + delta;
	while ballSpawnProgress > period do
		ballSpawnProgress = ballSpawnProgress - period;
		table.insert(trailBalls,{
			x = ballTrailPosX + (math.random() - 0.5);
			y = ballTrailPosY + (math.random() - 0.5);
			z = ballTrailPosZ + (math.random() - 0.5);
			s = size;
		})
	end
	
	for key,ball in pairs(trailBalls) do
		ball.s = ball.s - (delta * life);
		if ball.s < 0 then
			table.remove(trailBalls,key);
		end
	end
	
	local maxKey = 0;
	for key,ball in pairs(trailBalls) do
		if key > 99 then break; end
		maxKey = key;
		SetFloatArray(trailSphereFloats, ((key-1)*6)+0, ball.x,ball.y,ball.z);
		SetFloatArray(trailSphereFloats, ((key-1)*6)+3, ball.s,ball.s,ball.s);
	end
	UpdateInstanceBuffer(trailSphereBuffer,trailSphereFloats,maxKey);
end

function UpdateCubeTrail(delta,period,size,life)
	cubeTrailProgress = cubeTrailProgress + (delta / 8);
	if cubeTrailProgress > 1 then cubeTrailProgress = 0; end
	
	--cubeTrailPosX = math.sin(cubeTrailProgress * math.pi * 2) * 10;
	--cubeTrailPosZ = math.cos(cubeTrailProgress * math.pi * 2) * 10;
	
	cubeSpawnProgress = cubeSpawnProgress + delta;
	while cubeSpawnProgress > period do
		cubeSpawnProgress = cubeSpawnProgress - period;
		table.insert(trailCubes,{
			x = cubeTrailPosX + (math.random() - 0.5);
			y = cubeTrailPosY + (math.random() - 0.5);
			z = cubeTrailPosZ + (math.random() - 0.5);
			s = size * 0.5;
		})
	end
	
	for key,cube in pairs(trailCubes) do
		cube.s = cube.s - (delta * life * 0.5);
		if cube.s < 0 then
			table.remove(trailCubes,key);
		end
	end
	
	local maxKey = 0;
	for key,cube in pairs(trailCubes) do
		if key > 99 then break; end
		maxKey = key;
		SetFloatArray(trailCubeFloats, ((key-1)*6)+0, cube.x,cube.y,cube.z);
		SetFloatArray(trailCubeFloats, ((key-1)*6)+3, cube.s,cube.s,cube.s);
	end
	UpdateInstanceBuffer(trailCubeBuffer,trailCubeFloats,maxKey);
end

function AnimateIsoSurface(delta,wildness)
	size = GetAmplitude(soundFile);
	local pos, vel;
	local nudge = size * wildness;
	for i = 1,3 do
		vel = ballSpeeds[i];
		pos = ballPositions[i];
		vel.x = vel.x + ((math.random()-0.5) * nudge);
		vel.y = vel.y + ((math.random()-0.5) * nudge);
		vel.z = vel.z + ((math.random()-0.5) * nudge);
		
		vel.x = vel.x - ((pos.x-ballCenterX) * ballDamping);
		vel.y = vel.y - ((pos.y-ballCenterY) * ballDamping);
		vel.z = vel.z - ((pos.z-ballCenterZ) * ballDamping);
		vel.x = vel.x * ballDrag;
		vel.y = vel.y * ballDrag;
		vel.z = vel.z * ballDrag;
		
		pos.x = pos.x + vel.x;
		pos.y = pos.y + vel.y;
		pos.z = pos.z + vel.z;
		
		AddIsoSurfaceBall(isoSurface, pos.x, pos.y, pos.z, 2);
	end
end

function AnimateDoorSvg(progress)
	doorModel = GetSVGModel(doorSvg,true,progress);
	local doorMeshes = GetNumMeshes(doorModel);
	for i = 0,doorMeshes-1 do
		SetMeshEffect(doorModel,i,effect);
		SetMeshColor(doorModel,i,lerp(4,20,progress),lerp(4,20,progress),lerp(4,20,progress));
	end
end

function UpdateSection1(delta,sectionTime)
	local beat = GetMetronomeBeat();
	--if beat < 4 then
	--	local progress = sectionTime / GetMetronomeBeatTime(4);
	--	SetCameraPosition(projCamera,0,lerp(0.5,-3,progress),-16);
	--	SetCameraTarget(projCamera,lerp(0,-2,progress),lerp(0.5,-10,progress),0);
	if beat < 12 then
		local progress = (sectionTime) / GetMetronomeBeatTime(12);
		SetCameraPosition(projCamera,lerp(2,6.5,progress),lerp(-6,-0.63,progress),lerp(-7.5,-9.3,progress));
		SetCameraTarget(projCamera,lerp(1.8,6.1,progress),lerp(-6.3,-1.04,progress),lerp(-6.8,-8.5,progress));
	elseif beat < 16 then
		local progress = (sectionTime - GetMetronomeBeatTime(12)) / GetMetronomeBeatTime(4);
		SetCameraPosition(projCamera,lerp(7.3,11.4,progress),lerp(1.7,2.1,progress),lerp(-5.4,0.8,progress));
		SetCameraTarget(projCamera,lerp(6.5,10.5,progress),lerp(1.7,2.1,progress),lerp(-4.8,1.2,progress));
	elseif beat < 20 then
		local progress = (sectionTime - GetMetronomeBeatTime(16)) / GetMetronomeBeatTime(4);
		SetCameraPosition(projCamera,lerp(5.3,6.3,progress),-5.32,-5.45);
		SetCameraTarget(projCamera,lerp(5.1,6.1,progress),-5.5,-6.4);
	elseif beat < 24 then
		local progress = (sectionTime - GetMetronomeBeatTime(20)) / GetMetronomeBeatTime(4);
		SetCameraPosition(projCamera,lerp(6.2,7.2,progress),lerp(-5.9,-5.4,progress),lerp(-5.5,-2.7,progress));
		SetCameraTarget(projCamera,lerp(5.9,6.8,progress),lerp(-6,-5.6,progress),lerp(-6.5,-3.6,progress));
	elseif beat < 30 then
		local progress = (sectionTime - GetMetronomeBeatTime(24)) / GetMetronomeBeatTime(6);
		SetCameraPosition(projCamera,lerp(7.2,10.9,progress),lerp(-5.4,-6.4,progress),lerp(-2.7,-5.6,progress));
		SetCameraTarget(projCamera,lerp(6.8,10.1,progress),lerp(-5.6,-6.1,progress),lerp(-3.6,-5.1,progress));
	end
	
	if beat < 12 then
		local moveTime = (sectionTime) / GetMetronomeBeatTime(12);
		ballCenterY = -15 + (15 * moveTime);
	end
	
	if beat >= 16 and beat < 24 then
		local progress = (sectionTime - GetMetronomeBeatTime(16)) / GetMetronomeBeatTime(8);
		AnimateDoorSvg(progress)
		SetModelPosition(doorModel,5,-5.6,-7);
		SetModelScale(doorModel,0.002);
	end
	
	AnimateIsoSurface(delta,1,2);
	AddIsoSurfacePlane(isoSurface, 0,-10,0,0,5,0);
	
	if beat >= 24 then
		local progress = (sectionTime - GetMetronomeBeatTime(24)) / GetMetronomeBeatTime(6);
		progress = progress * progress;
		ballTrailPosX = lerp(5,6,progress);
		ballTrailPosY = lerp(-5.8,0,progress);
		ballTrailPosZ = lerp(-7,0,progress);
		-- CAN THESE BE ACCELERATED INSTEAD OF LERPED? 
		cubeTrailPosX = lerp(5,-6,progress);
		cubeTrailPosY = lerp(-5.8,0,progress);
		cubeTrailPosZ = lerp(-7,0,progress);
		
		UpdateSphereTrail(delta,0.02,0.2 + (0.3 * progress),0.25);
		UpdateCubeTrail(delta,0.02,0.2 + (0.3 * progress),0.25);
	end
end

function UpdateSection2(delta,sectionTime)
	ballCenterY = 0;
	
	local beat = GetMetronomeBeat() - 30;
	if beat < 10 then
		SetCameraPosition(projCamera,-10.7,3.8,-10.3);
		SetCameraTarget(projCamera,-10,3.5,-9.6);		
	elseif beat < 18 then
		SetCameraPosition(projCamera,math.sin(sectionTime),20,math.cos(sectionTime))
		SetCameraTarget(projCamera,0,0,0);
	else
		SetCameraPosition(projCamera,math.sin(sectionTime)*20,0,math.cos(sectionTime)*20)
		SetCameraTarget(projCamera,0,0,0);
	end
	
	if beat >=4 then
		local interval = (sectionTime - GetMetronomeBeatTime(4)) / GetMetronomeBeatTime(4);
		ballTrailPosX = math.cos(interval) * 10;
		ballTrailPosY = math.sin(interval) * 6;
		ballTrailPosZ = math.sin(interval) * 10;
		
		cubeTrailPosX = -math.cos(interval) * 10;
		cubeTrailPosY = -math.sin(interval) * 6;
		cubeTrailPosZ = -math.sin(interval) * 10;
	end
	if beat >= 2 and beat < 3 then
		local progress = (sectionTime - GetMetronomeBeatTime(2)) / GetMetronomeBeatTime(1);
		ballTrailPosX = lerp( 6, 10,progress);
		cubeTrailPosX = lerp(-6,-10,progress);
	end
	
	if beat >= 28 then
		local progress = (sectionTime - GetMetronomeBeatTime(28)) / GetMetronomeBeatTime(4);
		AnimateIsoSurface(delta,(1-progress),2);
	else
		AnimateIsoSurface(delta,1,2);
	end
	
	AddIsoSurfacePlane(isoSurface,0,-10,0,0,5,0);
	
	UpdateSphereTrail(delta,0.02,0.5,0.25);
	UpdateCubeTrail(delta,0.02,0.5,0.25);
end

function UpdateSection3(delta, sectionTime)
	local beat = GetMetronomeBeat() - 64;
	if beat < 22 then
		DivideIsoSurface(delta);
	else
		DivideIsoSurface(0);
	end
	
	local progress = sectionTime / GetMetronomeBeatTime(28);
	SetCameraPosition(projCamera,lerp(14,110.8,progress),lerp(16.25,108.7,progress),lerp(-26.3,54,progress));
	SetCameraTarget(projCamera,lerp(13.6,110.2,progress),lerp(15.75,108,progress),lerp(-25.7,53.5,progress));
	
	ballTrailPosZ = lerp(0, 100,progress);
	cubeTrailPosZ = lerp(0, 100,progress);
	
	UpdateSphereTrail(delta,0.02,0.5,0.25);
	UpdateCubeTrail(delta,0.02,0.5,0.25);
end

function UpdateSection4(delta,sectionTime)
	if not section4gotInstances then
		IsoSurfaceInstances(10,2,1);
		section4gotInstances = true;
	end
	
	SetLightColor(light,2,2,2);
	
	AnimateIsoSurface(delta,0.3,2);
	
	local beat = GetMetronomeBeat();
	local greetBeat = beat - 96;
	
	if greetBeat == 1 then
		greetText = "HELLO";
		drawGreet = true;
	elseif greetBeat == 3 then
		greetText = "TO";
		drawGreet = true;
	elseif greetBeat == 5 then
		greetText = "EVERYBODY";
		drawGreet = true;
	elseif greetBeat == 7 then
		greetText = "AT";
		drawGreet = true;
	elseif greetBeat == 9 then
		greetText = "SUNDOWN";
		drawGreet = true;
	elseif greetBeat == 11 then
		greetText = "2014";
		drawGreet = true;
	elseif greetBeat == 13 then
		greetText = "!!!!!";
		drawGreet = true;
	else
		drawGreet = false;
	end

	local trailZspeed = 24 / GetMetronomeBeatTime(1);
	if beat < 96 then
		ballTrailPosX = -47;
		ballTrailPosY = -3.0;
		ballTrailPosZ = -146;
		
		cubeTrailPosX = -49;
		cubeTrailPosY = -3.0;
		cubeTrailPosZ = -146;
	elseif beat < 112 then
		ballTrailPosZ = ballTrailPosZ + (trailZspeed * delta);
		cubeTrailPosZ = cubeTrailPosZ + (trailZspeed * delta);
		
		isoSurfaceOffsetZ = (math.floor(ballTrailPosZ/24) * 24) + 72;
	else
		ballTrailPosZ = ballTrailPosZ + (trailZspeed * delta);
		cubeTrailPosZ = cubeTrailPosZ + (trailZspeed * delta);
		
		isoSurfaceOffsetZ = (math.floor(ballTrailPosZ/24) * 24) - 48;
	end
	
	if beat < 96 then
		SetCameraPosition(projCamera,-51.7,-2.3,-149.4);
		SetCameraTarget(projCamera,-51.3,-2.2,-148.5);
	elseif beat < 112 then
		SetCameraPosition(projCamera,-55.7,-1.3,ballTrailPosZ - 3)
		SetCameraTarget(projCamera,ballTrailPosX,ballTrailPosY,ballTrailPosZ);
	else
		local progress = (sectionTime - GetMetronomeBeatTime(20)) / GetMetronomeBeatTime(16);
		local positionX = -math.cos(progress*math.pi) * 20;
		local positionZ =  math.sin(progress*math.pi) * 20;
		--SetCameraPosition(projCamera, positionX, -1.3, ballTrailPosZ + positionZ);
		SetCameraPosition(projCamera, ballTrailPosX - ((1-progress)*20), -1.3, ballTrailPosZ + 3 + (progress * 20));
		--SetCameraPosition(projCamera, positionX, -1.3, ballTrailPosZ + positionZ);
		--SetCameraPosition(projCamera,ballTrailPosX,ballTrailPosY, ballTrailPosZ + 3);
		SetCameraTarget(projCamera,ballTrailPosX,ballTrailPosY,ballTrailPosZ);
	end
	
	UpdateSphereTrail(delta,0.005,0.5,1);
	UpdateCubeTrail(delta,0.005,0.5,1);
end

function UpdateSection5(delta,sectionTime)
	local beat = GetMetronomeBeat() - 128;
	
	ballCenterY = 0;
	SetLightColor(light,2,2,2);
	
	if beat < 8 then
		isoSurfaceOffsetZ = 2096
		local progress = (sectionTime) / GetMetronomeBeatTime(8);
		AnimateDoorSvg(progress)
		SetModelPosition(doorModel,-1.6,0.6,2096);
		SetModelScale(doorModel,0.005);
		if beat < 4 then
			SetCameraPosition(projCamera,lerp(-5,-2.5,progress*2),0,lerp(2094,2080,progress*2));
			SetCameraTarget(projCamera,0,0,2096);
		else
			SetCameraPosition(projCamera,lerp(-2.5,0,(progress*2)-1),0,lerp(2080,2000,(progress*2)-1));
			SetCameraTarget(projCamera,0,0,2096);
		end
	elseif beat < 16 then
		local progress = (sectionTime - GetMetronomeBeatTime(8)) / GetMetronomeBeatTime(8);
		SetCameraPosition(projCamera,ballTrailPosX,ballTrailPosY,lerp(ballTrailPosZ + 30,ballTrailPosZ + 3,progress));
		SetCameraTarget(projCamera,ballTrailPosX,ballTrailPosY,ballTrailPosZ);
		SetCameraUp(projCamera,math.sin((1-progress) * math.pi * 2),math.cos((1-progress) * math.pi * 2),0);
		SetModelPosition(doorModel,-1.6,0.6,ballTrailPosZ + 250)
		isoSurfaceOffsetZ = (math.floor(ballTrailPosZ/24) * 24) - 48;
	elseif beat < 20 then
		SetCameraPosition(projCamera,-5,0,ballTrailPosZ - 5)
		SetCameraTarget(projCamera,ballTrailPosX,ballTrailPosY,ballTrailPosZ + 5);
		SetModelPosition(doorModel,-1.6,0.6,ballTrailPosZ + 50);
		isoSurfaceOffsetZ = (math.floor(ballTrailPosZ/24) * 24) + 72;
	elseif beat < 28 then
		local progress = (sectionTime - GetMetronomeBeatTime(20)) / GetMetronomeBeatTime(8);
		local cameraPosZ = lerp(prevBallTrailPosZ + 8,prevBallTrailPosZ + 108,progress);
		SetCameraPosition(projCamera,-5.7,1.3,cameraPosZ);
		SetCameraTarget(projCamera,ballTrailPosX,ballTrailPosY,lerp(prevBallTrailPosZ, prevBallTrailPosZ + 100, progress));
		
		ballTrailPosZ = lerp(prevBallTrailPosZ, prevBallTrailPosZ + 200, progress * progress);
		cubeTrailPosZ = lerp(prevBallTrailPosZ, prevBallTrailPosZ + 200, progress * progress);
		
		SetModelPosition(doorModel,-1.6,0.6,2500);
		
		isoSurfaceOffsetZ = (math.floor(cameraPosZ/24) * 24) - 72;
	elseif beat < 31 then
		local progress = (sectionTime - GetMetronomeBeatTime(28)) / GetMetronomeBeatTime(3);
		isoSurfaceOffsetZ = 2096
		SetModelPosition(doorModel,-1.6,0.6,2096);
		SetCameraPosition(projCamera,0,0,lerp(2000,2095,progress*progress));
		SetCameraTarget(projCamera,0,0,2096);
	else
		drawBlack = true;
	end
	
	--610.83
	
	if beat < 1 then
		ballTrailPosZ = 0;
		cubeTrailPosZ = 0;
	elseif beat >= 6 and beat < 20 then
		local trailZspeed = 24 / GetMetronomeBeatTime(1);
		ballTrailPosZ = ballTrailPosZ + (trailZspeed * delta);
		cubeTrailPosZ = cubeTrailPosZ + (trailZspeed * delta);
		
		ballTrailPosX =  math.sin(sectionTime) * 0.5;
		ballTrailPosY =  math.cos(sectionTime) * 0.5;
		cubeTrailPosX = -math.sin(sectionTime) * 0.5;
		cubeTrailPosY = -math.cos(sectionTime) * 0.5;
		
		prevBallTrailPosZ = ballTrailPosZ;
	end

	AddIsoSurfaceBall(isoSurface,0,0,0,3);--lerp(3,7,sectionTime/GetMetronomeBeatTime(32)) + (math.sin((sectionTime/GetMetronomeBeatTime(1))*math.pi) * 0.25));
	UpdateSphereTrail(delta,0.005,0.5,1);
	UpdateCubeTrail(delta,0.005,0.5,1);
end

function Update(delta)
	if IsLoaded(ticket) then
		print("Ticket Loaded!");
		effect = CreateEffect("PathShader",true);
		SetEffectParam(effect,0,0);
		SetMeshEffect(rect,0,effect);
		SetMeshEffect(progressRect,0,effect);
		
		soundFile = GetAsset("SoundFile");
		
		if soundFile then
			soundDuration = GetSoundDuration(soundFile);
			--SetupMetronome(soundFile,101.25,0.25);
			SetupMetronome(soundFile, 95, 0.0);
			
			PlaySound(soundFile);
		end
		
		vignetteEffect = CreateEffect("Vignette",false);
		
		--skyEffect = CreateEffect("SkyShader",true);
		--skymap = GetAsset("SkyTexture");
		--if skyEffect and skymap then
		--	skyCube = CreateSkyCube();
		--	SetModelScale(skyCube,70);
		--	SetMeshCubeMap(skyCube,0,skymap);
		--	SetMeshEffect(skyCube,0,skyEffect);
		--end
		
		doorSvg = GetAsset("DoorSVG");
		if doorSvg then
			doorModel = GetSVGModel(doorSvg,true,0);
			local doorMeshes = GetNumMeshes(doorModel);
			for i = 0,doorMeshes-1 do
				SetMeshEffect(doorModel,i,effect);
				SetMeshColor(doorModel,i,20,20,20);
			end
			SetModelPosition(doorModel,5,-5.6,-7);
			SetModelScale(doorModel,0.002);
		end
		
		greetImg = GetAsset("Greet");
		SetMeshEffect(greetQuad,0,effect);
		SetMeshTexture(greetQuad,0,greetImg);
		
		ticket = -1;
	end

	local mouseX, mouseY = GetMousePosition();
	local screenWidth, screenHeight = GetScreenSize();
	--local hue = mouseY / screenHeight;
	
	--SetClearColor(HSLToRGB(hue,1,0.5));
	--SetMeshColor(rect,0,HSLToRGB(1-hue,1,0.5));
	--SetModelScale(rect,size);
	--SetMeshColor(progressRect,0,HSLToRGB(1-hue,1,0.5))
	
	if soundFile then
		soundProgress = GetSoundProgress(soundFile);
		--SetModelPosition(progressRect,-5 * (screenWidth/screenHeight), 4, 0);
		--SetModelScale(progressRect,(screenWidth/screenHeight) * (soundProgress/soundDuration) * 10, 1, 0);
	end
	
	UpdateFlyCamera(delta);
	
	ClearIsoSurface(isoSurface);
	
	if soundFile then
		UpdateMetronome(delta);
		local beat = GetMetronomeBeat();
		local trackTime = GetSoundProgress(soundFile);
		local sectionTime;
		--if math.floor((metronomeBeat) % 2) == 0 then
		--	SetMeshColor(isoSurfaceModel,0,0,0,1);
		--else
		--	SetMeshColor(isoSurfaceModel,0,1,0,0);
		--end
		if trackTime > 0 then
			drawFinale = false;
			if beat < 30 then
				UpdateSection1(delta,trackTime);
			elseif beat < 64 then
				UpdateSection2(delta,trackTime-GetMetronomeBeatTime(30));
			elseif beat < 92 then
				UpdateSection3(delta,trackTime-GetMetronomeBeatTime(64));
			elseif beat < 128 then
				UpdateSection4(delta,trackTime-GetMetronomeBeatTime(92));
			elseif beat < 160 then
				UpdateSection5(delta,trackTime-GetMetronomeBeatTime(128));
			end
		else
			drawBlack = false;
			drawGreet = false;
			drawFinale = true;
		end
	end
	
	isoSurfaceModel = GetIsoSurfaceModel(isoSurface);
	SetMeshColor(isoSurfaceModel,0,0.1,0.1,0.1);
	SetModelPosition(isoSurfaceModel,0,0,isoSurfaceOffsetZ);
	
	modelx = modelx + (delta * rotSpeed);
	if(modelx > (math.pi * 2)) then modelx = modelx - (math.pi * 2); end
	local lightPosX = math.sin(-0.5) * 50;
	local lightPosY = 15;
	local lightPosZ = math.cos(-0.5) * 50;
	SetLightPosition(light,lightPosX,lightPosY,lightPosZ);
	SetModelPosition(lightSphere,lightPosX,lightPosY,lightPosZ);
	--SetLightDirection(light,math.sin(modelx-0.5),0.4,math.cos(modelx-0.5));
	
	--UpdateSphereTrail(delta);
	UpdateHDR(delta);
end

function DrawSection1()
	DrawComplexModel(lightSphere,projCamera,0,sceneSurface);
	DrawComplexModel(isoSurfaceModel,projCamera,light,sceneSurface);
	DrawComplexModel(plane,projCamera,light,sceneSurface);
	DrawComplexModel(doorModel,projCamera,0,sceneSurface);
	DrawComplexModel(trailSphere,projCamera,light,sceneSurface,trailSphereBuffer);
	DrawComplexModel(trailCube,projCamera,light,sceneSurface,trailCubeBuffer);
	--DrawComplexModel(trailSphere,projCamera,light,sceneSurface,trailSphereBuffer);
	
	--SetBlendMode("none");
	--ClearSurface(greetSurface, 1,1,1,1);
	--DrawText(greetFont,"KUDOS TO EVERYONE AT SUNDOWN 2014!!!",0,0,0,greetSurface);
	--greetTexture = GetSurfaceTexture(greetSurface);
	--SetMeshTexture(greetQuad,0,greetTexture);
	--DrawComplexModel(greetQuad,projCamera,0,sceneSurface);
	--SetBlendMode("none");
	
end

function DrawSection2()
	DrawComplexModel(lightSphere,projCamera,0,sceneSurface);
	DrawComplexModel(isoSurfaceModel,projCamera,light,sceneSurface);
	DrawComplexModel(plane,projCamera,light,sceneSurface);
	DrawComplexModel(trailSphere,projCamera,light,sceneSurface,trailSphereBuffer);
	DrawComplexModel(trailCube,projCamera,light,sceneSurface,trailCubeBuffer);
end

function DrawSection3()
	DrawComplexModel(lightSphere,projCamera,0,sceneSurface);
	DrawComplexModel(isoSurfaceModel,projCamera,light,sceneSurface,surfaceDivideBuffer);
	DrawComplexModel(trailSphere,projCamera,light,sceneSurface,trailSphereBuffer);
	DrawComplexModel(trailCube,projCamera,light,sceneSurface,trailCubeBuffer);
end

function DrawSection4()
	DrawComplexModel(lightSphere,projCamera,0,sceneSurface);
	DrawComplexModel(isoSurfaceModel,projCamera,light,sceneSurface,surfaceDivideBuffer);
	DrawComplexModel(trailSphere,projCamera,light,sceneSurface,trailSphereBuffer);
	DrawComplexModel(trailCube,projCamera,light,sceneSurface,trailCubeBuffer);
end

function DrawSection5()
	DrawComplexModel(lightSphere,projCamera,0,sceneSurface);
	DrawComplexModel(isoSurfaceModel,projCamera,light,sceneSurface,surfaceDivideBuffer);
	DrawComplexModel(trailSphere,projCamera,light,sceneSurface,trailSphereBuffer);
	DrawComplexModel(trailCube,projCamera,light,sceneSurface,trailCubeBuffer);
	DrawComplexModel(doorModel,projCamera,light,sceneSurface);
end

function Draw()
	if ticket == -1 then
		--DrawComplexModel(rect,camera);
		if drawGreet then
			SetClearColor(0,0,0);
			SetBlendMode("alpha");
			screenWidth,screenHeight = GetScreenSize()
			DrawText(greetFont,greetText,screenWidth/2,screenHeight/2,true);
		elseif drawBlack then
			SetClearColor(0,0,0);
		elseif drawFinale then
			SetClearColor(0,0,0);
			SetBlendMode("alpha");
			screenWidth,screenHeight = GetScreenSize()
			DrawText(finaleFont,"@richcopperwaite",0,0,false);
			DrawText(finaleFont,"DashW",0,50,false);
			DrawText(finaleFont,"Space Fighter Loop - Kevin MacLeod",0,screenHeight-50,false);
			DrawText(finaleFont,"Creative Commons Music",0,screenHeight-100,false);
		else
			SetClearColor(0.1,0.1,0.1);
			ClearSurface(sceneSurface, 0.3,0.3,0.3);
			SetBlendMode("none");
			
			local beat = GetMetronomeBeat();
			if beat < 30 then
				DrawSection1();
			elseif beat < 64 then
				DrawSection2();
			elseif beat < 92 then
				DrawSection3();
			elseif beat < 128 then
				DrawSection4();
			elseif beat < 160 then
				DrawSection5();
			end
	
			hdrSurface = ShadeHDR(sceneSurface);
			ShadeSurface(hdrSurface,vignetteEffect,vignetteSurface);
			DrawSurface(vignetteSurface);
		end
		
		
		--DrawSprite(greetImg,0,0,1);
		
		--if soundFile then
		--	DrawText(debugFont, string.format("%2.2f/%2.2f",soundProgress,soundDuration), 0, 0, 0);
		--	DrawText(debugFont, string.format("Beat: %d",GetMetronomeBeat()), 0, 40, 0);
		--	--DrawText(debugFont, string.format("Camera: (%2.2f,%2.2f,%2.2f)", flyCamX,flyCamY,flyCamZ), 0, 80, 0);
		--	--
		--	--local dirX = (math.sin(flyCamYAngle) * math.cos(flyCamUpAngle));
		--	--local dirY = (math.sin(flyCamUpAngle));
		--	--local dirZ = (math.cos(flyCamYAngle) * math.cos(flyCamUpAngle));
		--	--
		--	--DrawText(debugFont, string.format("Target: (%2.2f,%2.2f,%2.2f)", flyCamX+dirX,flyCamY+dirY,flyCamZ+dirZ),0,120,0);
		--end
	end
end

function End()
end

-- Helper function
function PlayJump(beat)
	PlaySound(soundFile,GetMetronomeBeatTime(beat));
end