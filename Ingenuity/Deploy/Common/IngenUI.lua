--
-- Ingenuity User Interface Component Framework
--

-- Component Types
INGENUI_PANEL   = 0;
INGENUI_LABEL   = 1;
INGENUI_BUTTON  = 2;
INGENUI_TEXTBOX = 3;
INGENUI_SLIDER  = 4;
-- INGENUI_SCROLLPANEL = 5;

-- Anchoring Types
INGENUI_ANCHOR_LEFT = 0;
INGENUI_ANCHOR_CENTERX = 1;
INGENUI_ANCHOR_RIGHT = 2;
INGENUI_ANCHOR_TOP = 0;
INGENUI_ANCHOR_CENTERY = 3;
INGENUI_ANCHOR_BOTTOM = 6;

function GetUIPosition(component,parent)
	local positionX = (((component.anchor % 3) / 2) * parent.width) + parent.displayX + component.x;
	local positionY = ((math.floor(component.anchor / 3) / 2) * parent.height) + parent.displayY + component.y;
	return positionX, positionY;
end

-- UI PANEL

function UpdateUIPanel(panel,parent,delta)
	for key, component in pairs(panel.components) do
		component.displayX,component.displayY = GetUIPosition(component,panel);
		component.Update(component,panel,delta);
	end
end
function DrawUIPanel(panel)
	local panelX, panelY = panel.displayX, panel.displayY;
	if panel.texture then
		SetModelPosition(ingenUiTexSquare,panelX,-panelY,100);
		SetModelScale(ingenUiTexSquare,panel.width,panel.height,1);
		SetMeshTexture(ingenUiTexSquare,0,panel.texture);
		DrawComplexModel(ingenUiTexSquare,ingenUiCamera);
	end
	for key, component in pairs(panel.components) do
		component.Draw(component);
	end
end
function CreateUIPanel()
	return {
		type = INGENUI_PANEL;
		Update = function(panel,parent,delta) UpdateUIPanel(panel,parent,delta) end;
		Draw   = function(panel) DrawUIPanel(panel) end;
		x = 0;
		y = 0;
		displayX = 0;
		displayY = 0;
		width = 100;
		height = 100;
		anchor = 0; -- TOP LEFT
		components = {};
	}
end

-- UI Label

function DrawUILabel(label)
	local labelX, labelY = label.displayX, label.displayY;
	SetModelPosition(ingenUiSquare,labelX,-labelY,40);
	SetModelScale(ingenUiSquare,label.width,label.height,1);
	SetMeshColor(ingenUiSquare,0,196/255,204/255,204/255);
	DrawComplexModel(ingenUiSquare,ingenUiCamera);
	DrawText(ingenUiFont,label.text,labelX,labelY + (label.height/2) - 15);
end
function CreateUILabel()
	return {
		type = INGENUI_LABEL;
		Update = function(button,parent,delta) end;
		Draw   = function(button) DrawUILabel(button) end;
		x = 0;
		y = 0;
		displayX = 0;
		displayY = 0;
		width = 100;
		height = 100;
		text = "Label";
		anchor = 0; -- TOP LEFT
	}
end

-- UI Button

function UpdateUIButton(button,parent,delta)
	local mouseX, mouseY = GetMousePosition();
	local buttonX, buttonY = button.displayX, button.displayY;
	
	if mouseX > buttonX and mouseY > buttonY 
	and mouseX < buttonX + button.width 
	and mouseY < buttonY + button.height then
		button.hover = true;
	else
		button.hover = false;
	end
	
	local down, pressed, released = GetMouseLeft();
	if pressed and button.hover and not button.clicked then
		button.action(button);
		button.clicked = true;
	end
	if button.clicked and not down then
		button.clicked = false;
	end
end
function DrawUIButton(button)
	local buttonX, buttonY = button.displayX, button.displayY;
	SetModelPosition(ingenUiSquare,buttonX,-buttonY,40);
	SetModelScale(ingenUiSquare,button.width,button.height,1);
	if button.hover then
		SetMeshColor(ingenUiSquare,0,236/255,236/255,236/255);
	else
		SetMeshColor(ingenUiSquare,0,218/255,222/255,222/255);
	end
	DrawComplexModel(ingenUiSquare,ingenUiCamera);
	if button.image then
		SetModelPosition(ingenUiTexSquare,buttonX+10,-buttonY-10,30);
		SetModelScale(ingenUiTexSquare,button.width-20,button.height-20,1);
		SetMeshTexture(ingenUiTexSquare,0,button.image);
		DrawComplexModel(ingenUiTexSquare,ingenUiCamera);
	end
	DrawText(ingenUiFont,button.text,buttonX,buttonY + (button.height/2) - 12);
end
function CreateUIButton()
	return {
		type = INGENUI_BUTTON;
		Update = function(button,parent,delta) UpdateUIButton(button,parent,delta) end;
		Draw   = function(button,parent) DrawUIButton(button,parent) end;
		x = 0;
		y = 0;
		displayX = 0;
		displayY = 0;
		width = 100;
		height = 100;
		text = "Button";
		action = function(button) end;
		hover = false;
		clicked = false;
		anchor = 0; -- TOP LEFT
	}
end

-- UI TEXTBOX

function ClickUITextBox(textbox)
	if textbox.hasFocus then
		textbox.submit(textbox);
	end
	textbox.hasFocus = not textbox.hasFocus;
end
function UpdateUITextBox(textbox,parent,delta)
	UpdateUIButton(textbox,parent,delta);
	
	if textbox.hasFocus then
		local typedText = GetTypedText();
		for i=1,string.len(typedText) do
			local typedChar = string.sub(typedText,i,1);
			if string.match(typedChar,"[%w%d%p ]") then
				textbox.text = textbox.text .. typedChar;
			else
				local typedByte = string.byte(typedChar);
				if typedByte == 8 then --backspace
					textbox.text = string.sub(textbox.text,1,string.len(textbox.text) - 1);
				end
				if typedByte == 13 then --enter
					textbox.hasFocus = false;
					textbox.submit(textbox);
				end
			end
		end
	end
end
function DrawUITextBox(textbox)
	local textboxX, textboxY = textbox.displayX, textbox.displayY;
	SetModelPosition(ingenUiSquare,textboxX,-textboxY,40);
	SetModelScale(ingenUiSquare,textbox.width,textbox.height,1);
	if textbox.hover or textbox.hasFocus then
		SetMeshColor(ingenUiSquare,0,236/255,236/255,236/255);
	else
		SetMeshColor(ingenUiSquare,0,218/255,222/255,222/255);
	end
	DrawComplexModel(ingenUiSquare,ingenUiCamera);
	local displayText = textbox.text;
	if textbox.hasFocus then displayText = displayText .. "|" end
	DrawText(ingenUiFont,displayText,textboxX,textboxY + (textbox.height/2) - 12);
end
function CreateUITextBox()
	return {
		type = INGENUI_TEXTBOX;
		Update = function(textbox,delta) UpdateUITextBox(textbox,delta) end;
		Draw   = function(textbox) DrawUITextBox(textbox) end;
		x = 0;
		y = 0;
		displayX = 0;
		displayY = 0;
		width = 100;
		height = 100;
		text = "";
		action = function(textbox) ClickUITextBox(textbox) end;
		submit = function(textbox) end;
		hover = false;
		clicked = false;
		hasFocus = false;
		anchor = 0; -- TOP LEFT
	}
end

-- UI SLIDER

function UpdateUISlider(slider,parent,delta)
	local mouseX, mouseY = GetMousePosition();
	local sliderX, sliderY = slider.displayX, slider.displayY;
	local sliderOffset = (slider.value-slider.minValue)/(slider.maxValue-slider.minValue);
	local barX = sliderX+10;
	local barY = sliderY+10;
	local barWidth = slider.width-20;
	local barHeight = slider.height-20;
	
	if mouseX > barX and mouseY > barY 
	and mouseX < barX + barWidth 
	and mouseY < barY + barHeight then
		slider.hover = true;
	else
		slider.hover = false;
	end
	
	if GetMouseLeft() and slider.hover and not slider.clicked then
		slider.clicked = true;
	end
	if not GetMouseLeft() then
		slider.clicked = false;
	end
	
	if slider.clicked then
		local tempValue = slider.value;
		if mouseX < sliderX + 15 then 
			tempValue = slider.minValue;
		elseif mouseX > sliderX + slider.width - 15 then
			tempValue = slider.maxValue;
		else
			tempValue = slider.minValue + ((mouseX-sliderX-15)/(slider.width-30) * (slider.maxValue-slider.minValue));
		end
		
		if tempValue ~= slider.value then
			slider.value = tempValue;
			slider.action(slider);
		end
	end
end
function DrawUISlider(slider)
	local buttonX, buttonY = slider.displayX, slider.displayY
	
	-- Background
	SetModelPosition(ingenUiSquare,buttonX,-buttonY,40);
	SetModelScale(ingenUiSquare,slider.width,slider.height,1);
	SetMeshColor(ingenUiSquare,0,218/255,222/255,222/255);
	DrawComplexModel(ingenUiSquare,ingenUiCamera);
	
	-- Slider Area
	SetModelPosition(ingenUiSquare,buttonX+10,-buttonY-10,30);
	SetModelScale(ingenUiSquare,slider.width-20,slider.height-20,1);
	SetMeshColor(ingenUiSquare,0,173/255,185/255,185/255);
	DrawComplexModel(ingenUiSquare,ingenUiCamera);
	
	local sliderOffset = (slider.value-slider.minValue)/(slider.maxValue-slider.minValue);
	
	-- Slider Fill
	SetModelPosition(ingenUiSquare,buttonX+10,-buttonY-10,20);
	SetModelScale(ingenUiSquare,(slider.width-20)*sliderOffset,slider.height-24,1);
	SetMeshColor(ingenUiSquare,0,12/255,210/255,210/255);
	DrawComplexModel(ingenUiSquare,ingenUiCamera);
	
	-- Slider Nub
	SetModelPosition(ingenUiSquare,buttonX+10+((slider.width-30)*sliderOffset),-buttonY-10,10);
	SetModelScale(ingenUiSquare,10,slider.height-24,1);
	if slider.hover then
		SetMeshColor(ingenUiSquare,0,1,1,1);
	else
		SetMeshColor(ingenUiSquare,0,236/255,236/255,236/255);
	end
	DrawComplexModel(ingenUiSquare,ingenUiCamera);
	
	DrawText(ingenUiFont,string.format("%1.2f",slider.value),buttonX+10,buttonY+(slider.height/2)-14);
end
function CreateUISlider()
	return {
		type = INGENUI_SLIDER;
		Update = function(slider,delta) UpdateUISlider(slider,delta) end;
		Draw   = function(slider) DrawUISlider(slider) end;
		x = 0;
		y = 0;
		displayX = 0;
		displayY = 0;
		width = 0;
		height = 0;
		value = 0.5;
		minValue = 0;
		maxValue = 1;
		action = function(slider) end;
		hover = false;
		clicked = false;
		anchor = 0; -- TOP LEFT
	}
end

-- INGEN UI

function CreateUISquare(posX, posY, width, height, tex)
	local idx = {0,1,2,1,3,2};
	if tex then
		vtx = {
			{posX,       posY,        0,    0,    0}, 
			{posX+width, posY,        0,    1,    0},
			{posX,       posY-height, 0,    0,    1},
			{posX+width, posY-height, 0,    1,    1}
		};
		vtxType = "PosTex";
	else
		vtx = {
			{posX,       posY,        0}, 
			{posX+width, posY,        0},
			{posX,       posY-height, 0},
			{posX+width, posY-height, 0}
		};
		vtxType = "Pos";
	end
	return CreateModel(vtxType,vtx,idx);
end

function CreateUI()
	ingenUiLoaded = false;
	ingenUiPanel = CreateUIPanel();
	ingenUiTicket = LoadAssets({"FrameworkDir","PathShader.xml","Shader","pathShader"});
	ingenUiSquare = CreateUISquare(0,0,1,1,false);
	ingenUiTexSquare = CreateUISquare(0,0,1,1,true);
	ingenUiCamera = CreateCamera(true);
	SetCameraPosition(ingenUiCamera,0,0,-1);
	
	-- For this to work properly, fonts are going to need to become assets
	ingenUiFont = GetFont(24,"Arial");
end

function AddUIComponent(component,panel)
	if not panel then 
		panel = ingenUiPanel;
	end
	
	table.insert(panel.components, component);
	
	-- return table length - 1 ??
end

function ClearUIComponents(panel)
	if not panel then 
		panel = ingenUiPanel;
	end
	
	panel.components = {};
end

function UpdateUI(delta)
	if ingenUiTicket > -1 and IsLoaded(ingenUiTicket) then
		ingenUiEffect = CreateEffect("pathShader");
		
		SetMeshEffect(ingenUiSquare,0,ingenUiEffect);
		SetMeshEffect(ingenUiTexSquare,0,ingenUiEffect);
		
		ingenUiTicket = -1;
		ingenUiLoaded = true;
	end
	
	backbufferWidth, backbufferHeight = GetBackbufferSize();
	SetCameraPosition(ingenUiCamera,backbufferWidth/2,-backbufferHeight/2,-1);
	SetCameraTarget(ingenUiCamera,backbufferWidth/2,-backbufferHeight/2,0);
	SetCameraClipHeight(ingenUiCamera,1,5000,backbufferHeight);
	
	ingenUiPanel.width = backbufferWidth;
	ingenUiPanel.height = backbufferHeight;
	
	if ingenUiLoaded then
		ingenUiPanel.Update(ingenUiPanel,nil,delta);
	end
end

function DrawUI()
	if ingenUiLoaded then
		ingenUiPanel.Draw(ingenUiPanel);
	end
end
