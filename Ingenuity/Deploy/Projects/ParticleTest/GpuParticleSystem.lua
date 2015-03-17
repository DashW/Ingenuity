
local GPUPS = {}

function GPUPS.Create(numParticles)
	local instance = {
		numParticles = numParticles,
		particleInsertInterval = (8.0 * 30.0) / numParticles,
		timeSinceParticleInsert = 0.0,
		
		emitterX = 0.0,
		emitterY = 0.0,
		emitterZ = 0.0,
		
		consumerX = 50.0,
		consumerY = 0.0,
		consumerZ = 0.0
	};
	
	instance.ticket = LoadAssets(
		{"FrameworkDir", "ParticleSystemInsert.xml", "Shader", "particleInsert"},
		{"FrameworkDir", "ParticleSystemUpdate.xml", "Shader", "particleUpdate"},
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

function GPUPS.Update(instance,delta)
	if instance.ticket and IsLoaded(instance.ticket) then
		instance.particleInsertEffect = CreateEffect("particleInsert");
		instance.particleUpdateEffect = CreateEffect("particleUpdate");
		instance.particleRenderEffect = CreateEffect("particleRender");
		
		instance.particleTex = GetAsset("particleTex");
		
		instance.ticket = nil;
	end
	
	instance.timeSinceParticleInsert = instance.timeSinceParticleInsert + delta;

	if instance.particleInsertEffect and instance.timeSinceParticleInsert > instance.particleInsertInterval then
		SetEffectParam(instance.particleInsertEffect, 0, instance.particleBuf0);

		local scale = 2.0;
		--local randomX = (math.random() * scale) - (scale / 2.0);
		--local randomY = (math.random() * scale) - (scale / 2.0);
		--local randomZ = (math.random() * scale) - (scale / 2.0);
		
		local randomX = instance.consumerX - instance.emitterX;
		local randomY = instance.consumerY - instance.emitterY;
		local randomZ = instance.consumerZ - instance.emitterZ;

		-- normalize vector
		local length = math.sqrt(math.pow(randomX,2) + math.pow(randomY,2) + math.pow(randomZ,2));
		randomX = randomX / length;
		randomY = randomY / length;
		randomZ = randomZ / length;

		SetEffectParam(instance.particleInsertEffect, 1, instance.emitterX);
		SetEffectParam(instance.particleInsertEffect, 2, instance.emitterY);
		SetEffectParam(instance.particleInsertEffect, 3, instance.emitterZ);
		
		SetEffectParam(instance.particleInsertEffect, 4, randomX);
		SetEffectParam(instance.particleInsertEffect, 5, randomY);
		SetEffectParam(instance.particleInsertEffect, 6, randomZ);
		
		Compute(instance.particleInsertEffect, 1);
		
		instance.timeSinceParticleInsert = 0.0;
	end
	
	if instance.particleUpdateEffect then
		SetEffectParam(instance.particleUpdateEffect, 0, instance.particleBuf1);
		SetEffectParam(instance.particleUpdateEffect, 1, instance.particleBuf0);
		
		SetEffectParam(instance.particleUpdateEffect, 2, delta);
		
		SetEffectParam(instance.particleUpdateEffect, 3, instance.consumerX);
		SetEffectParam(instance.particleUpdateEffect, 4, instance.consumerY);
		SetEffectParam(instance.particleUpdateEffect, 5, instance.consumerZ);
		
		Compute(instance.particleUpdateEffect, instance.numParticles / 512);
	end
end

function GPUPS.Draw(instance,camera,surface)
	assert(camera and surface, "GPUPS.Draw needs both camera and surface!");
	if instance.particleRenderEffect then
		local viewMatrixFloats = CreateFloatArray(GetCameraViewMatrix(camera));
		local projMatrixFloats = CreateFloatArray(GetCameraProjMatrix(camera, surface));
		local consumerFloats = CreateFloatArray(CreateVector(instance.consumerX, instance.consumerX, instance.consumerX, 1.0));

		SetEffectParam(instance.particleRenderEffect, 0, instance.particleBuf1);
		SetEffectParam(instance.particleRenderEffect, 1, viewMatrixFloats);
		SetEffectParam(instance.particleRenderEffect, 2, projMatrixFloats);
		SetEffectParam(instance.particleRenderEffect, 3, consumerFloats);
		SetEffectParam(instance.particleRenderEffect, 4, instance.particleTex);

		SetDepthMode("Read");
		SetBlendMode("Additive");

		DrawIndirect(instance.particleRenderEffect, instance.particleBuf1, 0, surface);

		SetDepthMode("ReadWrite");
		SetBlendMode("Alpha");

		local temp = instance.particleBuf0;
		instance.particleBuf0 = instance.particleBuf1;
		instance.particleBuf1 = temp;
	end
end

return GPUPS;
