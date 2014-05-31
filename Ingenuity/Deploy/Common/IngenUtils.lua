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
