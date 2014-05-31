require("Common.ColorHelper");

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
	image = LoadImage("pic3.jpg");
	imageWidth, imageHeight = GetImageSize(image);
	effect = CreateEffect("PathShader");
	SetEffectParam(effect,0,0);
	SetMeshEffect(rect,0,effect);
	
	prevMouseX = 0
	prevMouseY = 0
end

function Update(delta)
	mouseX, mouseY = GetMousePosition();
	screenWidth, screenHeight = GetScreenSize();
	
	mouseX = mouseX + 1;
	mouseY = mouseY + 1;
		
	if mouseX ~= prevMouseX or mouseY ~= prevMouseY then
		local tileCount = math.floor(screenWidth/math.max(mouseX, 5));
		
		drawCallsText = string.format("%d Rectangles", tileCount * tileCount);
		
		local rectSize = screenWidth/tileCount
		local sampleSize = imageWidth/tileCount
		if imageHeight < imageWidth then
			sampleSize = imageHeight/tileCount
		end
		
		CreateRect(0,0,rectSize,rectSize,rect);

		ClearScene(scene);
		
		for x = 0,tileCount-1,1 do
			for y = 0,tileCount-1,1 do
				local r,g,b = GetImagePixel(image,x*sampleSize,imageHeight-1-(y*sampleSize));
				SetMeshPosition(rect,0,x*rectSize,y*rectSize,0);
				SetMeshColor(rect,0,r,g,b);
				AddToScene(scene,rect);
			end
		end		
		
		prevMouseX,prevMouseY = mouseX,mouseY;
	end
	
	UpdateFrameTime(delta);
end

function Draw()
	screenWidth, screenHeight = GetScreenSize();
	SetCameraClipHeight(camera,1,5000,screenHeight);
	SetCameraPosition(camera,screenWidth/2,-screenHeight/2,-1000);
	SetCameraTarget(camera,screenWidth/2,-screenHeight/2,0);
	
	DrawScene(scene,camera);
	
	DrawText(font,text,0,0,0);
	DrawText(font,drawCallsText,0,25,0);
end

function End()

end