function Begin()
	pixelSpaceCam = CreateSpriteCamera(true,false,false);
	deviceSpaceCam = CreateSpriteCamera(false,true,true);
	SetCameraClipHeight(deviceSpaceCam,1,5000,1080);
	SetCameraPosition(deviceSpaceCam,0,0,-5);
	
	sprites = {};
	
	texTicket = LoadAssets(
		{"ProjectDir","PixTestGrad.png","TextureAsset","pixTest"},
		{"ProjectDir","IngenuityBulbBackground.png","TextureAsset","background"},
		{"ProjectDir","IngenuityBulbDiamond.png","TextureAsset","diamond"},
		{"ProjectDir","IngenuityBulbGlare.png","TextureAsset","glare"},
		{"ProjectDir","IngenuityBulbShine.png","TextureAsset","shine"},
		{"ProjectDir","IngenuityBulbShineShadow.png","TextureAsset","shadow"},
		{"ProjectDir","IngenuityBulbBulb.png","TextureAsset","bulb"},
		{"ProjectDir","IngenuityBulbBanner.png","TextureAsset","banner"},
		{"ProjectDir","IngenuityBulbIntro.png","TextureAsset","intro"},
		{"ProjectDir","IngenuityBulbSparkle.png","TextureAsset","sparkle"}
	)
	
	SetSamplerParam(GetBaseEffect(),"mipDisable",1);
	SetSamplerParam(GetBaseEffect(),"filter",2);
	
	sparkleCount = 300;
	
	sparkleInstances, sparkleFloats = CreateInstanceBuffer("PosRotSca",sparkleCount);
	
	-- Init Sparkles
	for i=0,sparkleCount-1 do
		local sparkle = i*9;
		sparkleFloats[sparkle+0] = ((math.random() * 1920) - 960);
		sparkleFloats[sparkle+1] = ((math.random() * 1080) - 540);
		sparkleFloats[sparkle+2] = 1 - (i/sparkleCount);
	end
	
	UpdateInstanceBuffer(sparkleInstances,sparkleFloats,sparkleCount);
	
	print("Size of sparkleFloats: " .. tostring(#sparkleFloats));
	
	glareRotation = 0;
	shineRotation = 0;
end

--function Reload()
--end

function Update(delta)
	if texTicket and IsLoaded(texTicket) then
		pixTestSprite = CreateSpriteModel(GetAsset("pixTest"),false);
		
		backgroundSprite = CreateSpriteModel(GetAsset("background"));
		SetModelPosition(backgroundSprite,-960,-540,100);
		
		diamondSprite = CreateSpriteModel(GetAsset("diamond"));
		SetModelPosition(diamondSprite,-960,-540,80);
		table.insert(sprites,diamondSprite);
		
		glareSprite = CreateSpriteModel(GetAsset("glare"));
		SetMeshPosition(glareSprite,0,-1494,-1445,60);
		table.insert(sprites,glareSprite);
		
		shadowSprite = CreateSpriteModel(GetAsset("shadow"));
		SetMeshPosition(shadowSprite,0,-772,-772,50);
		SetModelPosition(shadowSprite,-3,10,0);
		table.insert(sprites,shadowSprite);
		
		shineSprite = CreateSpriteModel(GetAsset("shine"));
		SetMeshPosition(shineSprite,0,-779,-779,40);
		SetModelPosition(shineSprite,-3,24,0);
		table.insert(sprites,shineSprite);
		
		bulbSprite = CreateSpriteModel(GetAsset("bulb"));
		SetModelPosition(bulbSprite,-135,-295,30);
		table.insert(sprites,bulbSprite);
		
		introSprite = CreateSpriteModel(GetAsset("intro"));
		SetModelPosition(introSprite,-493,170,20);
		table.insert(sprites,introSprite);
		
		bannerSprite = CreateSpriteModel(GetAsset("banner"));
		SetModelPosition(bannerSprite,-520,-420,20);
		table.insert(sprites,bannerSprite);
		
		sparkleSprite = CreateSpriteModel(GetAsset("sparkle"));
		SetModelPosition(sparkleSprite,0,0,90);
		
		texTicket = nil;
	end
	
	if glareSprite then
		glareRotation = glareRotation + delta;
		if glareRotation > (math.pi*2) then
			glareRotation = glareRotation - (math.pi*2);
		end
		SetModelRotation(glareSprite,0,0,glareRotation);
		
		shineRotation = shineRotation - delta;
		if shineRotation < 0 then
			shineRotation = shineRotation + (math.pi*2);
		end
		SetModelRotation(shineSprite,0,0,shineRotation);
		SetModelRotation(shadowSprite,0,0,shineRotation);
	end
	
	-- Update Sparkles
	for i=0,sparkleCount-1 do
		local sparkle = i*9;
		local x = sparkleFloats[sparkle+0];
		local y = sparkleFloats[sparkle+1];
		if x < -1100 or x > 1100 or y < -700 or y > 700 then
			x = (math.random() * 100) - 50;
			y = (math.random() * 100) - 50;
		end
		local dist = math.sqrt(x*x + y*y);
		local unitX = x/dist;
		local unitY = y/dist;
		local scale = (400-dist)/500;
		sparkleFloats[sparkle+0] = (x + (unitX * delta * (300000/dist) ));
		sparkleFloats[sparkle+1] = (y + (unitY * delta * (300000/dist) ));
		sparkleFloats[sparkle+6] = scale;
		sparkleFloats[sparkle+7] = scale;
	end
	UpdateInstanceBuffer(sparkleInstances,sparkleFloats,sparkleCount);
	
	width,height = GetBackbufferSize();
	SetCameraPosition(pixelSpaceCam,width/2,height/2,-1);
	SetCameraTarget(pixelSpaceCam,width/2,height/2,0);
	SetCameraClipHeight(pixelSpaceCam,1,5000,-height);
end

function Draw()
	
	if backgroundSprite then
		DrawComplexModel(backgroundSprite,deviceSpaceCam,0,0);
	end
	
	if sparkleSprite then
		DrawComplexModel(sparkleSprite,deviceSpaceCam,0,0,sparkleInstances);
	end
	
	for i,sprite in pairs(sprites) do
		DrawComplexModel(sprite,deviceSpaceCam,0,0);
	end
	
	if pixTestSprite then
		DrawComplexModel(pixTestSprite,pixelSpaceCam,0,0);
	end
	
end

function End()
end