-- Ingenuity Utility Functions

function DeepPrint(t, s)
	if not s then s = 1 end
	for key,value in pairs(t) do
		local tabs = "";
		for i = 1,s do tabs = tabs.."  " end
		if type(value)=="table" then
			print(tabs..tostring(key)..":")
			deepPrint(value, s+1);
		else
			print(tabs..tostring(key)..": "..tostring(value))
		end
	end
end

function DeepCopy(t)
	if type(t) ~= 'table' then return t; end
	local mt = getmetatable(t);
	local res = {};
	for k,v in pairs(t) do
		if type(v) == 'table' then
			v = DeepCopy(v);
		end
		res[k] = v;
	end
	setmetatable(res,mt);
	return res;
end

function PrintGlobals()
	for k,v in pairs(_G) do
		print("Global key", k, "value", tostring(v))
	end
end

function CreateAccumulator()
	return {
		stack = {},
		count = 0,
		sum = 0,
		Add = function(self,value)
			table.insert(self.stack, value);
			self.count = self.count + 1;
			self.sum = self.sum + value;
		end,
		Average = function(self)
			return self.sum / self.count;
		end,
		Sum = function(self)
			return self.sum;
		end,
		Clear = function(self,sub)
			self.stack = {};
			self.count = 0;
			if sub then
				self.sum = self.sum - sub;
			else
				self.sum = 0;
			end
		end
	}
end

function UpdateFrameTime(delta)
	if not sumDeltas or not numDeltas or not frameTimeText then
		sumDeltas = 0.0; numDeltas = 0; frameTimeText = "";
	end
	sumDeltas = sumDeltas + delta;
	numDeltas = numDeltas + 1;
	if sumDeltas > 0.5 then
		local avgDeltas = sumDeltas / numDeltas;
		frameTimeText = string.format("%2.2fms %3.2f%%",avgDeltas * 1000,(avgDeltas * 100000) / 16.5);
		sumDeltas = sumDeltas - 0.5; numDeltas = 0;
	end
end

function SetupFlyCamera(camera,x,y,z,sens,speed)
	flyCamera = camera;
	flyCamYAngle = 0;
	flyCamUpAngle = 0;
	flyCamSensitivity = sens;
	flyCamSpeed = speed;
	
	flyCamX = x;
	flyCamY = y;
	flyCamZ = z;
end	
function UpdateFlyCamera(delta,lookDisabled)
	if GetMouseLeft() and not lookDisabled then
		local dx, dy = GetMouseDelta();
		
		flyCamYAngle = flyCamYAngle + (dx * flyCamSensitivity);
		
		if(flyCamYAngle > math.pi*2) then flyCamYAngle = flyCamYAngle - math.pi*2; end
		if(flyCamYAngle < 0) then flyCamYAngle = flyCamYAngle + math.pi*2; end

		flyCamUpAngle = flyCamUpAngle - (dy * flyCamSensitivity);
		
		if(flyCamUpAngle >=  math.pi/2) then flyCamUpAngle =  math.pi/2 - 0.01; end
		if(flyCamUpAngle <= -math.pi/2) then flyCamUpAngle = -math.pi/2 + 0.01; end
	end

	local dirX = (math.sin(flyCamYAngle) * math.cos(flyCamUpAngle));
	local dirY = (math.sin(flyCamUpAngle));
	local dirZ = (math.cos(flyCamYAngle) * math.cos(flyCamUpAngle));
	
	sideMult = 1 / math.sqrt((dirX*dirX) + (dirZ*dirZ));
	
	if GetKeyState('w') then
		flyCamX = flyCamX + (dirX * delta * flyCamSpeed);
		flyCamY = flyCamY + (dirY * delta * flyCamSpeed);
		flyCamZ = flyCamZ + (dirZ * delta * flyCamSpeed);
	end
	if GetKeyState('s') then
		flyCamX = flyCamX - (dirX * delta * flyCamSpeed);
		flyCamY = flyCamY - (dirY * delta * flyCamSpeed);
		flyCamZ = flyCamZ - (dirZ * delta * flyCamSpeed);
	end
	if GetKeyState('a') then
		flyCamX = flyCamX - (dirZ * delta * flyCamSpeed * sideMult);
		flyCamZ = flyCamZ + (dirX * delta * flyCamSpeed * sideMult);
	end
	if GetKeyState('d') then
		flyCamX = flyCamX + (dirZ * delta * flyCamSpeed * sideMult);
		flyCamZ = flyCamZ - (dirX * delta * flyCamSpeed * sideMult);
	end

	SetCameraPosition(flyCamera, flyCamX, flyCamY, flyCamZ);
	SetCameraTarget(flyCamera, flyCamX+dirX, flyCamY+dirY, flyCamZ+dirZ);
end

-- Stretches a model between two points along its Z-axis
-- Useful for drawing debug geometry
function StretchModelBetween(model,scale,startX,startY,startZ,endX,endY,endZ)
	local dispX = startX - endX;
	local dispY = startY - endY;
	local dispZ = startZ - endZ;
	
	-- position is halfway along the displacement vector
	
	local midX = startX - (dispX/2);
	local midY = startY - (dispY/2);
	local midZ = startZ - (dispZ/2);
	
	SetModelPosition(model,midX,midY,midZ);
	
	-- now, we need to figure out the rotation
	-- in the case of a cylinder, if displacement is (0,0,1), no rotation is needed
	-- GLM applies rotations Y, then X, then Z
	
	local length = math.sqrt((dispX*dispX) + (dispY*dispY) + (dispZ*dispZ));
	local xzRad = math.sqrt((dispX*dispX) + (dispZ*dispZ));
	local yRot = 0;
	if xzRad ~= 0 then
		yRot = math.atan2(dispX/xzRad,dispZ/xzRad);
	end
	local xRot = 0;
	if dispY > 0 then
		xRot = -math.acos(xzRad/length);
	else
		xRot = math.acos(xzRad/length);
	end
	SetModelRotation(model,xRot,yRot,0);
	
	-- finally, the easy part, the length
	
	SetModelScale(model,scale,scale,length);
end

function CreateSkyCube()
	local vtx = {
		{-1,-1,-1}, {-1, 1,-1}, { 1,-1,-1}, { 1, 1,-1},
		{ 1,-1, 1}, { 1, 1, 1}, {-1,-1, 1}, {-1, 1, 1}
	};
	local idx = {
		 0, 1, 2,   1, 3, 2,   6, 0, 4,   0, 2, 4,
		 1, 7, 3,   7, 5, 3,   4, 5, 6,   5, 7, 6,
		 6, 7, 0,   7, 1, 0,   2, 3, 4,   3, 5, 4
	};
	local skyCube = CreateModel("Pos",vtx,idx);
	SetModelScale(skyCube,50);
	return skyCube;
end

function UpdatePixelCamera(camera,window,centerOrigin,yUpwards,near,far)
	local windowWidth,windowHeight = GetBackbufferSize(window);
	local pixOffsetX = windowWidth % 2;
	local pixOffsetY = windowHeight % 2;
	local cameraHeight = windowHeight;
	
	if centerOrigin then
		SetCameraPosition(camera,pixOffsetX,pixOffsetY,-1);
		SetCameraTarget(camera,pixOffsetX,pixOffsetY,0);
	else
		SetCameraPosition(camera,windowWidth/2,windowHeight/2,-1);
		SetCameraTarget(camera,windowWidth/2,windowHeight/2,0);
	end
	
	if not yUpwards then
		cameraHeight = cameraHeight * -1;
	end
	
	SetCameraClipHeight(camera,near,far,cameraHeight);
end

function FutureAsset(folder,file,type)
	future = {
		name = folder .. ":" .. file,
		ticket = LoadAssets(folder,file,type,folder .. ":" .. file),
		value = nil,
		pendingFuncs = {},
		Update = function(self)
			if self.ticket and IsLoaded(self.ticket) then
				self.value = GetAsset(self.name);
				for i,tuple in pairs(self.pendingFuncs) do
					tuple.func(self.value,unpack(tuple.arg));
				end
				self.pendingFuncs = {};
				self.ticket = nil;
			end
		end,
		Finished = function(self)
			return self.ticket == nil;
		end,
		Ready = function(self)
			self:Update();
			return self.value ~= nil;
		end,
		WhenReady = function(self,func,...)
			if self.value then
				func(self.value,...);
			else
				table.insert(self.pendingFuncs,{func=func,arg={...}});
			end
		end,
		Progress = function(self)
			return GetLoadProgress(self.ticket);
		end
	};
	setmetatable( future, {
		__call = function(self)
			self:Update();
			return self.value;
		end
	});
	return future;
end
