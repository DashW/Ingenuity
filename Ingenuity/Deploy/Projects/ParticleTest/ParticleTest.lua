
Require("ProjectDir","../../Common/IngenUtils.lua");
Require("ProjectDir","GpuParticleSystem.lua","GPUPS");

function Begin()
	
	NUM_PARTICLES = 512 * 512;
	particles = GPUPS.Create(NUM_PARTICLES);
	
	camera = CreateCamera();
	SetCameraPosition(camera, 0.0, 2.0, 20.0);
	SetupFlyCamera(camera,0.0,2.0,20.0,0.01,5);
	flyCamYAngle = math.pi;
	
	appTime = 0.0;
	
end

function Update(secs)
	
	GPUPS.Update(particles,secs);
	
	appTime = appTime + secs;
	particles.consumerX = math.sin(appTime * 3.0) * 50.0;
	particles.consumerY = math.cos(appTime * 3.0) * 50.0;
	
	print(particles.consumerX);
	
	UpdateFlyCamera(secs);
	
end

function Draw()
	
	GPUPS.Draw(particles, camera, GetWindowSurface(GetMainWindow()));
	
end

function End()
end