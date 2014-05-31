-- Ingenuity Model Rendering Demo
--
-- Simply loads a model and rotates it about 360 degrees
--
-- Richard Copperwaite 2013

text = "";
camRadius = 200.0;
sumDeltas = 0;
numDeltas = 0;
--lights = {};
rotSpeed = 0.4;
ticket = -1;

function Begin()
	--tex = LoadTexture("PixTest.png");
	-- model = GetModel("vase.obj");
	ticket = LoadAssets("ProjectDir","vase.obj","WavefrontModel","vase");
	modelx = 0.0;
	camera = CreateCamera();
	font = GetFont(40,"Arial");
	light = CreateLight("directional");
	--table.insert(lights,light);
	print("Script Started");
end

function Reload()
	Begin();
end

function Update(delta)
	if ticket > -1 and IsLoaded(ticket) then
		model = GetAsset("vase");
		ticket = -1;
	end

	modelx = modelx + (delta * rotSpeed);
	if(modelx > (math.pi * 2)) then modelx = modelx - (math.pi * 2); end
	--SetModelPosition(model,0,0,0);
	SetCameraPosition(camera,math.sin(modelx) * camRadius,200.0,math.cos(modelx) * camRadius);
	SetLightDirection(light,math.sin(modelx-0.5),0.25,math.cos(modelx-0.5));
	
	sumDeltas = sumDeltas + delta;
	numDeltas = numDeltas + 1;
	if sumDeltas > 0.5 then
		local avgDeltas = sumDeltas / numDeltas;
		text = string.format("%2.4fms",avgDeltas * 1000);
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
	if model then
		DrawComplexModel(model,camera,light);
	end
	--DrawSprite(tex,0,0);
	DrawText(font,text,0,0,0);
end

function End()
	print("Script Finished");
end