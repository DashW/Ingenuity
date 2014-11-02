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
function UpdateFlyCamera(delta)
	if GetMouseLeft() then
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
function StretchModelBetween(model,startX,startY,startZ,endX,endY,endZ)
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
	local xzRad = math.sqrt((dispX*dispX) + (dispZ*dispZ))
	local yRot = math.atan2(dispX/xzRad,dispZ/xzRad);
	local xRot = math.acos(xzRad/length);
	
	SetModelRotation(model,xRot,yRot,0);
	
	-- finally, the easy part, the length
	-- TODO: Make the overall scale an additional parameter
	
	SetModelScale(model,0.1,0.1,length);
end
