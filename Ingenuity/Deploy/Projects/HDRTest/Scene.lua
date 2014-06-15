
function BeginScene()
	texTicket = LoadAssets(
		{"ProjectDir","env2.bmp","Texture"},
		{"ProjectDir","env3.bmp","Texture"},
		{"ProjectDir","ground2.bmp","Texture"},
		{"ProjectDir","seafloor.bmp","Texture"}
	);
	
	wall1 = CreateModel("PosNorTex", CreateGrid(15, 3, 2, 2, 0, 0, 7, 2));
	SetModelRotation(wall1, math.pi/2, 0, 0);
	SetModelPosition(wall1, 0, 0, -10);
	SetMeshFactors(wall1, 0, 0.5, 1);
	SetMeshSpecular(wall1, 0, 5);
	
	wall2 = CreateModel("PosNorTex", CreateGrid(20, 3, 2, 2, 0, 0, 7, 2));
	SetModelRotation(wall2, -math.pi/2, math.pi/2, 0);
	SetModelPosition(wall2, 7.5, 0, 0);
	SetMeshFactors(wall2, 0, 0.5, 1);
	SetMeshSpecular(wall2, 0, 5);
	
	wall3 = CreateModel("PosNorTex", CreateGrid(15, 3, 2, 2, 0, 0, 7, 2));
	SetModelRotation(wall3, -math.pi/2, 0, 0);
	SetModelPosition(wall3, 0, 0, 10);
	SetMeshFactors(wall3, 0, 0.5, 1);
	SetMeshSpecular(wall3, 0, 5);
	
	wall4 = CreateModel("PosNorTex", CreateGrid(20, 3, 2, 2, 0, 0, 7, 2));
	SetModelRotation(wall4, math.pi/2, math.pi/2, 0);
	SetModelPosition(wall4, -7.5, 0, 0);
	SetMeshFactors(wall4, 0, 0.5, 1);
	SetMeshSpecular(wall4, 0, 5);
	
	floorMdl = CreateModel("PosNorTex", CreateGrid(15, 20, 2, 2, 0, 0, 8, 8));
	SetModelPosition(floorMdl, 0, -1.5, 0);
	SetMeshFactors(floorMdl, 0, 1, 3);
	SetMeshSpecular(floorMdl, 0, 50);
	
	ceilMdl = CreateModel("PosNorTex", CreateGrid(15, 20, 2, 2, 0, 0, 6, 6));
	SetModelRotation(ceilMdl, math.pi, 0, 0);
	SetModelPosition(ceilMdl, 0, 1.5, 0);
	SetMeshFactors(ceilMdl, 0, 0.3, 0.3);
	SetMeshSpecular(ceilMdl, 0, 5);
	
	local cubeVtx, cubeIdx = CreateCube();
	column1 = CreateModel("PosNorTex",cubeVtx,cubeIdx);
	SetModelScale(column1, 0.325, 1.5, 0.325);
	SetModelPosition(column1, 3.5, 0, 3);
	SetMeshFactors(column1, 0, 0.5, 1);
	SetMeshSpecular(column1, 0, 5);

	column2 = CreateModel("PosNorTex",cubeVtx,cubeIdx);
	SetModelScale(column2, 0.325, 1.5, 0.325);
	SetModelPosition(column2, -3.5, 0, 3);
	SetMeshFactors(column2, 0, 0.5, 1);
	SetMeshSpecular(column2, 0, 5);

	column3 = CreateModel("PosNorTex",cubeVtx,cubeIdx);
	SetModelScale(column3, 0.325, 1.5, 0.325);
	SetModelPosition(column3, 3.5, 0, -3);
	SetMeshFactors(column3, 0, 0.5, 1);
	SetMeshSpecular(column3, 0, 5);
	
	column4 = CreateModel("PosNorTex",cubeVtx,cubeIdx);
	SetModelScale(column4, 0.325, 1.5, 0.325);
	SetModelPosition(column4, -3.5, 0, -3);
	SetMeshFactors(column4, 0, 0.5, 1);
	SetMeshSpecular(column4, 0, 5);
	
	local pictVtx, pictIdx = CreateGrid(3.5,2.5,2,2,0,1,1,0);
	painting = CreateModel("PosNorTex",pictVtx,pictIdx);
	SetModelRotation(painting, -math.pi/2, 0, 0);
	SetModelPosition(painting, -3.5, 0, 9.8);
	SetMeshFactors(painting, 0, 1, 0.3);
	SetMeshSpecular(painting, 0, 5);

	painting2 = CreateModel("PosNorTex",pictVtx,pictIdx);
	SetModelRotation(painting2, -math.pi/2, 0, 0);
	SetModelPosition(painting2, 3.5, 0, 9.8);
	SetMeshFactors(painting2, 0, 1, 0.3);
	SetMeshSpecular(painting2, 0, 5);

	light1 = CreateLight("point");
	SetLightColor(light1, 80, 80, 80);
	SetLightPosition(light1, -3.5, 0.5, 8);
	-- lights[0]->atten = 0.0f;

	light2 = CreateLight("point");
	SetLightColor(light2, 8, 8, 8);
	SetLightPosition(light2, 3.5, 0.5, 8);
	-- lights[1]->atten = 0.0f;
	
	lights = { light1, light2 };

	local sphereVtx, sphereIdx = CreateSphere();
	sphere = CreateModel("PosNor",sphereVtx,sphereIdx);
	SetModelScale(sphere,0.2);
	SetModelPosition(sphere, -3.5, 0.5, 8);
	SetMeshColor(sphere, 0, 3200, 3200, 3200);

	sphere2 = CreateModel("PosNor",sphereVtx,sphereIdx);
	SetModelScale(sphere2,0.2);
	SetModelPosition(sphere2, 3.5, 0.5, 8);
	SetMeshColor(sphere2, 0, 320, 320, 320);
end

function UpdateScene(delta)
	if texTicket and IsLoaded(texTicket) then
		wallTex = GetAsset("ProjectDir", "env2.bmp");
		paintingTex = GetAsset("ProjectDir", "env3.bmp");
		floorTex = GetAsset("ProjectDir", "ground2.bmp");
		ceilTex = GetAsset("ProjectDir", "seafloor.bmp");

		if wallTex then
			SetMeshTexture(wall1, 0, wallTex);
			SetMeshTexture(wall2, 0, wallTex);
			SetMeshTexture(wall3, 0, wallTex);
			SetMeshTexture(wall4, 0, wallTex);
			SetMeshTexture(column1, 0, wallTex);
			SetMeshTexture(column2, 0, wallTex);
			SetMeshTexture(column3, 0, wallTex);
			SetMeshTexture(column4, 0, wallTex);
		end
		if floorTex then
			SetMeshTexture(floorMdl, 0, floorTex);
		end
		if ceilTex then
			SetMeshTexture(ceilMdl, 0, ceilTex);
		end
		if paintingTex then
			SetMeshTexture(painting, 0, paintingTex);
			SetMeshTexture(painting2, 0, paintingTex);
		end
		
		SetAnisotropy(16);
		
		texTicket = nil;
	end
end

function DrawScene(camera,surface)
	DrawComplexModel(wall1, camera, lights, surface);
	DrawComplexModel(wall2, camera, lights, surface);
	DrawComplexModel(wall3, camera, lights, surface);
	DrawComplexModel(wall4, camera, lights, surface);
	DrawComplexModel(floorMdl, camera, lights, surface);
	DrawComplexModel(ceilMdl, camera, lights, surface);
	DrawComplexModel(column1, camera, lights, surface);
	DrawComplexModel(column2, camera, lights, surface);
	DrawComplexModel(column3, camera, lights, surface);
	DrawComplexModel(column4, camera, lights, surface);
	DrawComplexModel(painting, camera, lights, surface);
	DrawComplexModel(painting2, camera, lights, surface);
	DrawComplexModel(sphere, camera, 0, surface);
	DrawComplexModel(sphere2, camera, 0, surface);
end