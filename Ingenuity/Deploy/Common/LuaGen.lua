
local LuaGen = {};

function LuaGen.Call(name, ...)
	return name .. "(" .. table.concat({...},",") .. ")";
end

function LuaGen.Assign(expr, ...)
	return table.concat({...},",") .. " = " .. expr;
end

return LuaGen;
