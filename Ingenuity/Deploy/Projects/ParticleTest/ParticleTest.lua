
Require("ProjectDir","../../Common/IngenUtils.lua");
Require("ProjectDir","GpuParticleSystemCurl.lua","GPUPS");

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
	
	particles = GPUPS.Create(NUM_PARTICLES, 8, (8 * 8) / NUM_PARTICLES);
	particles.scaleX = 0.05;
	particles.scaleY = 0.05;
	
	soundTicket = LoadAssets(
		{"ProjectDir", "one_sparkler_loop.wav", "WavAudio", "sparkler"}
	);
	
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
	
	fingerInterval = 0.05;
	timeSinceFinger = 0.0;
	
	appTime = 0.0;
	
end

function Update(secs)
	if soundTicket and IsLoaded(soundTicket) then
		sparklerSound = GetAsset("sparkler");
		PlaySound(sparklerSound,0,true);
		SetSoundSpeed(sparklerSound,0);
		print("Playing sparkler sound...");
		soundTicket = nil;
	end
    
	local fvis, fx, fy, fz = GetLeapFinger(leapHelper,6); -- Right index finger
    if fvis and fz < 0.0 then
		timeSinceFinger = timeSinceFinger + secs;
		if timeSinceFinger > fingerInterval then
			fingerPoints[fingerIndex] = { -fx, fy, fz };
			SetFloatArray(pointFloats, (fingerIndex-1)*3, -fx, fy, fz);
			fingerIndex = (fingerIndex % NUM_POINTS) + 1;
			if fingerCount < NUM_POINTS then
				fingerCount = fingerCount + 1;
			end
            UpdateInstanceBuffer(pointInstBuf,pointFloats,fingerCount);
            timeSinceFinger = timeSinceFinger - fingerInterval;
        end
	end
    
	if fingerCount > 1 then
		local last = fingerPoints[((fingerIndex-2) % NUM_POINTS) + 1];
		local prev = fingerPoints[((fingerIndex-3) % NUM_POINTS) + 1];
		local dx = last[1] - prev[1];
		local dy = last[2] - prev[2];
		local dz = last[3] - prev[3];
		local distance = math.sqrt((dx*dx) + (dy*dy) + (dz*dz));
		SetSoundSpeed(sparklerSound, 1.0 + distance/10);
		SetSoundVolume(sparklerSound, 0.5 + distance/10);
		
		particles.timeSinceParticleInsert = particles.timeSinceParticleInsert + secs;
		while particles.particleInsertEffect and particles.timeSinceParticleInsert > particles.particleInsertInterval do
			
			local randOffset = math.random();
			local segmentNumber = math.floor(randOffset * (fingerCount - 1)) + 1;
			local segmentIndex = (((segmentNumber + fingerIndex - fingerCount) - 2) % NUM_POINTS) + 1;
			--print("Segment Number: " .. segmentNumber .. " Segment Index: " .. segmentIndex);
			local segmentOffset = math.random();
			local startPoint = fingerPoints[segmentIndex];
			local endPoint = fingerPoints[(segmentIndex % NUM_POINTS) + 1];
			local insertX = startPoint[1] + ((endPoint[1] - startPoint[1]) * segmentOffset);
			local insertY = startPoint[2] + ((endPoint[2] - startPoint[2]) * segmentOffset);
			local insertZ = startPoint[3] + ((endPoint[3] - startPoint[3]) * segmentOffset);
			
			--local radius = 0.2;
			--local insertX = math.random() - 0.5;
			--local insertY = math.random() - 0.5;
			--local insertZ = math.random() - 0.5;
			--local factor = math.sqrt((insertX*insertX) + (insertY*insertY) + (insertZ*insertZ)) / radius;
			--insertX = insertX / factor;
			--insertY = insertY / factor;
			--insertZ = insertZ / factor;
			
			GPUPS.Insert(particles, insertX, insertY, insertZ);
			particles.timeSinceParticleInsert = particles.timeSinceParticleInsert - particles.particleInsertInterval;
			
		end
	end
	
	GPUPS.Update(particles,secs,false);
	
	appTime = appTime + secs;
	--particles.consumerX = (math.sin(appTime * 3.0) * 50.0) + math.random();
	--particles.consumerY = (math.cos(appTime * 3.0) * 50.0) + math.random();
	
	UpdateLeapHand();
	
	UpdateFlyCamera(secs);
	
	local d,p,r = GetKeyState(' ');
	if p then
		fingerPoints = {};
		fingerCount = 0;
		fingerIndex = 1;
	end
	
end

function Draw()
	
	DrawLeapHand();

	--DrawComplexModel(fingerSphere, camera, 0, 0, pointInstBuf);
	
	--DrawComplexModel(fingerCylinder, camera, 0, 0, lineInstBuf);
	
	GPUPS.Draw(particles, camera, GetWindowSurface(GetMainWindow()));
	
end

function End()
end