-- Ingenuity Lua Math Performance Test

-- BE WARNED, THIS SCRIPT COULD BE UNRESPONSIVE FOR UP TO A MINUTE!

-- Observed Results:
-- -- LuaJIT:
-- -- -- C++ GLM: 1.4ms
-- -- -- luaMatrix: 2.0ms
-- -- Lua:
-- -- -- C++ GLM: 4.4ms
-- -- -- luaMatrix: ~40ms!

Require("ProjectDir","matrix.lua","matrix");

function Begin()
	SetClearColor(1,1,1);
	debugFont = GetFont(40,"Arial");
	
	luaMat = matrix:new( 4, "I" );
	luaVec = matrix.transpose(matrix:new( { 100, 200, 300, 1 } ));
	
	print(tostring(luaVec));
	print("Lua:" .. luaVec[1][1] .. "," .. luaVec[1][2] .. "," .. luaVec[1][3]);
	
	ingVec = CreateVector(100,200,300,1);
	
	BeginTimestamp("lua", true, false);
	for iter = 1,1000000 do
		luaRot = matrix:rotation(0,0,0.01);
		luaVec = matrix.mul(luaVec,luaRot);
	end
	EndTimestamp("lua", true, false);
	
	BeginTimestamp("ing", true, false);
	for iter = 1,1000000 do
		ingRot = RotMatrix(0,0,0.01);
		ingVec = MulMatrix(ingRot,ingVec);
	end
	EndTimestamp("ing", true, false);
	
	cppTime = GetTimestampData("cpp", true);
	luaTime = GetTimestampData("lua", true);
	ingTime = GetTimestampData("ing", true);
	
	print("Lua: " .. luaTime .. "  Ingenuity: " .. ingTime );
end

function Update(delta)	
	luaRot = matrix:rotation(0,0,delta);
	luaVec = matrix.mul(luaVec,luaRot);
	
	ingRot = RotMatrix(0,0,delta);
	ingVec = ingRot * ingVec;
end

function Draw()
	DrawText(debugFont,"Running Tests...",0,0,0);
	DrawText(debugFont,"LUA",luaVec[1][1],luaVec[1][2],0);
	DrawText(debugFont,"Ingenuity",ingVec.x,ingVec.y,0);
end

function End()

end
