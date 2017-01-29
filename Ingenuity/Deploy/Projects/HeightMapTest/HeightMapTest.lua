-- Ingenuity Multitexture Heightmap Demo
--
-- Loads a heightmapped model and renders it
-- with a custom multitexturing shader
--
-- Richard Copperwaite 2013

text = "";
sumDeltas = 0;
numDeltas = 0;
function UpdateFrameTime(delta)
	sumDeltas = sumDeltas + delta;
	numDeltas = numDeltas + 1;
	if sumDeltas > 0.5 then
		local avgDeltas = sumDeltas / numDeltas;
		text = string.format("%2.2fms %3.2f%%",avgDeltas * 1000,(avgDeltas * 100000) / 16.65);
		sumDeltas = 0;
		numDeltas = 0;
	end
end

function Begin()
	ticket = LoadAssets(
		{ "ProjectDir", "hmblend.dds", "Tex2D", "blend" },
		{ "ProjectDir", "dirt.dds", "Tex2D", "dirt" },
		{ "ProjectDir", "stone.dds", "Tex2D", "stone" },
		{ "ProjectDir", "grass.dds", "Tex2D", "grass" },
		{ "ProjectDir", "hmheight.raw", "RawHeightMap", "hmheight" },
		{ "FrameworkDir", "MultiTextureAnimY.xml", "Shader", "landShader" }
	);
	initialised = false;
	modelx = 0.0;
	camRadius = 2.0;
	rotSpeed = 0.4;
	camera = CreateCamera();
	font = GetFont(40,"Arial");
	light = CreateLight("directional");
	SetClearColor(1,1,1);
	print("Script Started");
end

function Reload()
	
end

function Initialise()
	effect = CreateEffect(GetAsset("landShader"));
	SetEffectParam(effect,0,GetAsset("grass"));  --tex1
	SetEffectParam(effect,1,GetAsset("dirt")); --tex2
	SetEffectParam(effect,2,GetAsset("stone")); --tex3
	SetEffectParam(effect,3,0.0);               --yStart
	SetEffectParam(effect,4,1.0);               --yProgress
	
	model = GetHeightmapModel(GetAsset("hmheight"));
	SetMeshTexture(model,0,GetAsset("blend"));
	SetMeshEffect(model,0,effect);
	SetModelScale(model,2,0.004,2);
	
	initialised = true;
end

function Update(delta)
	if not initialised and IsLoaded(ticket) then
		Initialise();
	end

	modelx = modelx - (delta * rotSpeed);
	--if(modelx > (math.pi * 2)) then modelx = modelx - (math.pi * 2); end
	SetCameraPosition(camera,math.sin(modelx) * camRadius,camRadius,math.cos(modelx) * camRadius);
	SetLightDirection(light,math.sin(modelx-0.5),0.75,math.cos(modelx-0.5));
	
	UpdateFrameTime(delta);
end

function PrintGlobals()
	for k,v in pairs(_G) do
		print("Global key", k, "value", tostring(v))
	end
end

function Draw()
	if initialised then
		DrawComplexModel(model,camera,light);
	end
	
	DrawText(font,text,0,0,0);
end

function End()
	print("Script Finished");
end