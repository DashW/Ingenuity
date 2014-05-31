
Require("ProjectDir","../../Common/ColorHelper.lua");

function Begin()
	camera = CreateCamera(true);
	SetCameraClipHeight(camera,1,5000,10);
	SetCameraPosition(camera,0,0,-1000);
	SetCameraTarget(camera,0,0,0);
	
	ticket = LoadAssets("FrameworkDir","PathShader.xml","Shader","PathShader");

	rect = CreateRect( -5, -5, 10, 10 );
end

function Reload()
	Begin()
end

function Update(delta)
	if IsLoaded(ticket) then
		print("Ticket Loaded!");
		effect = CreateEffect("PathShader",true);
		SetEffectParam(effect,0,0);
		SetMeshEffect(rect,0,effect);
		ticket = -1;
	end

	local mouseX, mouseY = GetMousePosition();
	local screenWidth, screenHeight = GetScreenSize();
	local hue = mouseY / screenHeight;
	local size = mouseX / screenWidth;
	
	SetClearColor(HSLToRGB(hue,1,0.5));
	SetMeshColor(rect,0,HSLToRGB(1-hue,1,0.5));
	SetModelScale(rect,size);
end

function Draw()
	if ticket == -1 then
		DrawComplexModel(rect,camera);
	end
end

function End()
end