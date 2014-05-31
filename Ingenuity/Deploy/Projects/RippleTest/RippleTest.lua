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

-- function CreateCube(normals)
	-- local vtx = {
		-- {-1,-1,-1},
		-- {-1, 1,-1},
		-- { 1,-1,-1},
		-- { 1, 1,-1},
		
		-- {-1,-1, 1},
		-- {-1,-1,-1},
		-- { 1,-1, 1},
		-- { 1,-1,-1},
		
		-- {-1, 1,-1},
		-- {-1, 1, 1},
		-- { 1, 1,-1},
		-- { 1, 1, 1},
		
		-- { 1,-1, 1},
		-- { 1, 1, 1},
		-- {-1,-1, 1},
		-- {-1, 1, 1},
		
		-- {-1,-1, 1},
		-- {-1, 1, 1},
		-- {-1,-1,-1},
		-- {-1, 1,-1},
		
		-- { 1,-1,-1},
		-- { 1, 1,-1},
		-- { 1,-1, 1},
		-- { 1, 1, 1}
	-- };
	-- local idx = {
		 -- 0, 1, 2,
		 -- 1, 3, 2,
		 -- 4, 5, 6,
		 -- 5, 7, 6,
		 -- 8, 9,10,
		 -- 9,11,10,
		-- 12,13,14,
		-- 13,15,14,
		-- 16,17,18,
		-- 17,19,18,
		-- 20,21,22,
		-- 21,23,22
	-- };
	-- return CreateModel("Pos",vtx,24,idx,36);
-- end

function Begin()	
	test = GetTexture("PixTest.png");
	surface = CreateSurface(); -- fullscreen
	surface2 = CreateSurface();
	surface3 = CreateSurface();
	
	rippleDropoff = 0.7;
	rippleProgress = 0.5;
	rippleSpeed = 2.0;
	
	rippleEffect = CreateEffect("RippleLight",true);
	SetEffectParam(rippleEffect,0,rippleProgress);
	SetEffectParam(rippleEffect,1,0.05);
	SetEffectParam(rippleEffect,2,rippleDropoff);
	
	spec = 1000000000;
	
	roomFloor = CreatePlane(-0.5,-0.5,1,1,0,0);
	roomWallLeft = CreatePlane(-0.5,-0.5,1,1,0,0);
	roomWallBack = CreatePlane(-0.5,-0.5,1,1,0,0);
	SetModelRotation(roomWallBack,0,0,math.pi/2);
	SetModelPosition(roomWallBack,0.5,0.5,0.7);
	SetMeshSpecular(roomWallBack,0,spec);
	SetModelRotation(roomWallLeft,-math.pi/2,0,0);
	SetModelPosition(roomWallLeft,0.0,0.5,1.0);
	SetMeshSpecular(roomWallLeft,0,spec);
	SetModelPosition(roomFloor,0.0,0.0,0.5);
	SetMeshSpecular(roomFloor,0,spec);
	
	roomFloor2 = CreatePlane(-0.5,-0.5,1,1,0,0);
	roomWallRight = CreatePlane(-0.5,-0.5,1,1,0,0);
	roomWallBack2 = CreatePlane(-0.5,-0.5,1,1,0,0);	
	SetModelRotation(roomWallBack2,0,0,math.pi/2);
	SetModelPosition(roomWallBack2,0.5,0.5,-0.7);
	SetMeshSpecular(roomWallBack2,0,spec);
	SetModelRotation(roomWallRight,math.pi/2,0,0);
	SetModelPosition(roomWallRight,0.0,0.5,-1.0);	
	SetMeshSpecular(roomWallRight,0,spec);
	SetModelPosition(roomFloor2,0.0,0.0,-0.5);
	SetMeshSpecular(roomFloor2,0,spec);
	
	roomFloor3 = CreatePlane(-0.5,-0.5,1,1,0,0);
	roomWallBack3 = CreatePlane(-0.5,-0.5,1,1,0,0);
	SetModelRotation(roomWallBack3,0,0,math.pi/2);
	SetModelPosition(roomWallBack3,1.5,0.5,0.0);
	SetMeshSpecular(roomWallBack3,0,spec);
	SetModelPosition(roomFloor3,1.0,0.0,0.0);
	SetMeshSpecular(roomFloor3,0,spec);
	
	roomDoor = CreatePlane(-0.5,-0.2,1,0.4,0,0);
	SetModelRotation(roomDoor,0,0,math.pi/2);
	SetModelPosition(roomDoor,0.5,0.5,0.0);
	SetMeshSpecular(roomDoor,0,spec);	
	
	roomSphere = CreateSphere();
	SetModelPosition(roomSphere,0.35,0.15,-0.7);
	SetModelSize(roomSphere,0.3);
	SetMeshSpecular(roomSphere,0,spec);
	
	meshEffect = rippleEffect;
	
	SetMeshEffect(roomWallBack,0,meshEffect);
	SetMeshEffect(roomWallLeft,0,meshEffect);
	SetMeshEffect(roomFloor,0,meshEffect);
	SetMeshEffect(roomWallBack2,0,meshEffect);
	SetMeshEffect(roomWallRight,0,meshEffect);
	SetMeshEffect(roomFloor2,0,meshEffect);
	SetMeshEffect(roomWallBack3,0,meshEffect);
	SetMeshEffect(roomFloor3,0,meshEffect);
	SetMeshEffect(roomDoor,0,meshEffect);
	SetMeshEffect(roomSphere,0,meshEffect);
	
	SetClearColor(0,0,0);
	
	bright = 0.3;
	
	model = 0;
	castle = 0;
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
	SetLightColor(light,bright,bright,bright);
	SetLightAttenuation(light,2);
	
	light2 = CreateLight("point");
	SetLightPosition(light2,0.4,0.1,-0.9);
	SetLightColor(light2,bright,bright,bright);
	SetLightAttenuation(light2,2);
	
	light3 = CreateLight("point");
	SetLightPosition(light3,1.4,0.1,0.0);
	SetLightColor(light3,bright,bright,bright);
	SetLightAttenuation(light3,4);	
	
	darkEffect = CreateEffect("Darken",false);
	SetEffectParam(darkEffect,0,0.03);
	
	blurEffect = CreateEffect("BoxBlur",false);
	copyEffect = CreateEffect("TextureCopy",false);
	addEffect = CreateEffect("TextureAdd",false);
	
	colorEffect = CreateEffect("ColorVignette",false);
	SetEffectParam(colorEffect,1,0.0);
	SetEffectParam(colorEffect,0,0.05);
	SetEffectParam(colorEffect,2,0.15);
	
	echoAbility = false;
	
	doorPos = 1.0;
	doorDir = -1.0;	
	
	camerax = -2;
	cameraz = 0;
	
	print("Script Started");
end

function Reload()
	Begin();
	print("Reloaded");
end

function KeyDown(key)
	if key == 32 then
		echoAbility = true;
	end
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
	if key == 32 then
		echoAbility = false;
	end
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
	
	rippleProgress = rippleProgress + (delta * rippleSpeed);
	if(rippleProgress > rippleDropoff + 0.5) then rippleProgress = rippleProgress - (rippleDropoff + 0.5); end
	
	SetEffectParam(rippleEffect,0,rippleProgress);
	
	SetCameraPosition(camera,camerax,0.5,cameraz);
	SetCameraTarget(camera,camerax + math.cos(modelx),0.5,cameraz + math.sin(modelx));
	
	if doorPos < delta or doorPos > 1.0 then 
		doorDir = -doorDir;
	end
	doorPos = doorPos + (doorDir * delta);
	
	SetModelPosition(roomDoor,0.5,0.5+doorPos,0.0);	
	
	if moveUp then
		camerax = camerax + (math.cos(modelx) * moveSpeed * delta);
		cameraz = cameraz + (math.sin(modelx) * moveSpeed * delta);
	end
	if moveDown then
		camerax = camerax - (math.cos(modelx) * moveSpeed * delta);
		cameraz = cameraz - (math.sin(modelx) * moveSpeed * delta);
	end	
	
	if echoAbility then
		meshEffect = 0;
	else
		meshEffect = rippleEffect;
	end
	
	SetMeshEffect(roomWallBack,0,meshEffect);
	SetMeshEffect(roomWallLeft,0,meshEffect);
	SetMeshEffect(roomFloor,0,meshEffect);
	SetMeshEffect(roomWallBack2,0,meshEffect);
	SetMeshEffect(roomWallRight,0,meshEffect);
	SetMeshEffect(roomFloor2,0,meshEffect);
	SetMeshEffect(roomWallBack3,0,meshEffect);
	SetMeshEffect(roomFloor3,0,meshEffect);
	SetMeshEffect(roomDoor,0,meshEffect);	
	SetMeshEffect(roomSphere,0,meshEffect);
	
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
	
	--ShadeSurface(surface2,copyEffect,surface);
	--ShadeSurface(surface,blurEffect,surface2);
	
	-- DARKEN UN-BLURRED PREVIOUS
	
	ShadeSurface(surface,darkEffect,surface);
	ShadeSurface(surface,colorEffect,surface);
	
	ClearSurface(surface2);	
	
	DrawComplexModel(roomFloor3,camera,light3,surface2);
	DrawComplexModel(roomWallBack3,camera,light3,surface2);	
	DrawComplexModel(roomDoor,camera,light3,surface2);	

	--DrawComplexModel(ground,camera,light);
	DrawComplexModel(roomFloor,camera,light,surface2);
	DrawComplexModel(roomWallBack,camera,light,surface2);
	DrawComplexModel(roomWallLeft,camera,light,surface2);
	
	DrawComplexModel(roomFloor2,camera,light2,surface2);
	DrawComplexModel(roomWallBack2,camera,light2,surface2);
	DrawComplexModel(roomWallRight,camera,light2,surface2);
	DrawComplexModel(roomSphere,camera,light2,surface2);
	
	-- ADD TO UN-BLURRED PREVIOUS
	
	ShadeSurface(surface2,addEffect,surface);
	
	-- *THEN* BLUR
	
	ShadeSurface(surface,copyEffect,surface3);
	DrawSurface(surface3);
end

function End()
	Unload(ticket);
	print("Script Finished");
end