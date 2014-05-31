-- Ingenuity Shape-Drawing Demo
--
-- Tests out the line and 2D shape drawing
-- functionality
--
-- Richard Copperwaite 2013

text = "";
numDeltas = 0;
sumDeltas = 0;
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
	camera = CreateCamera(true); -- orthographic
	--camera = CreateCamera();	
	--SetModelPosition(rectModel,0,0,20);
	
	ticket = LoadAssets({"FrameworkDir","PathShader.xml","Shader","pathShader"});
	
	mouseDownX = -1; mouseDownY = -1;
	drawingModel = nil;
	models = {};
	
	font = GetFont(20,"Arial");
	print("Script Started");
end

function Update(delta)
	UpdateFrameTime(delta);
	
	if IsLoaded(ticket) then
		effect = CreateEffect("pathShader",true);
		if effect then
			SetEffectParam(effect,0,0);
		end
		
		ticket = -1;
	end
	
	if effect then
		if GetMouseLeft() then
			mouseX, mouseY = GetMousePosition();
			if drawingModel == nil then
				mouseDownX = mouseX; mouseDownY = mouseY;
				drawingModel = CreateEllipseStroke(mouseDownX,mouseDownY,1,1,4);
				if drawingModel ~= nil then
					SetMeshColor(drawingModel,0,math.random(),math.random(),math.random(),1);
					SetMeshEffect(drawingModel,0,effect);
				end
			else
				CreateEllipseStroke(mouseDownX,mouseDownY,math.max(1,mouseX - mouseDownX),math.max(1,mouseY - mouseDownY),4,drawingModel); -- CREATERECT. HMM. 
			end
		else
			if drawingModel ~= nil then
				print("Created Model!");
				table.insert(models,drawingModel);
				drawingModel = nil;
			end
		end
	end
	
	_ignore, rPressed = GetKeyState("r");
	if rPressed then
		models = {};
	end
end

function Draw()
	screenWidth, screenHeight = GetScreenSize();
	SetCameraClipHeight(camera,1,5000,screenHeight);
	SetCameraPosition(camera,screenWidth/2,-screenHeight/2,-1000);
	SetCameraTarget(camera,screenWidth/2,-screenHeight/2,0);	
	-- SetCameraPosition(camera,0,0,-1000);
	-- SetCameraTarget(camera,0,0,0.0);
	-- SetModelPosition(rectModel,screenWidth/2,-screenHeight/2,0.0);	
	for i = 1,#models do
		DrawComplexModel(models[i],camera);
	end
	if drawingModel ~= nil then
		DrawComplexModel(drawingModel,camera);
	end
	DrawText(font,text,0,0,0);
end

function End()
	print("Script Finished");
end