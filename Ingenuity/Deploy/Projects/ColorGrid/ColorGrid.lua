
Require("ProjectDir","../../Common/ColorHelper.lua");

sumDeltas = 0;
numDeltas = 0;
text = "";
drawCallsText = "";
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
	scene = CreateScene("instanced");

	camera = CreateCamera(true); -- orthographic
	
	rect = CreateRect(0,0,10,10);
	
	font = GetFont(20,"Arial");
	
	ticket = LoadAssets("FrameworkDir","PathShader.xml","Shader","PathShader");
	
	prevMouseX = 1;
	prevMouseY = 1;
end

function Update(delta)
	if IsLoaded(ticket) then
		print("Ticket loaded!");
		effect = CreateEffect("PathShader",true);
		SetEffectParam(effect,0,0);
		SetMeshEffect(rect,0,effect);
		ticket = -1;
	end

	mouseX, mouseY = GetMousePosition();
	screenWidth, screenHeight = GetScreenSize();
	
	mouseX = mouseX + 1;
	mouseY = mouseY + 1;
		
	if mouseX ~= prevMouseX or mouseY ~= prevMouseY then
		numDrawCalls = math.ceil(screenWidth/mouseX) * math.ceil(screenHeight/mouseY);
		drawCallsText = string.format("%d Rectangles", numDrawCalls);
		
		CreateRect(0,0,mouseX,mouseY,rect);

		ClearScene(scene);
		
		for x = 0,screenWidth,mouseX do
			for y = 0,screenHeight,mouseY do
				SetMeshPosition(rect,0,x,y,0);
				SetMeshColor(rect,0,HSLToRGB(x/screenWidth,1,0.5 + (0.5 * (y/screenHeight))));
				AddToScene(scene,rect);
			end
		end		
		
		prevMouseX,prevMouseY = mouseX,mouseY;
	end
	
	UpdateFrameTime(delta);
end

function Draw()
	if ticket == -1 then
		screenWidth, screenHeight = GetScreenSize();
		SetCameraClipHeight(camera,1,5000,screenHeight);
		SetCameraPosition(camera,screenWidth/2,-screenHeight/2,-1000);
		SetCameraTarget(camera,screenWidth/2,-screenHeight/2,0);
		
		DrawScene(scene,camera);
		
		DrawText(font,text,0,0,0);
		DrawText(font,drawCallsText,0,25,0);
	end
end

function End()

end