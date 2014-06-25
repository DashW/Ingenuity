
Require("ProjectDir","../../Common/ColorHelper.lua");
Require("ProjectDir","../../Common/IngenHDR.lua");

function Begin()
	camera = CreateCamera(true);
	SetCameraClipHeight(camera,1,5000,10);
	SetCameraPosition(camera,0,0,-1000);
	SetCameraTarget(camera,0,0,0);
	
	ticket = LoadAssets(
		{"FrameworkDir","PathShader.xml","Shader","PathShader"},
		{"ProjectDir","Space Fighter Loop.wav","WavAudio","SoundFile"}
	);

	rect = CreateRect( -5, -5, 10, 10 );
	progressRect = CreateRect( 0, 0, 1, 1 );
	
	debugFont = GetFont(30,"Arial");
end

function Reload()
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
			
			PlaySound(soundFile);
		end
		
		ticket = -1;
	end

	local mouseX, mouseY = GetMousePosition();
	local screenWidth, screenHeight = GetScreenSize();
	local hue = mouseY / screenHeight;
	local size = 0;
	if ticket == -1 then
		size = (GetAmplitude(soundFile) + GetAmplitude(soundFile) + GetAmplitude(soundFile)) / 3;
	end
	
	SetClearColor(HSLToRGB(hue,1,0.5));
	SetMeshColor(rect,0,HSLToRGB(1-hue,1,0.5));
	SetModelScale(rect,size);
	SetMeshColor(progressRect,0,HSLToRGB(1-hue,1,0.5))
	
	if soundFile then
		soundProgress = GetSoundProgress(soundFile);
		SetModelPosition(progressRect,-5 * (screenWidth/screenHeight), 4, 0);
		SetModelScale(progressRect,(screenWidth/screenHeight) * (soundProgress/soundDuration) * 10, 1, 0);
	end
end

function Draw()
	if ticket == -1 then
		DrawComplexModel(rect,camera);
		DrawComplexModel(progressRect,camera);
		if soundFile then
			DrawText(debugFont, string.format("%2.2f/%2.2f",soundProgress,soundDuration), 0, 0, 0);
		end
	end
end

function End()
end