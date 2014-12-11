-- Ingenuity Shader Sandbox
--
-- Loads a model and allows the user to apply any shader
-- effect and modify any parameter through GUI controls
--
-- Richard Copperwaite 2013-2015

Require("ProjectDir","../../Common/IngenUtils.lua");
Require("ProjectDir","../../Common/IngenUI.lua");
Require("ProjectDir","../../Common/XmlHelper.lua");

camRadius = 2.5;
lightRadius = 2.5;
rotSpeed = 0.4;
pendingTex = "";
pendingTexIndex = 0;

initialMdlShader = "ProjectorShader.xml";
initialTexShader = "TextureCopy.xml";
initialModel = "vase.obj";

-- State machine states:
STATE_LOADING_TEXTURE_SHADER = 0;
STATE_FINISHED = 1;

function FixupPickedPath(path)
	local searchString = "Projects\\ShaderSandbox\\";
	print(path);
	if string.sub(path,1,string.len(searchString)) == searchString then
		path = string.sub(path,string.len(searchString) + 1);
	end
	return path;
end

function GetShaderType(shaderElement)
	if shaderElement.label == "shader" then
		for arg,argVal in pairs(shaderElement.args) do
			if arg == "type" and argVal == "model" then
				return "model";
			end
			if arg == "type" and argVal == "texture" then
				return "texture";
			end
		end
	end
	return "none";
end

function LoadShader(shaderName)
	LoadFile("FrameworkDir", shaderName, function(data)
		if data and string.len(data) > 0 then
			print("Loaded XML data for '"..shaderName.."'");
			pendingShader = shaderName;
			pendingShaderMeta = collect(data);
			shaderType = GetShaderType(pendingShaderMeta[2]);
			texShaderSelected = (shaderType == "texture");
			shaderTicket = LoadAssets("FrameworkDir",shaderName,"Shader",shaderName);
		else
			print("Failed to load XML data for '"..shaderName.."'");
		end
	end);
end

function CreateStandardParams()
	local specLabel = CreateUILabel();
	specLabel.x = -250;
	specLabel.y = 50;
	specLabel.width = 200;
	specLabel.height = 50;
	specLabel.anchor = INGENUI_ANCHOR_TOP + INGENUI_ANCHOR_RIGHT;
	specLabel.text = "Specular Power";
	AddUIComponent(specLabel);
	
	local specTextBox = CreateUITextBox();
	specTextBox.x = -250;
	specTextBox.y = 100;
	specTextBox.width = 200;
	specTextBox.height = 50;
	specTextBox.anchor = INGENUI_ANCHOR_TOP + INGENUI_ANCHOR_RIGHT;
	specTextBox.text = "12.0";
	specTextBox.submit = function(textbox)
		local textNumber = tonumber(textbox.text);
		if textNumber and model then SetMeshSpecular(model,0,textNumber); end
	end
	AddUIComponent(specTextBox);
	
	local colorLabel = CreateUILabel();
	colorLabel.x = -250;
	colorLabel.y = 150;
	colorLabel.width = 200;
	colorLabel.height = 50;
	colorLabel.anchor = INGENUI_ANCHOR_TOP + INGENUI_ANCHOR_RIGHT;
	colorLabel.text = "RGB Color";
	AddUIComponent(colorLabel);
	
	local redSlider = CreateUISlider();
	redSlider.x = -250;
	redSlider.y = 200;
	redSlider.width = 200;
	redSlider.height = 50;
	redSlider.anchor = INGENUI_ANCHOR_TOP + INGENUI_ANCHOR_RIGHT;
	redSlider.value = 1.0;
	redSlider.action = function(slider)
		modelColorR = slider.value;
		if model then SetMeshColor(model,0,modelColorR,modelColorG,modelColorB) end;
	end
	AddUIComponent(redSlider);
	
	local greenSlider = CreateUISlider();
	greenSlider.x = -250;
	greenSlider.y = 250;
	greenSlider.width = 200;
	greenSlider.height = 50;
	greenSlider.anchor = INGENUI_ANCHOR_TOP + INGENUI_ANCHOR_RIGHT;
	greenSlider.value = 1.0;
	greenSlider.action = function(slider)
		modelColorG = slider.value;
		if model then SetMeshColor(model,0,modelColorR,modelColorG,modelColorB) end;
	end
	AddUIComponent(greenSlider);
	
	local blueSlider = CreateUISlider();
	blueSlider.x = -250;
	blueSlider.y = 300;
	blueSlider.width = 200;
	blueSlider.height = 50;
	blueSlider.anchor = INGENUI_ANCHOR_TOP + INGENUI_ANCHOR_RIGHT;
	blueSlider.value = 1.0;
	blueSlider.action = function(slider)
		modelColorB = slider.value;
		if model then SetMeshColor(model,0,modelColorR,modelColorG,modelColorB) end;
	end
	AddUIComponent(blueSlider);
end

function Begin()
	
	pendingModel = initialModel;
	modelTicket = LoadAssets("ProjectDir",pendingModel,"WavefrontModel",pendingModel);
	
	-- Enumerate the Framework Directory
	EnumerateDirectory("FrameworkDir");
	
	modelx = 0.0;
	camera = CreateCamera();
	SetCameraPosition(camera, 0, camRadius, -camRadius);
	orthoCam = CreateCamera(true);
	SetCameraPosition(orthoCam, 0, 0, -1);
	font = GetFont(40,"Arial");
	-- light = CreateLight("directional");
	light = CreateLight("spot");
	--table.insert(lights,light);
	print("Script Started");
	
	modelColorR = 1.0;
	modelColorG = 1.0;
	modelColorB = 1.0;
	
	CreateUI();
	uiVisible = true;
	
	leftPanel = CreateUIPanel();
	leftPanel.x = 50;
	leftPanel.y = 50;
	leftPanel.width = 200;
	leftPanel.height = 1000;
	AddUIComponent(leftPanel);
	
	paramPanel = CreateUIPanel();
	paramPanel.x = -250;
	paramPanel.y = 50;
	paramPanel.width = 200;
	paramPanel.height = 1000;
	paramPanel.anchor = INGENUI_ANCHOR_TOP + INGENUI_ANCHOR_RIGHT;
	AddUIComponent(paramPanel);
	
	mdlLabel = CreateUILabel();
	mdlLabel.width = 200;
	mdlLabel.height = 50;
	mdlLabel.text = "Model";
	AddUIComponent(mdlLabel,leftPanel);
	
	mdlButton = CreateUIButton();
	mdlButton.y = 50;
	mdlButton.width = 200;
	mdlButton.height = 50;
	mdlButton.text = "";
	mdlButton.action = function(button)
		PickFile("ProjectDir","*.obj",function(data)
			if data and string.len(data) > 0 then
				data = FixupPickedPath(data);
				pendingModel = data;
				print("Loading Model: "..data);
				modelTicket = LoadAssets("ProjectDir",pendingModel,"WavefrontModel",pendingModel);
			end
		end);
	end
	AddUIComponent(mdlButton,leftPanel);
	
	mdlShaderLabel = CreateUILabel();
	mdlShaderLabel.y = 100;
	mdlShaderLabel.width = 200;
	mdlShaderLabel.height = 50;
	mdlShaderLabel.text = "Model Shader";
	AddUIComponent(mdlShaderLabel,leftPanel);
	
	mdlShaderButton = CreateUIButton();
	mdlShaderButton.y = 150;
	mdlShaderButton.width = 200;
	mdlShaderButton.height = 50;
	mdlShaderButton.text = "";
	mdlShaderButton.action = function(button)
		PickFile("FrameworkDir","*.xml",function(data)
			-- This won't work in Windows 8!
			if data and string.len(data) > 0 then
				LoadShader(data);
			end
		end);
	end
	AddUIComponent(mdlShaderButton,leftPanel);
	
	texShaderLabel = CreateUILabel();
	texShaderLabel.y = 200;
	texShaderLabel.width = 200;
	texShaderLabel.height = 50;
	texShaderLabel.text = "Screen Shader";
	AddUIComponent(texShaderLabel,leftPanel);
	
	texShaderButton = CreateUIButton();
	texShaderButton.y = 250;
	texShaderButton.width = 200;
	texShaderButton.height = 50;
	texShaderButton.text = "";
	texShaderButton.action = function(button)
		PickFile("FrameworkDir","*xml",function(data)
			if data and string.len(data) > 0 then
				LoadShader(data);
			end
		end);
	end
	AddUIComponent(texShaderButton,leftPanel);
	
	CreateStandardParams();
	
	rotSlider = CreateUISlider();
	rotSlider.x = -150;
	rotSlider.y = -80;
	rotSlider.width = 300;
	rotSlider.height = 50;
	rotSlider.anchor = INGENUI_ANCHOR_BOTTOM + INGENUI_ANCHOR_CENTERX;
	rotSlider.minValue = math.pi;
	rotSlider.maxValue = -math.pi;
	rotSlider.value = 0;
	rotSlider.action = function(slider)
		SetCameraPosition(camera,math.sin(slider.value) * camRadius, camRadius, -math.cos(slider.value) * camRadius);
	end;
	AddUIComponent(rotSlider);
	
	maxAmplitude = 0;
	
	sphere = CreateModel("PosNor", CreateSphere());
	SetMeshColor(sphere,0,0,0,1,0.5);
	SetModelPosition(sphere,0,0.35,0);
	SetModelScale(sphere,2);
	
	screenSurface1 = CreateSurface();
	screenSurface2 = CreateSurface();
	
	SetClearColor(1,1,1);
	
	
	LoadShader(initialTexShader);
	
	state = STATE_LOADING_TEXTURE_SHADER;
end

function Reload()
	-- Begin();
	
	print("reloaded!");
end

function ShaderLoaded()
	print("SHADER LOADED!");
	if texShaderSelected then
		local tempScreenEffect = CreateEffect(pendingShader);
		if tempScreenEffect then
			print("APPLYING SCREEN SHADER!");
			screenEffect = tempScreenEffect;
		end
		texShaderButton.text = pendingShader:sub(-20);
	else
		modelEffect = CreateEffect(pendingShader);
		if modelEffect and model then
			print("APPLYING MODEL SHADER!");
			SetMeshEffect(model,0,modelEffect);
		end
		if pendingShader == "MatcapShader.xml" and matcapTex then
			SetEffectParam(modelEffect,0,matcapTex);
		end
		mdlShaderButton.text = pendingShader:sub(-20);
	end
	
	ClearUIComponents(paramPanel);
	CreateParamButtons(pendingShaderMeta);
	
	pendingShader = "";
	pendingShaderMeta = nil;
end

function CreateParamButtons(xmlStruct)
	local paramButtonOffset = 300;
	local shaderElement = xmlStruct[2];
	local paramsElement = nil;
	for i = 1,shaderElement.n do
		if shaderElement[i].label == "params" then
			paramsElement = shaderElement[i];
			break;
		end
	end
	if paramsElement then
		for i = 1,paramsElement.n do
			local displayName = paramsElement[i].args["displayName"];
			local label = CreateUILabel();
			label.y = paramButtonOffset;
			label.width = 200;
			label.height = 50;
			label.text = displayName or "displayName??";
			
			AddUIComponent(label,paramPanel);
			
			paramButtonOffset = paramButtonOffset + 50;
			
			local paramType = paramsElement[i].args["type"];
			local paramIndexString = paramsElement[i].args["index"];
			local paramIndex = 0;
			if paramIndexString then paramIndex = tonumber(paramIndexString) end;
			
			if paramType and paramType == "tex2D" then
				local button = CreateUIButton();
				button.y = paramButtonOffset;
				button.width = 200;
				button.height = 200;
				button.action = function(button)
					PickFile("ProjectDir","*.png;*.jpg;*.gif;*.dds",function(data)
						if data and string.len(data) > 0 then
							data = FixupPickedPath(data);
							pendingTex = data;
							pendingTexButton = button;
							pendingTexIndex = button.paramIndex;
							print("Loading Texture: "..pendingTex);
							texTicket = LoadAssets("ProjectDir",pendingTex,"Texture",pendingTex);
						end
					end);
				end
				button.text = "[Select Texture...]"
				button.paramIndex = paramIndex;
				
				AddUIComponent(button,paramPanel);
				paramButtonOffset = paramButtonOffset + 200;
				
				if displayName == "MatCap" then
					pendingTex = "matcaps/metal_foundry.png";
					pendingTexButton = button;
					pendingTexIndex = 0;
					texTicket = LoadAssets("ProjectDir",pendingTex,"Texture",pendingTex);
				end
				
				if displayName == "ProjectionTex" then
					pendingTex = "Flashlight.png";
					pendingTexButton = button;
					pendingTexIndex = 0;
					texTicket = LoadAssets("ProjectDir",pendingTex,"Texture",pendingTex);
				end
			end
			
			if paramType and paramType == "tex3D" then
				local button = CreateUIButton();
				button.y = paramButtonOffset;
				button.width = 200;
				button.height = 50;
				button.action = function(button)
					PickFile("ProjectDir","*.dds",function(data)
						if data and string.len(data) > 0 then
							data = FixupPickedPath(data);
							pendingTex = data;
							pendingTexIndex = button.paramIndex;
							print("Loading 3D Texture: "..pendingTex);
							texTicket = LoadAssets("ProjectDir",pendingTex,"VolumeTex",pendingTex);
						end
					end);
				end
				button.text = "[Select 3D Texture...]"
				button.paramIndex = paramIndex;
				
				AddUIComponent(button,paramPanel);
				paramButtonOffset = paramButtonOffset + 50;
			end
			
			if paramType and paramType == "float" then
				local paramDefault = paramsElement[i].args["default"];
				local paramMin = paramsElement[i].args["min"];
				local paramMax = paramsElement[i].args["max"];
				if paramMin and paramMax then
					local slider = CreateUISlider();
					slider.y = paramButtonOffset;
					slider.width = 200;
					slider.height = 50;
					slider.value = tonumber(paramDefault);
					slider.minValue = tonumber(paramMin);
					slider.maxValue = tonumber(paramMax);
					slider.action = function(slider)
						if texShaderSelected then
							SetEffectParam(screenEffect, slider.paramIndex, slider.value);
						else
							SetEffectParam(modelEffect, slider.paramIndex, slider.value);
						end
					end
					slider.paramIndex = paramIndex;
					
					AddUIComponent(slider,paramPanel);
					paramButtonOffset = paramButtonOffset + 50;
				end
			end
		end
	end
end

function Update(delta)
	if state == STATE_LOADING_TEXTURE_SHADER then
		if pendingShader == "" then
			LoadShader(initialMdlShader)
			state = STATE_FINISHED;
		end
	end
	
	if modelTicket and IsLoaded(modelTicket) then
		model = GetAsset(pendingModel);
		if model then
			print("Model Loaded! "..pendingModel);
			
			mdlButton.text = pendingModel;
			
			if modelEffect then
				SetMeshEffect(model,0,modelEffect);
			end
			
			SetMeshColor(model,0,modelColorR,modelColorG,modelColorB);
			
			modelX, modelY, modelZ, modelR = GetMeshBounds(model,0);
			local modelScale = 1/modelR;
			SetModelScale(model,modelScale);
			SetModelPosition(model,modelScale * -modelX, (modelScale * -modelY) + 0.35, modelScale * -modelZ);
		end
		modelTicket = nil;
	end
	
	if texTicket and IsLoaded(texTicket) then
		loadedTex = GetAsset(pendingTex);
		if loadedTex then
			if texShaderSelected then
				SetEffectParam(screenEffect, pendingTexIndex, loadedTex);
			else
				SetEffectParam(modelEffect, pendingTexIndex, loadedTex);
			end
			if pendingTexButton then
				pendingTexButton.image = loadedTex;
				pendingTexButton.text = "";
			end
		end
		texTicket = nil;
	end
	
	if shaderTicket and IsLoaded(shaderTicket) then
		ShaderLoaded();
		shaderTicket = nil;
	end
	
	ingenUiPanel.texture = GetSurfaceTexture(screenSurface2);

	modelx = modelx + (delta * rotSpeed);
	if(modelx > (math.pi * 2)) then modelx = modelx - (math.pi * 2); end
	-- if model then
		-- SetModelPosition(model,0,0,0);
		-- SetCameraPosition(camera,math.sin(modelx) * camRadius, camRadius, math.cos(modelx) * camRadius);
	-- end
	SetLightDirection(light,math.sin(modelx-0.5),-0.4,math.cos(modelx-0.5));
	SetLightPosition(light,-math.sin(modelx-0.5) * lightRadius,0.5 * lightRadius,-math.cos(modelx-0.5) * lightRadius);
	
	screenWidth, screenHeight = GetScreenSize();
	SetCameraClipHeight(orthoCam,1,5000,screenHeight);
	
	-- SetModelPosition(square,(screenWidth/2)-160,screenHeight/2-32,0);
	
	if uiVisible then
		UpdateUI(delta);
	end
	UpdateFrameTime(delta);
	
	if modelTicket and numDeltas == 0 then
		local loadPercent = GetLoadProgress(modelTicket) * 100;
		frameTimeText = frameTimeText .. string.format("  Load: %2.2f%%", loadPercent);
	end
	
	amplitude = GetAmplitude();
	if amplitude > maxAmplitude then maxAmplitude = amplitude end;
	amplitudeString = string.format("Amp: %2.4f  Max: %2.4f", amplitude, maxAmplitude);
	
	local discard,keyDown,discard = GetKeyState("u");
	if keyDown then uiVisible = not uiVisible end
end

function Draw()
	if model then
		DrawComplexModel(model,camera,light,screenSurface1);
	end
	
	-- DrawComplexModel(sphere,camera,0,screenSurface1);
	
	if screenEffect then
		ShadeSurface(screenSurface1,screenEffect,screenSurface2);
	end
	
	if uiVisible then
		DrawUI();
	end
	
	--DrawSprite(tex,0,0);
	DrawText(font,frameTimeText,0,0,0);
	-- DrawText(font,amplitudeString,0,40,0);
	
	ClearSurface(screenSurface1,1,1,1);
	ClearSurface(screenSurface2,1,1,1);
end

function End()
	print("Script Finished");
end