
Require("ProjectDir","../../Common/IngenUtils.lua");
Require("ProjectDir","GpuParticleSystem.lua","GPUPS");

ROTATION_MATRIX = RotMatrix(0.0,math.pi,0.0);

function UpdateLeapHand()
	for i = 1,GetLeapNumBones(leapHelper) do
		visible, length, radius = GetLeapBoneDetails(leapHelper, i-1);
		leapVisibilities[i] = visible;
		if visible then
			if leapModels[i] == nil then
				--leapPhysicals[i] = CreatePhysicsCapsule(radius, length, false);
				--AddToPhysicsWorld(physicsWorld,leapPhysicals[i],true);
				
				local vtx, idx = CreateCapsule(length / (radius * 2.0));
				local boneModel = CreateModel("PosNor",vtx,idx);
				SetMeshScale(boneModel, 0, radius * 2.0);
				leapModels[i] = boneModel;
				print("Created Leap Model " .. i-1);
			end
			local leapBoneMatrix = GetLeapBoneMatrix(leapHelper, i-1);
			leapBoneMatrix = ROTATION_MATRIX * leapBoneMatrix;
			--SetPhysicsMatrix(leapPhysicals[i], leapBoneMatrix);
			SetModelMatrix(leapModels[i], leapBoneMatrix);
		end
	end
end

function DrawLeapHand()
	for i,leapModel in pairs(leapModels) do
		if leapModel and leapVisibilities[i] then
			DrawComplexModel(leapModel,camera,light);
		end
	end
end

function Begin()
	
	NUM_PARTICLES = 512 * 512;
	NUM_POINTS = 100;
	
	particles = GPUPS.Create(NUM_PARTICLES);
	particles.scaleX = 0.05;
	particles.scaleY = 0.05;
	
	leapHelper = CreateLeapHelper();
	SetLeapPosition(leapHelper,0,-0.5,0);
	SetLeapScale(leapHelper,0.025);
	leapVisibilities = {};
	leapModels = {};
	fingerPoints = {};
	fingerIndex = 1;
	fingerCount = 0;
	
	camera = CreateCamera();
	SetCameraPosition(camera, 0.0, 2.0, 20.0);
	SetupFlyCamera(camera,0.0,2.0,20.0,0.01,5);
	flyCamYAngle = math.pi;
	
	light = CreateLight("point");
	
	fingerSphere = CreateModel("PosNor",CreateSphere());
	SetModelScale(fingerSphere,0.1);
	SetMeshColor(fingerSphere,0,1,1,1);
	
	fingerCylinder = CreateModel("PosNor",CreateCylinder(1.0));
	
	pointFloats = CreateFloatArray(NUM_POINTS * 3);
	pointInstBuf = CreateInstanceBuffer("Pos", NUM_POINTS);
	
	lineFloats = CreateFloatArray(NUM_POINTS * 9);
	lineInstBuf = CreateInstanceBuffer("PosRotSca", NUM_POINTS);
	
	fingerInterval = 1 / 30;
	timeSinceFinger = 0;
	
	appTime = 0.0;
	
end

function Update(secs)
	
	timeSinceFinger = timeSinceFinger + secs;
	
	if timeSinceFinger > fingerInterval then
		local fvis, fx, fy, fz = GetLeapFinger(leapHelper,6); -- Right index finger
		if fvis then
			fingerPoints[fingerIndex] = { -fx, fy, fz };
			SetFloatArray(pointFloats,(fingerIndex-1)*3,-fx,fy,fz);
			fingerIndex = (fingerIndex % NUM_POINTS) + 1;
			if fingerCount < NUM_POINTS then
				fingerCount = fingerCount + 1;
			end
			
			
		end
		UpdateInstanceBuffer(pointInstBuf,pointFloats,fingerCount);
		timeSinceFinger = timeSinceFinger - fingerInterval;
	end
	
	GPUPS.Update(particles,secs);
	
	appTime = appTime + secs;
	--particles.consumerX = (math.sin(appTime * 3.0) * 50.0) + math.random();
	--particles.consumerY = (math.cos(appTime * 3.0) * 50.0) + math.random();
	
	UpdateLeapHand();
	
	UpdateFlyCamera(secs);
	
end

function Draw()
	
	DrawLeapHand();

	--for i,point in pairs(fingerPoints) do
		--SetModelPosition(fingerSphere, point[1], point[2], point[3]);
		--DrawComplexModel(fingerSphere, camera, 0, 0, pointInstBuf);
	--end
	
	DrawComplexModel(fingerCylinder, camera, 0, 0, lineInstBuf);
	
	GPUPS.Draw(particles, camera, GetWindowSurface(GetMainWindow()));
	
end

function End()
end