
local GPUPS = {}

function GPUPS.Create(numParticles, lifetime, period)
	
	local instance = {
		numParticles = numParticles,
		lifetime = lifetime,
		
		particleInsertInterval = period, 
		timeSinceParticleInsert = 0.0, 
		
		emitterX = 0.0, 
		emitterY = 0.0, 
		emitterZ = 0.0,
		
		scaleX = 0.5,
		scaleY = 0.5,
		
		runningTime = 0.0
	};
	
	instance.ticket = LoadAssets(
		{"FrameworkDir", "ParticleSystemInsert.xml", "Shader", "particleInsert"},
		{"FrameworkDir", "ParticleSystemUpdateCurl.xml", "Shader", "particleUpdate"},
		{"FrameworkDir", "ParticleSystemRender.xml", "Shader", "particleRender"},
		{"ProjectDir", "Particle.png", "Texture", "particleTex"}
	);
	
	-- How on earth is a developer supposed to know that the stride is (7x4=)28?!
	-- Seems like the sort of thing that needs to go into the shader metadata,
	-- the problem is that the script needs a reference to this buffer,
	-- so that it can be passed to other shaders!
	instance.particleBuf0 = CreateParamBuffer(NUM_PARTICLES, 28, true);
	instance.particleBuf1 = CreateParamBuffer(NUM_PARTICLES, 28, true);
	
	return instance;

end

function GPUPS.Insert(instance, x, y, z)
	
	SetEffectParam(instance.particleInsertEffect, 0, instance.particleBuf0);

	local scale = 2.0;
	local randomX = (math.random() * scale) - (scale / 2.0);
	local randomY = (math.random() * scale) - (scale / 2.0);
	local randomZ = (math.random() * scale) - (scale / 2.0);
	
	--local randomX = instance.consumerX - instance.emitterX;
	--local randomY = instance.consumerY - instance.emitterY;
	--local randomZ = instance.consumerZ - instance.emitterZ;

	-- normalize vector
	local length = math.sqrt(math.pow(randomX,2) + math.pow(randomY,2) + math.pow(randomZ,2));
	randomX = randomX / length;
	randomY = randomY / length;
	randomZ = randomZ / length;

	SetEffectParam(instance.particleInsertEffect, 1, x);
	SetEffectParam(instance.particleInsertEffect, 2, y);
	SetEffectParam(instance.particleInsertEffect, 3, z);
	
	SetEffectParam(instance.particleInsertEffect, 4, randomX);
	SetEffectParam(instance.particleInsertEffect, 5, randomY);
	SetEffectParam(instance.particleInsertEffect, 6, randomZ);
	
	Compute(instance.particleInsertEffect, 1);
	
end

function GPUPS.Update(instance,delta,doInsert)
	if doInsert == nil then doInsert = true end;
	
	instance.runningTime = instance.runningTime + delta;
	
	if instance.ticket and IsLoaded(instance.ticket) then
		instance.particleInsertEffect = CreateEffect("particleInsert");
		instance.particleUpdateEffect = CreateEffect("particleUpdate");
		instance.particleRenderEffect = CreateEffect("particleRender");
		
		instance.particleTex = GetAsset("particleTex");
		
		instance.ticket = nil;
	end
	
	if doInsert then
		instance.timeSinceParticleInsert = instance.timeSinceParticleInsert + delta;
	
		if instance.particleInsertEffect and instance.timeSinceParticleInsert > instance.particleInsertInterval then
	
			GPUPS.Insert(instance, instance.emitterX, instance.emitterY, instance.emitterZ);
			instance.timeSinceParticleInsert = 0.0;
			
		end
	end
	
	if instance.particleUpdateEffect then
		SetEffectParam(instance.particleUpdateEffect, 0, instance.particleBuf1);
		SetEffectParam(instance.particleUpdateEffect, 1, instance.particleBuf0);
		
		SetEffectParam(instance.particleUpdateEffect, 2, delta);
		SetEffectParam(instance.particleUpdateEffect, 3, instance.lifetime);
		SetEffectParam(instance.particleUpdateEffect, 4, instance.runningTime);
		SetEffectParam(instance.particleUpdateEffect, 6, 0.75);
		SetEffectParam(instance.particleUpdateEffect, 8, 0.0025);
		SetEffectParam(instance.particleUpdateEffect, 9, 0.0);
		SetEffectParam(instance.particleUpdateEffect,10, 0.0);
		SetEffectParam(instance.particleUpdateEffect,11,-1.0);
		
		Compute(instance.particleUpdateEffect, instance.numParticles / 512);
		
		local temp = instance.particleBuf0;
		instance.particleBuf0 = instance.particleBuf1;
		instance.particleBuf1 = temp;
	end
	
end

function GPUPS.Draw(instance,camera,surface)
	
	assert(camera and surface, "GPUPS.Draw needs both camera and surface!");
	
	if instance.particleRenderEffect then
		local viewMatrixFloats = CreateFloatArray(GetCameraViewMatrix(camera));
		local projMatrixFloats = CreateFloatArray(GetCameraProjMatrix(camera, surface));
		local consumerFloats = CreateFloatArray(CreateVector(instance.consumerX, instance.consumerX, instance.consumerX, 1.0));

		SetEffectParam(instance.particleRenderEffect, 0, instance.particleBuf0);
		SetEffectParam(instance.particleRenderEffect, 1, viewMatrixFloats);
		SetEffectParam(instance.particleRenderEffect, 2, projMatrixFloats);
		SetEffectParam(instance.particleRenderEffect, 3, consumerFloats);
		SetEffectParam(instance.particleRenderEffect, 4, instance.particleTex);
		SetEffectParam(instance.particleRenderEffect, 5, instance.scaleX);
		SetEffectParam(instance.particleRenderEffect, 6, instance.scaleY);
		SetEffectParam(instance.particleRenderEffect, 7, instance.lifetime);

		SetDepthMode("Read");
		SetBlendMode("Additive");

		DrawIndirect(instance.particleRenderEffect, instance.particleBuf0, 0, surface);

		SetDepthMode("ReadWrite");
		SetBlendMode("Alpha");
	end
	
end

return GPUPS;
