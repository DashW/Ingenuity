
function HSLToRGB(h,s,l)
	local c = s * (1 - math.abs((2 * l) - 1));
	local h6 = h * 6;
	--local hRem = h6 - math.floor(h6);
	local hRem = math.abs((h6 % 2) - 1);
	local x = c * (1 - hRem);
	local r,g,b = 0,0,0;
	if h6 <= 1 then
		r=c; g=x; b=0;
	elseif h6 <= 2 then
		r=x; g=c; b=0;
	elseif h6 <= 3 then
		r=0; g=c; b=x;
	elseif h6 <= 4 then
		r=0; g=x; b=c;
	elseif h6 <= 5 then
		r=x; g=0; b=c;
	elseif h6 <= 6 then
		r=c; g=0; b=x;
	end
	local m = l - (c/2);
	return r+m,g+m,b+m;
end
