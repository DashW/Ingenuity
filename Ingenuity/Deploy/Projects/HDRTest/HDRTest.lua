-- HDR Lighting Test

Require("ProjectDir","Scene.lua");
Require("ProjectDir","../../Common/IngenHDR.lua");
Require("ProjectDir","../../Common/IngenUtils.lua");

function Begin()
	BeginScene();
	BeginHDR();
	
	camera = CreateCamera();
	SetCameraClipFov(camera, 0.2, 30, math.pi/4);
	SetupFlyCamera(camera, 0, 0.4, -9, 0.01, 10);
	
	sceneTarget = CreateSurface(1, 1, true, "3x10f");
	
	debugFont = GetFont(40,"Arial");
end

function Reload()
end

function Update(delta)
	deltaTime = delta;
	
	UpdateScene(delta);
	UpdateHDR(delta);
	UpdateFlyCamera(delta);
	--UpdateFrameTime(delta);
end

function DrawTimestamp(name,font,yOffset)
	local cpuTime = GetTimestampData(name,true) * 1000;
	local gpuTime = GetTimestampData(name,false) * 1000;
	--local text = name..": CPU: "..cpuTime.." GPU: "..gpuTime;
	local text = string.format("CPU: %1.2f GPU: %1.2f", cpuTime, gpuTime);
	DrawText(font,name..":",0,yOffset,0);
	DrawText(font,text,200,yOffset,0);
end

function Draw()
	ClearSurface(sceneTarget);
	-- ClearSurface(downSampleTarget);
	
	BeginTimestamp("scene",true,true);
	DrawScene(camera,sceneTarget);
	EndTimestamp("scene",true,true);
	
	BeginTimestamp("HDR",true,true);
	finalSurface = ShadeHDR(sceneTarget,deltaTime);
	EndTimestamp("HDR",true,true);
	
	DrawSurface(finalSurface);
	
	--DrawText(debugFont,frameTimeText,0,0,0);
	DrawTimestamp("scene",debugFont,40);
	DrawTimestamp("HDR",debugFont,80);
	DrawTimestamp("tonemapping",debugFont,120);
	DrawTimestamp("brightpass",debugFont,160);
	DrawTimestamp("star",debugFont,200);
	DrawTimestamp("bloom",debugFont,240);
	DrawTimestamp("merge",debugFont,280);
	DrawTimestamp("detail",debugFont,320);
end

function End()
end