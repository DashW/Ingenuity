-- Ingenuity Model Rendering Demo
--
-- Simply loads a model and rotates it about 360 degrees
--
-- Richard Copperwaite 2013

text = "hello";

function Begin()
	arial20 = GetFont(20,"Arial");
	arial40 = GetFont(40,"Arial");
	arial60 = GetFont(60,"Arial");
	courier20 = GetFont(20,"Courier New");
	courier40 = GetFont(40,"Courier New");
	courier60 = GetFont(60,"Courier New");
	
	LoadFile("FrameworkDir", "BaseShader.xml", function(data) 
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