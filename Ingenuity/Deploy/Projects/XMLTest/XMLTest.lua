text = "hello";

Require("ProjectDir","../../Common/XmlHelper.lua");

function deepPrint(t, s)
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

function Begin()
	arial20 = GetFont(20,"Arial");
	arial40 = GetFont(40,"Arial");
	arial60 = GetFont(60,"Arial");
	courier20 = GetFont(20,"Courier New");
	courier40 = GetFont(40,"Courier New");
	courier60 = GetFont(60,"Courier New");
	
	LoadFile("FrameworkDir", "MultiTextureAnimY.xml", function(data) 
		text = data; 
		print(data);
	end);
	
	print("Script Started");
end

function Reload()
end

function Update(delta)
end

function Draw()
	DrawText(arial20,"Arial 20pt",100,100);
	DrawText(arial40,"Arial 40pt",100,200);
	DrawText(arial60,"Arial 60pt",100,325);
	DrawText(courier20,"Courier 20pt",400,100);
	DrawText(courier40,"Courier 40pt",400,200);
	DrawText(courier60,"Courier 60pt",400,325);
	
	DrawText(arial40,text,100,450);
end

function End()
	print("Script Finished");
end