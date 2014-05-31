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

function CreateWater()
	water = CreatePlane(-1,-1,2,2,10,10);

	waterEffect = CreateEffect("Water");
	waterNormal1 = GetTexture("wave0.dds");
	waterNormal2 = GetTexture("wave1.dds");
	
	SetMeshEffect(water,0,waterEffect);
	SetMeshNormal(water,0,waterNormal1);
	SetMeshCubeMap(water,0,skymap);
	SetEffectParam(waterEffect,0,waterNormal2);
end

function CreateSphere(stacks,sectors)
	local vtx = {};
	local numvtx = 0;
	local idx = {};
	local numidx = 0;
	
	for stack=1,stacks do
		for sector=1,sectors do
			--table.insert(vertices,{x,y,z})
		end
	end
	
	return CreateModel("Pos",vtx,numvtx,idx,numidx);
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
	logoSprite = GetTexture("IngenuityIcon01.png");

	tex = GetTexture("blend_castle.dds");
	dirt = GetTexture("dirt.dds");
	grass = GetTexture("grass.dds");
	stone = GetTexture("stone.dds");
	ground, heightmap = GetHeightmapModel("castlehm257.raw",257,0.002);
	skymap = GetTexture("grassCubeMap.dds","cube");
	
	-- y = GetHeightValue(ground,x,z);
	
	effect = CreateEffect("MultiTexAnimY");
	
	--ticket = Load("skull3.obj","castle.obj");
	
	test = GetTexture("PixTest.png");
	surface = CreateSurface(); -- fullscreen
	surface2 = CreateSurface();
	surface3 = CreateSurface();
	
	SetEffectParam(effect,0,grass);  --tex1
	SetEffectParam(effect,1,dirt);   --tex2
	SetEffectParam(effect,2,stone);  --tex3
	SetEffectParam(effect,3,0.0);    --yStart
	SetEffectParam(effect,4,1.0);    --yProgress	
	
	SetMeshTexture(ground,0,tex);
	SetMeshEffect(ground,0,effect);
	
	--CreateWater();
	
	--CRASH HAZARD: LOADING THE WRONG KIND OF TEXTURE FOR A SHADER (FLAT,CUBE,VOLUME)
	dotsTex = GetTexture("HalftoneDots16x16.dds","volume"); 
	dotsEffect = CreateEffect("HalftoneDots");
	SetEffectParam(dotsEffect,0,dotsTex);		
	
	skyEffect = CreateEffect("Sky");
	skyBox = CreateCube(false);
	SetModelSize(skyBox,50);
	SetMeshCubeMap(skyBox,0,skymap);
	SetMeshEffect(skyBox,0,skyEffect);
	
	--addEffect = CreateEffect("BaseAdd");
	
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
	
	-- SetMeshColor(roomFloor,0,0,0,0);
	-- SetMeshColor(roomWallLeft,0,0,0,0);
	-- SetMeshColor(roomWallBack,0,0,0,0);
	
	-- SetMeshColor(roomFloor2,0,0,0,0);
	-- SetMeshColor(roomWallRight,0,0,0,0);
	-- SetMeshColor(roomWallBack2,0,0,0,0);	
	
	--SetMeshTexture(roomFloor,0,tex);
	
	SetClearColor(0,0,0);
	
	vase = GetModel("vase.obj");
	barrel = GetModel("oildrum.obj");
	
	bright = 0.1;
	
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
	SetLightAttenuation(light,4);
	
	light2 = CreateLight("point");
	SetLightPosition(light2,0.4,0.1,-0.9);
	SetLightColor(light2,bright,bright,bright);
	SetLightAttenuation(light2,4);
	
	light3 = CreateLight("point");
	SetLightPosition(light3,1.4,0.1,0.0);
	SetLightColor(light3,bright,bright,bright);
	SetLightAttenuation(light3,4);
	
	echoLight = CreateLight("point");
	SetLightColor(echoLight,1.0,1.0,1.0);
	SetLightAttenuation(echoLight,0.8);
	
	darkEffect = CreateEffect("Darken");
	SetEffectParam(darkEffect,0,0.005);
	
	blurEffect = CreateEffect("BoxBlur");
	copyEffect = CreateEffect("TexCopy");
	addEffect = CreateEffect("TexAdd");
	
	colorEffect = CreateEffect("ColorVignette");
	SetEffectParam(colorEffect,1,0.0);
	SetEffectParam(colorEffect,0,0.05);
	SetEffectParam(colorEffect,2,0.15);	
	
	echoFrames = 0;
	
	doorPos = 1.0;
	doorDir = -1.0;
	
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
	if key == 32 then --space
		echoFrames = 2;
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
	
	height = GetHeightmapHeight(heightmap,camerax,cameraz);
	
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
	
	SetLightPosition(echoLight,camerax,0.5,cameraz);
	
	if doorPos < delta or doorPos > 1.0 then 
		doorDir = -doorDir;
	end
	doorPos = doorPos + (doorDir * delta);
	
	SetModelPosition(roomDoor,0.5,0.5+doorPos,0.0);
	
	--SetCameraPosition(camera,0,3,-0.001);
	--SetLightDirection(light,math.sin(modelx-0.5),0.75,math.cos(modelx-0.5));
	--SetLightDirection(light,math.sin(2.5),0.75,math.cos(2.5));
	
	--progressText = string.format("%1.3f complete",GetLoadProgress(ticket));
	--if not loadComplete and IsLoaded(ticket) then
	--	model = GetModel("skull3.obj");
	--	castle = GetModel("castle.obj");
		--SetModelPosition(model,0,-0.25,0);
	--	loadComplete = true;
	--end
	
	if loadComplete then
		SetModelSize(model,0.01);
		SetModelPosition(model,0.03,0.15,0.37);
		
		SetModelSize(castle,0.0038);
		SetModelPosition(castle,0.03,0.15,0.33);
	end
	
	SetModelPosition(vase,0,0.152,0.33);
	SetModelSize(vase,0.0002);
	SetModelPosition(barrel,0.06,0.152,0.33);
	SetModelSize(barrel,0.015);
	
	positionText = string.format("%1.3f,%1.3f,%1.3f",camerax,cameraz,modelx);
	
	--SetModelSize(water,0.7);
	--SetModelPosition(water,0,0.13,0.13);
	
	--waterShift = waterShift + (delta * 0.05);
	--if waterShift > 1 then waterShift = waterShift - 1 end
	--SetEffectParam(waterEffect,1,-waterShift);
	--SetEffectParam(waterEffect,2,waterShift);
	--SetEffectParam(waterEffect,3,waterShift);
	--SetEffectParam(waterEffect,4,waterShift);
	
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
	
	if echoFrames > 0 then
		l1 = echoLight;
		l2 = echoLight;
		l3 = echoLight;
		echoFrames = echoFrames - 1;
	else
		l1 = light;
		l2 = light2;
		l3 = light3;
	end
	
	-- DARKEN UN-BLURRED PREVIOUS
	
	ShadeSurface(surface,darkEffect,surface);
	ShadeSurface(surface,blurEffect,surface);
	
	ClearSurface(surface2);	

	--DrawComplexModel(ground,camera,light);
	DrawComplexModel(roomFloor3,camera,l3,surface2);
	DrawComplexModel(roomWallBack3,camera,l3,surface2);	
	
	DrawComplexModel(roomDoor,camera,l3,surface2);
	
	DrawComplexModel(roomFloor,camera,l1,surface2);
	DrawComplexModel(roomWallBack,camera,l1,surface2);
	DrawComplexModel(roomWallLeft,camera,l1,surface2);
	
	DrawComplexModel(roomFloor2,camera,l2,surface2);
	DrawComplexModel(roomWallBack2,camera,l2,surface2);
	DrawComplexModel(roomWallRight,camera,l2,surface2);
	
	-- ADD TO UN-BLURRED PREVIOUS
	
	ShadeSurface(surface2,addEffect,surface);
	
	-- *THEN* BLUR
	
	ShadeSurface(surface3,colorEffect,surface3);
	ShadeSurface(surface,darkEffect,surface);
	ShadeSurface(surface,addEffect,surface3);
	
	--DrawComplexModel(water,camera,light);
	--if castle ~= 0 then
	--	DrawComplexModel(castle,camera,light);
	--end
	--if model ~= 0 then
	--	DrawComplexModel(model,camera,light);--,surface);
	--end
	--DrawComplexModel(vase,camera,light);
	--DrawComplexModel(barrel,camera,light);
	--DrawComplexModel(skyBox,camera);
	--ShadeSurface(surface,dotsEffect,surface2);
	--DrawSurface(surface);
	--DrawSprite(test,-0.05,-1);
	
	DrawSurface(surface3);
	
	--DrawText(font,text,0,0,0);
	-- if not loadComplete then
		-- DrawText(font,progressText,400,0,0);
	-- end
	--DrawText(font,positionText,700,0,0);
end

function End()
	Unload(ticket);
	print("Script Finished");
end