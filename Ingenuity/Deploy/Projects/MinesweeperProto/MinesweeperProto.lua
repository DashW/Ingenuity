-- Ingenuity Minesweeper
--
-- Richard Copperwaite 2013

Require("ProjectDir","../../Common/IngenUtils.lua");

-- Ultimate Random Seed(TM) http://lua-users.org/wiki/MathLibraryTutorial
-- math.randomseed( tonumber(tostring(os.time()):reverse():sub(1,6)) )

function GridNeighbour(index,neighbourIndex)
	if neighbourIndex == 1 and index > columns and index % columns ~= 1 then return index - columns - 1; end
	if neighbourIndex == 2 and index > columns then return index - columns; end
	if neighbourIndex == 3 and index > columns and index % columns ~= 0 then return index - columns + 1; end
	if neighbourIndex == 4 and index % columns ~= 1 then return index - 1; end
	if neighbourIndex == 5 and index % columns ~= 0 then return index + 1; end
	if neighbourIndex == 6 and index <= columns*(rows-1) and index % columns ~= 1 then return index + columns - 1; end
	if neighbourIndex == 7 and index <= columns*(rows-1) then return index + columns; end
	if neighbourIndex == 8 and index <= columns*(rows-1) and index % columns ~= 0 then return index + columns + 1; end
	return 0;
end

function GenerateGrid()
	grid = {};
	for i = 1,columns*rows do
		grid[i] = { 
			hasMine = false,
			hasFlag = false,
			revealed = false,
			grouped = false,
			nearbyMines = 0 
		};
	end
	
	if numMines >= columns*rows then return; end -- error
	
	for i = 1, numMines do
		gridIndex = 0;
		while gridIndex == 0 or grid[gridIndex].hasMine do
			gridIndex = math.ceil(math.random() * columns * rows);
		end
		grid[gridIndex].hasMine = true;
		for i = 1,8 do
			local neighbour = GridNeighbour(gridIndex,i);
			if neighbour > 0 then
				grid[neighbour].nearbyMines = grid[neighbour].nearbyMines + 1;
			end
		end
	end
end

function RevealEmpty(gridIndex)
	for i = 1,8 do
		local neighbour = GridNeighbour(gridIndex,i);
		if neighbour ~= 0 and grid[neighbour].revealed == false then
			grid[neighbour].revealed = true;
			if grid[neighbour].nearbyMines == 0 then
				RevealEmpty(neighbour);
			end
		end
	end
end

function AddToStartGroup(group, gridIndex)
	if gridIndex ~= 0 
	and grid[gridIndex].nearbyMines == 0 
	and grid[gridIndex].hasMine == false 
	and grid[gridIndex].grouped == false 
	then
		table.insert(startGroups[group],gridIndex);
		startGroups[group].length = startGroups[group].length + 1;
		grid[gridIndex].grouped = true;
		
		for i = 1,8 do
			AddToStartGroup(group, GridNeighbour(gridIndex,i));
		end
		
		return true
	end
	return false
end

function PickStartPoint()
	startGroups = {};
	startGroupNum = 1;
	startGroups[startGroupNum] = { length = 0 }
	longestLength = 0;
	longestGroup = 0;
	
	for i = 1, columns * rows do
		if AddToStartGroup(startGroupNum, i) then
			local length = startGroups[startGroupNum].length;
			if length > longestLength then
				longestLength = length;
				longestGroup = startGroupNum;
			end		
			startGroupNum = startGroupNum + 1;
			startGroups[startGroupNum] = { length = 0 }
		end
	end
	
	local randomStartPoint = math.random(longestLength);
	startPoint = startGroups[longestGroup][randomStartPoint];
end

function Begin()
	SetClearColor(1,1,1);
	
	camera = CreateSpriteCamera(false,true,false);
	
	ticket = LoadAssets(
		{"ProjectDir","Square.png","Texture","square"},
		{"ProjectDir","Flag.png",  "Texture","flag"  }
	);
	
	debugFont = GetFont(35,"Courier New","regular",false);
	SetFontColor(debugFont,0,0,1);
	
	idealRows = 8;
	
	columns = 30;
	rows = 16;
	numMines = 99;
	GenerateGrid();
	PickStartPoint();
	
	clicked = false;
	rightClicked = false;
	showCursor = true;
end

function UpdateParams()
	scale = idealRows / rows;
	unit = scale * (78 / 768);
	xOffset = -(columns+2) * unit * 0.5;
	yOffset = -(rows+2) * unit * 0.5;
end

function Reload()
	--GenerateGrid();
	--PickStartPoint();
end

function PixelToDIC(x,y)
	local DICx = (x / screenHeight) - (0.5 * (screenWidth/screenHeight));
	local DICy = (y / screenHeight) - (0.5);
	return DICx, DICy;
end

function DICToPixel(x,y)
	local pixelx = (x + ((screenWidth/screenHeight)/2)) * (screenHeight);
	local pixely = (y + 0.5) * (screenHeight);
	return pixelx, pixely;
end

function GetCellAtPoint(x,y)
	if x > xOffset+unit and x < xOffset+((columns+1)*unit)
			and y > yOffset+unit and y < yOffset+((rows+1)*unit) then
		local gridX = (x - (xOffset+unit)) / (columns*unit);
		local gridY = (y - (yOffset+unit)) / (rows*unit);
		-- print(gridX,gridY);
		return math.ceil(gridX * columns) + (math.floor(gridY * rows) * columns);
	else
		return 0
	end
end

function Update(delta)
	if IsLoaded(ticket) then
		print("Ticket Loaded!");
		square = CreateSpriteModel(GetAsset("square"));
		flag = CreateSpriteModel(GetAsset("flag"));
		SetMeshScale(square,0,78/768);
		SetMeshScale(flag,0,78/768);
		ticket = -1;
	end

	UpdateFrameTime(delta);
	UpdateParams();
	
	screenWidth, screenHeight = GetBackbufferSize();
	mouseX, mouseY = PixelToDIC(GetMousePosition()); -- Could we not use a flag to return mouse position in DIC ?
	
	if GetMouseLeft() then
		if not clicked then
			local clickedCell = GetCellAtPoint(mouseX,mouseY);
			print(clickedCell);
			if clickedCell ~= 0 then
				grid[clickedCell].revealed = true;
				if grid[clickedCell].nearbyMines == 0 and grid[clickedCell].hasMine == false then
					RevealEmpty(clickedCell);
				end
			end
			clicked = true;
		end
	else
		clicked = false;
	end
	
	if GetMouseRight() then
		if not rightClicked then
			local clickedCell = GetCellAtPoint(mouseX,mouseY);
			if clickedCell ~= 0 then
				grid[clickedCell].hasFlag = not grid[clickedCell].hasFlag;
			end
			rightClicked = true;
		end
	else
		rightClicked = false;
	end
	
	local down, pressed, released = GetKeyState('r');
	if pressed then
		GenerateGrid();
		PickStartPoint();
	end
end

function Draw()
	if square then
		SetModelScale(square, scale, scale, 0);
		SetModelScale(flag, scale, scale, 1);
		
		for cellX = 1,columns do
			for cellY = 1,rows do
				local cellPosX = xOffset + (cellX*unit);
				local cellPosY = yOffset + (cellY*unit);
				local gridIndex = ((cellY-1)*columns) + cellX;
				
				if grid[gridIndex].revealed then
					if grid[gridIndex].hasMine then
						DrawText(debugFont,"M",cellPosX + (unit/2),cellPosY + (unit/2), true);
					else
						if grid[gridIndex].nearbyMines > 0 then
							DrawText(debugFont,
								tostring(grid[gridIndex].nearbyMines),
								cellPosX + (unit/2),cellPosY + (unit/2), true);
						end
					end
				else
					SetModelPosition(square, cellPosX, cellPosY, 0);
					DrawComplexModel(square, camera);
					
					if grid[gridIndex].hasFlag or gridIndex == startPoint then
						SetModelPosition(flag, cellPosX, cellPosY, 0);
						DrawComplexModel(flag, camera);
					end
				end
			end
		end
		
		if showCursor then
			mouseDICx, mouseDICy = PixelToDIC(mouseX, mouseY);
			SetModelPosition(square, mouseX, mouseY, 0);
			DrawComplexModel(square, camera); -- CURSOR
		end
	end
	
	DrawText(debugFont,"Minesweeper Proto",PixelToDIC(0,0)); -- SCALING!
end

function End()
end