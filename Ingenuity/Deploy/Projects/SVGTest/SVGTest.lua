-- Ingenuity SVG Rendering Demo
--
-- Loads an SVG file to a model and renders it
-- with a custom shader
--
-- Richard Copperwaite 2013

rotation = 0;

sumDeltas = 0;
numDeltas = 0;
text = "";
function UpdateFrameTime(delta)
	sumDeltas = sumDeltas + delta;
	numDeltas = numDeltas + 1;
	if sumDeltas > 0.5 then
		local avgDeltas = sumDeltas / numDeltas;
		text = string.format("%2.2fms %3.2f%%",avgDeltas * 1000,(avgDeltas * 100000) / 16);
		sumDeltas = 0;
		numDeltas = 0;
	end	
end

function Begin()
	SetMultisampling(4);

	svgTicket = LoadAssets(
		{"FrameworkDir", "PathShader.xml", "Shader", "pathShader"},
		{"ProjectDir", "Hummingbird3.svg", "SVGModel", "svgModel"});

	-- svgModel = GetSVGModel("Hummingbird3.svg");
	
	-- rectModel = CreateRect(0,0,1000,1000);
	-- SetMeshColor(rectModel,0,1,0,0,1);
	-- effect = CreateEffect("PathShader",true);
	-- SetEffectParam(effect,0,0.0);
	-- SetMeshEffect(rectModel,0,effect);
	-- SetModelPosition(rectModel,-373,247,0);
	
	--SetMeshColor(svgModel,0,1,0,0,1);
	
	--camera = CreateCamera(true); --orthographic!
	camera = CreateCamera(); --perspective!
	SetCameraPosition(camera,0,0,-1000);
	SetCameraTarget(camera,0,0,0);
	
	font = GetFont(20,"Arial");
	
	print("Script Started");
end

function Reload()
	Begin();
end

function Update(delta)	
	if IsLoaded(svgTicket) then
		svgModel = GetAsset("svgModel");
--		pathEffect = CreateEffect(")
	
		SetModelPosition(svgModel,-373,247,0);
		
		if svgModel then
			local meshCount = GetNumMeshes(svgModel)
			while meshCount > 1 do
				meshCount = meshCount - 1;
				SetMeshPosition(svgModel,meshCount,0.0,0.0,-meshCount * 10.0); -- WEIRDNESS!!!!!!
			end
		end
	
		svgTicket = -1;
	end
	
	UpdateFrameTime(delta);
	--rotation = rotation + (delta*1.0);
	--SetModelRotation(svgModel,0,rotation,rotation);
	mouseX, mouseY = GetMousePosition();
	screenWidth,screenHeight = GetScreenSize();
	ratio = screenWidth/screenHeight;
	camOffsetX = ((mouseX/screenWidth) - 0.5) * 700;
	camOffsetY = ((mouseY/screenHeight) - 0.5) * (700/ratio);
	
	SetCameraPosition(camera,-camOffsetX,camOffsetY,-1000);
	
	--text = string.format("%2.2f, %2.2f",mouseX,mouseY);
	
end

function Draw()
	--SetCameraClipHeight(camera,1,5000,screenHeight);
	--SetCameraPosition(camera,screenWidth/2,-screenHeight/2,-800.0);
	--SetCameraTarget(camera,screenWidth/2,-screenHeight/2,0.0);
	--SetModelPosition(svgModel,screenWidth/2,-screenHeight/2,0.0);
	-- DrawComplexModel(rectModel,camera);
	if svgModel then
		DrawComplexModel(svgModel,camera);	
	end
	DrawText(font,text,0,0,0);
end

function End()
end