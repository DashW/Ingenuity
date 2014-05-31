
root2 = math.sqrt(2);

function CreatePlane(x,z,width,depth,twidth,theight)
	local idx = { 0, 1, 2, 1, 3, 2 };
	if twidth and theight then
		local vtx = { 
		{x,       0, z,         0, 1, 0,   1, 0, 0, 1,   0,      0 },
		{x,       0, z+depth,   0, 1, 0,   1, 0, 0, 1,   0,      theight },
		{x+width, 0, z,         0, 1, 0,   1, 0, 0, 1,   twidth, 0 },
		{x+width, 0, z+depth,   0, 1, 0,   1, 0, 0, 1,   twidth, theight }
		};
		return CreateModel("PosNorTanTex",vtx,4,idx,6);		
	else
		local vtx = { 
		{x,       0, z,         0, 1, 0 },
		{x,       0, z+depth,   0, 1, 0 },
		{x+width, 0, z,         0, 1, 0 },
		{x+width, 0, z+depth,   0, 1, 0 }
		};
		return CreateModel("PosNor",vtx,4,idx,6);	
	end
end

function Begin()
	grRect1 = CreatePlane(-root2/2,-0.5,root2/2,1);
	grRect2 = CreatePlane(0,-0.5,root2/2,1);
	grRect3 = CreatePlane(-root2/2,-0.5,root2/2,1);
	SetMeshColor(grRect1,0,1.0,0.0,0.0);
	SetMeshColor(grRect2,0,1.0,0.0,0.0);
	SetMeshColor(grRect3,0,1.0,0.0,0.0);
	
	light = CreateLight("point");
	SetLightPosition(light,0,1,0);
	
	camera = CreateCamera();
	SetCameraClipFov(camera,0.001,50,0.78539);
	SetCameraPosition(camera,0,2,0.001);
	
	font = GetFont(40,"Arial");
	
	foldRotation = 0;
end

function Reload()
	--Begin()
	
	SetMeshColor(grRect1, 0, 1.0, 0.0, 0.0);
	SetMeshColor(grRect2, 0, 1.0, 0.0, 0.0);
	SetMeshColor(grRect3, 0, 1.0, 0.0, 0.0);
end

function Update(delta)
	foldRotation = foldRotation + delta;
	if foldRotation > math.pi then foldRotation = foldRotation - math.pi end
	
	local tweenPeriod = foldRotation/math.pi;
	local zOffset = (tweenPeriod/2) * (root2/2) * -1;
	local camZoom = tweenPeriod * 0.581;
	
	local yRotation = ((1-math.cos(foldRotation))/2) * (math.pi);
	local zRotation = ((1-math.cos(foldRotation))/2) * (math.pi);
	
	SetModelRotation(grRect1,0,yRotation/2,0);
	SetModelRotation(grRect2,0,yRotation/2,zRotation);
	SetModelRotation(grRect3,0,yRotation/2,zRotation - math.pi);
	
	SetModelPosition(grRect1,0,camZoom,zOffset);
	SetModelPosition(grRect2,0,camZoom,zOffset);
	SetModelPosition(grRect3,0,camZoom,zOffset);
	
	SetLightPosition(light,0,1 + (camZoom/2),0);
end

function Draw()

	DrawComplexModel(grRect1,camera,light);
	DrawComplexModel(grRect2,camera,light);
	DrawComplexModel(grRect3,camera,light);
	
	DrawText(font,"Ingenuity Win8",0,0,0);
end

function End()

end
