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

function UpdateFrameTime(delta)
	if not sumDeltas or not numDeltas or not frameTimeText then
		sumDeltas = 0.0; numDeltas = 0; frameTimeText = "";
	end
	sumDeltas = sumDeltas + delta;
	numDeltas = numDeltas + 1;
	if sumDeltas > 0.5 then
		local avgDeltas = sumDeltas / numDeltas;
		frameTimeText = string.format("%2.2fms %3.2f%%",avgDeltas * 1000,(avgDeltas * 100000) / 16.5);
		sumDeltas = 0; numDeltas = 0;
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
