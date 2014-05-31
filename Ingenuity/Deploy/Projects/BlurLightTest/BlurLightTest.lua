-- Ingenuity Multitexture Heightmap Demo
--
-- Loads a heightmapped model and renders it
-- with a custom multitexturing shader
--
-- Richard Copperwaite 2013

text = "";
sumDeltas = 0;
numDeltas = 0;
progressText = "";
loadComplete = false;
rotLeft = false;
rotRight = false;
moveUp = false;
moveDown = false;
positionText = "";

function CreatePlane(x,z,width,depth,twidth,theight)
	local idx = { 0, 1, 2, 1, 3, 2 };
	if twidth == 0 or theight == 0 then
		vtx = { 
		{x,       0, z,         0, 1, 0 },
		{x,       0, z+depth,   0, 1, 0 },
		{x+width, 0, z,         0, 1, 0 },
		{x+width, 0, z+depth,   0, 1, 0 }
		};
		return CreateModel("PosNor",vtx,4,idx,6);	
	else
		vtx = { 
		{x,       0, z,         0, 1, 0,   1, 0, 0, 1,   0,      0 },
		{x,       0, z+depth,   0, 1, 0,   1, 0, 0, 1,   0,      theight },
		{x+width, 0, z,         0, 1, 0,   1, 0, 0, 1,   twidth, 0 },
		{x+width, 0, z+depth,   0, 1, 0,   1, 0, 0, 1,   twidth, theight }
		};
		return CreateModel("PosNorTanTex",vtx,4,idx,6);	
	end
end

function CreateCube(normals)
	local vtx = {
		{-1,-1,-1},
		{-1, 1,-1},
		{ 1,-1,-1},
		{ 1, 1,-1},
		
		{-1,-1, 1},
		{-1,-1,-1},
		{ 1,-1, 1},
		{ 1,-1,-1},
		
		{-1, 1,-1},
		{-1, 1, 1},
		{ 1, 1,-1},
		{ 1, 1, 1},
		
		{ 1,-1, 1},
		{ 1, 1, 1},
		{-1,-1, 1},
		{-1, 1, 1},
		
		{-1,-1, 1},
		{-1, 1, 1},
		{-1,-1,-1},
		{-1, 1,-1},
		
		{ 1,-1,-1},
		{ 1, 1,-1},
		{ 1,-1, 1},
		{ 1, 1, 1}
	};
	local idx = {
		 0, 1, 2,
		 1, 3, 2,
		 4, 5, 6,
		 5, 7, 6,
		 8, 9,10,
		 9,11,10,
		12,13,14,
		13,15,14,
		16,17,18,
		17,19,18,
		20,21,22,
		21,23,22
	};
	return CreateModel("Pos",vtx,24,idx,36);
end

function Begin()	
	test = GetTexture("PixTest.png");
	surface = CreateSurface(); -- fullscreen
	surface2 = CreateSurface();
	
	addEffect = CreateEffect("BaseAdd",true);
	
	roomFloor = CreatePlane(-0.5,-0.5,1,1,0,0);
	roomWallLeft = CreatePlane(-0.5,-0.5,1,1,0,0);
	roomWallBack = CreatePlane(-0.5,-0.5,1,1,0,0);
	SetModelRotation(roomWallBack,0,0,math.pi/2);
	SetModelPosition(roomWallBack,0.5,0.5,0.5);
	SetMeshEffect(roomWallBack,0,addEffect);
	SetModelRotation(roomWallLeft,-math.pi/2,0,0);
	SetModelPosition(roomWallLeft,0.0,0.5,1.0);
	SetMeshEffect(roomWallLeft,0,addEffect);
	SetModelPosition(roomFloor,0.0,0.0,0.5);
	SetMeshEffect(roomFloor,0,addEffect);
	
	roomFloor2 = CreatePlane(-0.5,-0.5,1,1,0,0);
	roomWallRight = CreatePlane(-0.5,-0.5,1,1,0,0);
	roomWallBack2 = CreatePlane(-0.5,-0.5,1,1,0,0);	
	SetModelRotation(roomWallBack2,0,0,math.pi/2);
	SetModelPosition(roomWallBack2,0.5,0.5,-0.5);
	SetMeshEffect(roomWallBack2,0,addEffect);
	SetModelRotation(roomWallRight,math.pi/2,0,0);
	SetModelPosition(roomWallRight,0.0,0.5,-1.0);	
	SetMeshEffect(roomWallRight,0,addEffect);
	SetModelPosition(roomFloor2,0.0,0.0,-0.5);
	SetMeshEffect(roomFloor2,0,addEffect);
	
	SetMeshColor(roomFloor,0,0,0,0);
	SetMeshColor(roomWallLeft,0,0,0,0);
	SetMeshColor(roomWallBack,0,0,0,0);
	
	SetMeshColor(roomFloor2,0,0,0,0);
	SetMeshColor(roomWallRight,0,0,0,0);
	SetMeshColor(roomWallBack2,0,0,0,0);	
	
	SetClearColor(0,0,0);
	
	model = 0;
	-- castle = 0;
	modelx = 0;
	camRadius = 2;
	rotSpeed = 1.5;
	moveSpeed = 1.0;
	waterShift = 0;
	camera = CreateCamera();
	SetCameraClipFov(camera,0.001,50,0.78539);
	font = GetFont(40,"Arial");
	light = CreateLight("point");
	SetLightPosition(light,0.4,0.1,0.9);
	
	light2 = CreateLight("point");
	SetLightPosition(light2,0.4,0.1,-0.9);
	
	blurEffect = CreateEffect("BlurAndDark",false);
	SetEffectParam(blurEffect,0,0.005);
	
	copyEffect = CreateEffect("TextureCopy",false);
	
	camerax = -2;
	cameraz = 0;
	
	print("Script Started");
end

function Reload()
	--loadComplete = false; Begin();
	--SetCameraClipFov(camera,0.001,50,0.78539);
	Begin();
	print("Reloaded");
end

function KeyDown(key)
	if key == 37 then -- left
		rotLeft = true;
	end
	if key == 38 then
		moveUp = true;
	end
	if key == 39 then -- right
		rotRight = true;
	end
	if key == 40 then
		moveDown = true;
	end
end

function KeyUp(key)
	if key == 37 then -- left
		rotLeft = false;
	end
	if key == 38 then
		moveUp = false;
	end
	if key == 39 then -- right
		rotRight = false;
	end
	if key == 40 then
		moveDown = false;
	end
end

function Update(delta)
	if rotLeft then
		modelx = modelx + (delta * rotSpeed);
	elseif rotRight then
		modelx = modelx - (delta * rotSpeed);
	end
	
	-- height = GetHeightmapHeight(heightmap,camerax,cameraz);
	
	--if(modelx > (math.pi * 2)) then modelx = modelx - (math.pi * 2); end
	--SetCameraPosition(camera,math.sin(modelx) * camRadius,0.75 * camRadius,math.cos(modelx) * camRadius);
	--SetCameraPosition(camera,camerax,height + 0.013,cameraz);
	SetCameraPosition(camera,camerax,0.5,cameraz);
	SetCameraTarget(camera,camerax + math.cos(modelx),0.5,cameraz + math.sin(modelx));
	
	if moveUp then
		camerax = camerax + (math.cos(modelx) * moveSpeed * delta);
		cameraz = cameraz + (math.sin(modelx) * moveSpeed * delta);
	end
	if moveDown then
		camerax = camerax - (math.cos(modelx) * moveSpeed * delta);
		cameraz = cameraz - (math.sin(modelx) * moveSpeed * delta);
	end	
	
	positionText = string.format("%1.3f,%1.3f,%1.3f",camerax,cameraz,modelx);
	
	sumDeltas = sumDeltas + delta;
	numDeltas = numDeltas + 1;
	if sumDeltas > 0.5 then
		local avgDeltas = sumDeltas / numDeltas;
		text = string.format("%2.2fms %3.2f%%",avgDeltas * 1000,(avgDeltas * 100000) / 16);
		sumDeltas = 0;
		numDeltas = 0;
	end
end

function PrintGlobals()
	for k,v in pairs(_G) do
		print("Global key", k, "value", tostring(v))
	end
end

function Draw()
	ShadeSurface(surface2,copyEffect,surface);
	ShadeSurface(surface,blurEffect,surface2);

	DrawComplexModel(roomFloor,camera,light,surface2);
	DrawComplexModel(roomWallBack,camera,light,surface2);
	DrawComplexModel(roomWallLeft,camera,light,surface2);
	
	DrawComplexModel(roomFloor2,camera,light2,surface2);
	DrawComplexModel(roomWallBack2,camera,light2,surface2);
	DrawComplexModel(roomWallRight,camera,light2,surface2);
	
	DrawSurface(surface2);
	
	DrawText(font,text,0,0,0);
end

function End()
	Unload(ticket);
	print("Script Finished");
end