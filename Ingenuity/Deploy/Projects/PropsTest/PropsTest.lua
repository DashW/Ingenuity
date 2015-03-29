-- Ingenuity Multitexture Heightmap Demo
--
-- Loads a heightmapped model and renders it
-- with a custom multitexturing shader
--
-- Richard Copperwaite 2013

Require("ProjectDir","../../Common/IngenUtils.lua");

function PrintGlobals()
	for k,v in pairs(_G) do
		print("Global key", k, "value", tostring(v))
	end
end

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

function UpdateFrameTime(delta)
	sumDeltas = sumDeltas + delta;
	numDeltas = numDeltas + 1;
	if sumDeltas > 0.5 then
		local avgDeltas = sumDeltas / numDeltas;
		text = string.format("%2.2fms %3.2f%%",avgDeltas * 1000,(avgDeltas * 100000) / 16.5);
		sumDeltas = 0;
		numDeltas = 0;
	end
end

function CreateGround()
	groundTex = GetAsset("blend");
	dirt = GetAsset("dirt");
	grass = GetAsset("grass");
	stone = GetAsset("stone");
	heightmap = GetAsset("ground");
	SetHeightmapScale(heightmap,2,0.002,2);
	ground = GetHeightmapModel(heightmap);
	groundEffect = CreateEffect("multitex");
	
	SetEffectParam(groundEffect,0,grass);  --tex1
	SetEffectParam(groundEffect,1,dirt);   --tex2
	SetEffectParam(groundEffect,2,stone);  --tex3
	
	SetMeshTexture(ground,0,groundTex);
	SetMeshEffect(ground,0,groundEffect);	
end

function CreateWater()
	watervtx = { 
		{-1, 0,-1,   0, 1, 0,   1, 0, 0, 1,   0, 0  },
		{-1, 0, 1,   0, 1, 0,   1, 0, 0, 1,   0, 10 },
		{ 1, 0,-1,   0, 1, 0,   1, 0, 0, 1,   10, 0 },
		{ 1, 0, 1,   0, 1, 0,   1, 0, 0, 1,   10,10 }
	};
	wateridx = { 0, 1, 2, 1, 3, 2 };
	water = CreateModel("PosNorTanTex",watervtx,wateridx);	

	waterEffect = CreateEffect("waterShader");
	waterNormal1 = GetAsset("wave0");
	waterNormal2 = GetAsset("wave1");
	skymap = GetAsset("sky");
	
	SetMeshEffect(water,0,waterEffect);
	SetMeshNormal(water,0,waterNormal1);
	SetMeshCubeMap(water,0,skymap);
	SetEffectParam(waterEffect,0,waterNormal2);
end

function CreateCube()
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
	return CreateModel("Pos",vtx,idx);
end

function Begin()
	
	slowTicket = LoadAssets(
		{"ProjectDir","skull3.obj","WavefrontModel","skull"},
		{"ProjectDir","castle.obj","WavefrontModel","castle"}
		--{"ProjectDir","vase.obj","WavefrontModel","vase"},
		--{"ProjectDir","oildrum.obj","WavefrontModel","oildrum"}
	);
	
	vaseModel=FutureAsset("ProjectDir","vase.obj","WavefrontModel");
	vaseModel:WhenReady(SetModelPosition,0,0.152,0.33);
	vaseModel:WhenReady(SetModelScale,0.0002);
	
	barrelModel=FutureAsset("ProjectDir","oildrum.obj","WavefrontModel");
	barrelModel:WhenReady(SetModelPosition,0.06,0.152,0.33);
	barrelModel:WhenReady(SetModelScale,0.015);
	
	fastTicket = LoadAssets(
		{"ProjectDir","IngenuityLogo.png","Tex2D","logo"},
		
		-- Sky map and shader
		{"ProjectDir","grassCubeMap.dds","CubeMap","sky"},
		{"FrameworkDir","SkyShader.xml","Shader","skyShader"},
		
		-- Ground model, textures and shader
		{"ProjectDir","castlehm257.raw","RawHeightMap","ground"},
		{"ProjectDir","blend_castle.dds","Tex2D","blend"},
		{"ProjectDir","dirt.dds","Tex2D","dirt"},
		{"ProjectDir","grass.dds","Tex2D","grass"},
		{"ProjectDir","stone.dds","Tex2D","stone"},
		{"FrameworkDir","MultiTextureAnimY.xml","Shader","multitex"},
		
		-- Water textures and shader
		{"ProjectDir","wave0.dds","Tex2D","wave0"},	
		{"ProjectDir","wave1.dds","Tex2D","wave1"},
		{"FrameworkDir","WaterShader.xml","Shader","waterShader"}
	);
	
	surface = CreateSurface(); -- fullscreen
	surface2 = CreateSurface();
	
	-- vase = GetModel("vase.obj");
	-- barrel = GetModel("oildrum.obj");
	
	model = 0;
	castle = 0;
	modelx = 2.0;
	camRadius = 2;
	rotSpeed = 1.5;
	moveSpeed = 0.1;
	waterShift = 0;
	
	camera = CreateCamera();
	SetCameraClipFov(camera,0.001,50,0.78539);
	font = GetFont(40,"Arial");
	light = CreateLight("directional");
	
	camerax = 0.23;
	cameraz = -0.26;
	
	print("Script Started");
end

function Reload()
	--loadComplete = false; Begin();
	-- SetCameraClipFov(camera,0.001,50,0.78539);
	print("Reloaded");
end

function Update(delta)
	if fastTicket > -1 and IsLoaded(fastTicket) then
		print("Fast Ticket Loaded!!");
	
		CreateGround()
		CreateWater()
		
		skyEffect = CreateEffect("skyShader");
		skyBox = CreateCube();
		SetModelScale(skyBox,30);
		SetMeshCubeMap(skyBox,0,skymap);
		SetMeshEffect(skyBox,0,skyEffect);
		
		logoSprite = GetAsset("logo");
		
		fastTicket = -1;
	end

	if rotLeft then
		modelx = modelx + (delta * rotSpeed);
	elseif rotRight then
		modelx = modelx - (delta * rotSpeed);
	end
	
	if heightmap then
		height = GetHeightmapHeight(heightmap,camerax,cameraz);
	else
		height = 0;
	end
	
	--if(modelx > (math.pi * 2)) then modelx = modelx - (math.pi * 2); end
	--SetCameraPosition(camera,math.sin(modelx) * camRadius,0.75 * camRadius,math.cos(modelx) * camRadius);
	SetCameraPosition(camera,camerax,height + 0.013,cameraz);
	SetCameraTarget(camera,camerax + math.cos(modelx),height + 0.02,cameraz + math.sin(modelx));
	
	if moveUp then
		camerax = camerax + (math.cos(modelx) * moveSpeed * delta);
		cameraz = cameraz + (math.sin(modelx) * moveSpeed * delta);
	end
	if moveDown then
		camerax = camerax - (math.cos(modelx) * moveSpeed * delta);
		cameraz = cameraz - (math.sin(modelx) * moveSpeed * delta);
	end	
	
	--SetCameraPosition(camera,0,3,-0.001);
	--SetLightDirection(light,math.sin(modelx-0.5),0.75,math.cos(modelx-0.5));
	SetLightDirection(light,math.sin(2.5),0.75,math.cos(2.5));
	
	progressText = string.format("%1.3f complete",GetLoadProgress(slowTicket));
	if not loadComplete and IsLoaded(slowTicket) then
		model = GetAsset("skull");
		castle = GetAsset("castle");
		--vase = GetAsset("vase");
		--barrel = GetAsset("oildrum");
		--SetModelPosition(model,0,-0.25,0);
		loadComplete = true;
	end
	
	if loadComplete then
		SetModelScale(model,0.01);
		SetModelPosition(model,0.03,0.15,0.37);
		
		SetModelScale(castle,0.0038);
		SetModelPosition(castle,0.03,0.15,0.33);
		
		--SetModelPosition(vase,0,0.152,0.33);
		--SetModelScale(vase,0.0002);
		
		--SetModelPosition(barrel,0.06,0.152,0.33);
		--SetModelScale(barrel,0.015);
	end
	
	positionText = string.format("%1.3f,%1.3f,%1.3f",camerax,cameraz,modelx);
	
	if water then
		SetModelScale(water,0.8);
		SetModelPosition(water,0,0.13,0.13);
		
		waterShift = waterShift + (delta * 0.03);
		if waterShift > 1 then waterShift = waterShift - 1 end
		SetEffectParam(waterEffect,1,-waterShift);
		SetEffectParam(waterEffect,2,waterShift);
		SetEffectParam(waterEffect,3,waterShift);
		SetEffectParam(waterEffect,4,waterShift);
	end
	
	moveUp = GetKeyState(72); 
	moveDown = GetKeyState(80);
	rotLeft = GetKeyState(75);
	rotRight = GetKeyState(77);
  
	UpdateFrameTime(delta);
end

function Draw()
	if ground then
		DrawComplexModel(ground,camera,light);
	end
	if water then
		DrawComplexModel(water,camera,light);
	end
	if castle ~= 0 then
		DrawComplexModel(castle,camera,light);
	end
	if model ~= 0 then
		DrawComplexModel(model,camera,light);--,surface);
	end
	if vaseModel:Ready() then
		DrawComplexModel(vaseModel(),camera,light);
	end
	if barrelModel:Ready() then
		DrawComplexModel(barrelModel(),camera,light);
	end
	if skyBox then
		DrawComplexModel(skyBox,camera);
	end
	--ShadeSurface(surface,dotsEffect,surface2);
	--DrawSurface(surface);
	--DrawSprite(test,-0.05,-1);
	DrawText(font,text,0,0,0);
	
	if not loadComplete then
		DrawText(font,progressText,400,0,0);
	end
	
	local screenWidth,screenHeight = GetBackbufferSize();
	
	if logoSprite then
		--DrawSprite(logoSprite,0.05-(screenWidth/screenHeight),0.58);
	end
	--DrawText(font,positionText,700,0,0);
end

function End()
	Unload(ticket);
	print("Script Finished");
end

print("Script Loaded");
