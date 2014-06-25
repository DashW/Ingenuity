-- Ingenuity HDR lighting module

SD_NAME = 1;
SD_LINES = 2;
SD_PASSES = 3;
SD_LENGTH = 4;
SD_ATTEN = 5;
SD_ROTATE = 6;
SD_ROTATED = 7;

starDefs = {
	-- 1                 2       3       4       5      6        7
    -- star name         lines   passes  length  attn   rotate   bRotate
    {  "Disable",        0,      0,      0.0,    0.0,   0.0,     false  },  -- STLT_DISABLE
    {  "Cross",          4,      3,      1.0,    0.85,  0.0,     true   },  -- STLT_CROSS
    {  "CrossFilter",    4,      3,      1.0,    0.95,  0.0,     true   },  -- STLT_CROSSFILTER
    {  "snowCross",      6,      3,      1.0,    0.96,  0.3491,  true   },  -- STLT_SNOWCROSS
    {  "Vertical",       2,      3,      1.0,    0.96,  0.0,     false  }   -- STLT_VERTICAL
};

GD_NAME = 1;
GD_GLARE = 2;
GD_BLOOM = 3;
GD_GHOST = 4;
GD_DISTORT = 5;
GD_STAR = 6;
GD_STARTYPE = 7;
GD_ROTATE = 8;
GD_CA = 9
GD_CURRENT = 10;
GD_AFTER = 11;
GD_AILUM = 12;

glareDefs = {
	--  1                                 2      3      4      5      6      7
    --  glare name                        glare  bloom  ghost  distort star  star type
	--  8               9      10     11     12
    --  rotate          C.A    current after ai lum
    {   "Disable",                        0.0,   0.0,   0.0,   0.01,  0.0,   1,
        0.0,            0.5,   0.00,  0.00,  0.0  },   -- GLT_DISABLE
    {   "Camera",                         1.5,   1.2,   1.0,   0.00,  1.0,   2,
        00.0,           0.5,   0.25,  0.90,  1.0  },   -- GLT_CAMERA
    {   "Natural Bloom",                  1.5,   1.2,   0.0,   0.00,  0.0,   1,
        00.0,           0.0,   0.40,  0.85,  0.5  },   -- GLT_NATURAL
    {   "Cheap Lens Camera",              1.25,  2.0,   1.5,   0.05,  2.0,   2,
        00.0,           0.5,   0.18,  0.95,  1.0  },   -- GLT_CHEAPLENS
    {   "Cross Screen Filter",            1.0,   2.0,   1.7,   0.00,  1.5,   3,
        0.4363323125,   0.5,   0.20,  0.93,  1.0  },   -- GLT_FILTER_CROSSSCREEN
    {   "Spectral Cross Filter",          1.0,   2.0,   1.7,   0.00,  1.8,   3,
        1.221730475,    1.5,   0.20,  0.93,  1.0  },   -- GLT_FILTER_CROSSSCREEN_SPECTRAL
    {   "Snow Cross Filter",              1.0,   2.0,   1.7,   0.00,  1.5,   4,
        0.174532925,    0.5,   0.20,  0.93,  1.0  },   -- GLT_FILTER_SNOWCROSS
    {   "Spectral Snow Cross",            1.0,   2.0,   1.7,   0.00,  1.8,   4,
        0.6981317,      1.5,   0.20,  0.93,  1.0  },   -- GLT_FILTER_SNOWCROSS_SPECTRAL
    {   "Sunny Cross Filter",             1.0,   2.0,   1.7,   0.00,  1.5,   6,
        00.0,           0.5,   0.20,  0.93,  1.0  },   -- GLT_FILTER_SUNNYCROSS
    {   "Spectral Sunny Cross",           1.0,   2.0,   1.7,   0.00,  1.8,   6,
        0.7853981625,   1.5,   0.20,  0.93,  1.0  },   -- GLT_FILTER_SUNNYCROSS_SPECTRAL
    {   "Cine Camera Vertical Slits",     1.0,   2.0,   1.5,   0.00,  1.0,   5,
        1.570796325,    0.5,   0.20,  0.93,  1.0  },   -- GLT_CINECAM_VERTICALSLIT
    {   "Cine Camera Horizontal Slits",   1.0,   2.0,   1.5,   0.00,  1.0,   5,
        00.0,           0.5,   0.20,  0.93,  1.0  }    -- GLT_CINECAM_HORIZONTALSLIT
};

passColors = {
	{ 0.5, 0.5, 0.5, 0.0 }, -- w
    { 0.8, 0.3, 0.3, 0.0 },
    { 1.0, 0.2, 0.2, 0.0 }, -- r
    { 0.5, 0.2, 0.6, 0.0 },
    { 0.2, 0.2, 1.0, 0.0 }, -- b
    { 0.2, 0.3, 0.7, 0.0 },
    { 0.2, 0.6, 0.2, 0.0 }, -- g
    { 0.3, 0.5, 0.3, 0.0 }
};

function GenerateAberrColors(glareDef)
	local colorWhite = { 0.63, 0.63, 0.63, 0 };
	aberrColors = {};
	for p = 1,MAX_STAR_PASSES do
		local ratio = p / MAX_STAR_PASSES;
		aberrColors[p] = {};
		for s = 1,MAX_STAR_SAMPLES do
			aberrColors[p][s] = {};
			for c = 1,4 do
				local sampleAberrComp = passColors[s][c];
				local whiteComp = colorWhite[c];
				local chromaticAberrComp = sampleAberrComp + ((whiteComp - sampleAberrComp) * ratio);
				aberrColors[p][s][c] = whiteComp + ((chromaticAberrComp - whiteComp) * glareDef[GD_CA]);
			end
			--print("( "..aberrColors[p][s][1]..", "..aberrColors[p][s][2]..", "..aberrColors[p][s][3]..", "..aberrColors[p][s][4].." )");
		end
	end
end

function GaussianDistribution(x, y, rho)
	local g = 1 / math.sqrt(2 * math.pi * rho * rho);
	g = g * (math.exp(-(x * x + y * y) / (2 * rho * rho)));
	return g;
end

function SetupBloomParameters(texSize, sampleOffsetArray, sampleWeightArray, deviation, multiplier, xAxis)
	local i = 0;
	local tu = 1 / texSize;

	-- Fill the center texel
	local weight = multiplier * GaussianDistribution(0, 0, deviation);
	SetFloatArray(sampleOffsetArray,0,0);
	SetFloatArray(sampleOffsetArray,1,0);
	
	SetFloatArray(sampleOffsetArray,60,0);
	SetFloatArray(sampleOffsetArray,61,0);
	
	SetFloatArray(sampleWeightArray,0,weight);
	SetFloatArray(sampleWeightArray,1,weight);
	SetFloatArray(sampleWeightArray,2,weight);
	SetFloatArray(sampleWeightArray,3,1);
	
	SetFloatArray(sampleWeightArray,60,0);
	SetFloatArray(sampleWeightArray,61,0);
	SetFloatArray(sampleWeightArray,62,0);
	SetFloatArray(sampleWeightArray,63,0);

	for i = 1,7 do
		-- Get the Gaussian intensity for this offset
		weight = multiplier * GaussianDistribution(i, 0, deviation);
		local i4 = i*4;
		if xAxis then
			SetFloatArray(sampleOffsetArray, i4,   i * tu);
			SetFloatArray(sampleOffsetArray, i4+1, 0);
			
			SetFloatArray(sampleOffsetArray, 28+i4,   -i * tu);
			SetFloatArray(sampleOffsetArray, 28+i4+1, 0);
		else
			SetFloatArray(sampleOffsetArray, i4,   0);
			SetFloatArray(sampleOffsetArray, i4+1, i * tu);
			
			SetFloatArray(sampleOffsetArray, 28+i4,   0);
			SetFloatArray(sampleOffsetArray, 28+i4+1, -i * tu);
		end
		SetFloatArray(sampleWeightArray, i4, weight);
		SetFloatArray(sampleWeightArray, i4+1, weight);
		SetFloatArray(sampleWeightArray, i4+2, weight);
		SetFloatArray(sampleWeightArray, i4+3, 1);
		
		SetFloatArray(sampleWeightArray, 28+i4, weight);
		SetFloatArray(sampleWeightArray, 28+i4+1, weight);
		SetFloatArray(sampleWeightArray, 28+i4+2, weight);
		SetFloatArray(sampleWeightArray, 28+i4+3, 1);
	end
end

function SetupToneMapping()
end
function SetupBrightPass()
end
function SetupStar()
end
function SetupBloom()
end

function BeginHDR()
	shaderTicket = LoadAssets(
		{"FrameworkDir","DownSample.xml","Shader","downSample"},
		{"FrameworkDir","LogLuminance.xml","Shader","logLuminance"},
		{"FrameworkDir","EyeAdaptation.xml","Shader","eyeAdaptation"},
		{"FrameworkDir","ToneMapping.xml","Shader","toneMapping"},
		{"FrameworkDir","BrightPassFilter.xml","Shader","brightPass"},
		{"FrameworkDir","BlurShader.xml","Shader","blurShader"},
		{"FrameworkDir","BloomShader.xml","Shader","bloomShader"},
		{"FrameworkDir","TextureMultiply.xml","Shader","multiplyShader"}
	);
	
	toneMappingTarget = CreateSurface(1, 1, true, "3x10f");
	logLumTargets = {};
	for i = 1,5 do
		local texWidth = math.pow(4,i-1);
		print("Creating texture of width " .. tostring(texWidth) .. " texels");
		logLumTargets[i] = CreateSurface(texWidth,texWidth,false,"1x16f");
	end
	eyeAdaptPrevious = CreateSurface(1, 1, false, "1x16f");
	eyeAdaptTarget = CreateSurface(1, 1, false, "1x16f");
	downSampleTarget = CreateSurface(1/4, 1/4, true, "4x16f");
	brightPassTarget = CreateSurface(1/4, 1/4, true, "4x16f");
	blurTarget = CreateSurface(1/4, 1/4, true, "4x16f");
	starSurfaces = {};
	for i = 1,12 do
		starSurfaces[i] = CreateSurface(1/4, 1/4, true, "4x16f");
	end
	bloomSource = CreateSurface(1/8, 1/8, true, "4x16f");
	bloomSourceBlurred = CreateSurface(1/8, 1/8, true, "4x16f");
	bloomTempTarget = CreateSurface(1/8, 1/8, true, "4x16f");
	bloomTarget = CreateSurface(1/8, 1/8, true, "4x16f");
	
	glareDef = glareDefs[7]; --[12]GLT_CINECAM_VERTICALSLIT --[5]GLT_FILTER_CROSSSCREEN
	GenerateAberrColors(glareDef);
	
	-- Only initialise these when the star def or texture size change
	starSampleOffsets = CreateFloatArray(16*4);
	starSampleWeights = CreateFloatArray(16*4);
	
	-- Only initialise these when the texture size change!
	bloomSampleOffsets = CreateFloatArray(16*4);
	bloomSampleWeights = CreateFloatArray(16*4);
end

function UpdateHDR(delta)
	if shaderTicket and IsLoaded(shaderTicket) then
		downSampleEffect = CreateEffect("downSample");
		logLuminanceEffect = CreateEffect("logLuminance");
		eyeAdaptationEffect = CreateEffect("eyeAdaptation");
		toneMappingEffect = CreateEffect("toneMapping");
		brightPassEffect = CreateEffect("brightPass");
		blurEffect = CreateEffect("blurShader");
		bloomEffect = CreateEffect("bloomShader");
		multiplyEffect = CreateEffect("multiplyShader");
		
		shadersLoaded = true;
		shaderTicket = nil;
	end
	deltaTime = delta;
end

MAX_STAR_PASSES = 3;
MAX_STAR_SAMPLES = 8;

function ShadeTonemapping(sourceSurface)
	-- Get the average log luminance
	SetBlendMode("none");
	ShadeSurface(sourceSurface,logLuminanceEffect,logLumTargets[5]);
	
	for i = 4,1,-1 do
		ShadeSurface(logLumTargets[i+1],downSampleEffect,logLumTargets[i]);
	end
	
	-- Calculate eye adaptation
	local tempTarget = eyeAdaptPrevious;
	eyeAdaptPrevious = eyeAdaptTarget;
	eyeAdaptTarget = tempTarget;
	
	SetEffectParam(eyeAdaptationEffect, 0, eyeAdaptPrevious);
	SetEffectParam(eyeAdaptationEffect, 1, deltaTime);
	
	ClearSurface(eyeAdaptTarget);
	ShadeSurface(logLumTargets[1], eyeAdaptationEffect, eyeAdaptTarget);
	
	-- Perform tone mapping
	SetEffectParam(toneMappingEffect, 0, eyeAdaptTarget);
	
	BeginTimestamp("detail",false,true);
	ShadeSurface(sourceSurface, toneMappingEffect, toneMappingTarget);
	EndTimestamp("detail",false,true);
end

function ShadeBrightPass(sourceSurface)
	-- Perform bright pass filtering
	--ShadeSurface(sourceSurface, downSampleEffect, downSampleTarget); -- 6 MILLISECONDS!!!
	ShadeSurface(sourceSurface, brightPassEffect, brightPassTarget);
	ShadeSurface(brightPassTarget, blurEffect, blurTarget);
end

function ShadeStar(brightPassSurface)
	-- Generate the Star effect
	SetBlendMode("alpha");
	starDef = starDefs[glareDef[GD_STARTYPE]];
	atanFOV = math.atan(math.pi/8);
	screenWidth, screenHeight = GetScreenSize();
	local texWidth = screenWidth/2; 
	local texHeight = screenHeight/2;
	local radOffset = glareDef[GD_ROTATE] + starDef[SD_ROTATE];
	--print("RadOffset: "..radOffset);
	local workSurface = 2;
	local destSurface;
	
	local numLines = starDef[SD_LINES]
	for d=1,numLines do
		local attnPowScale = ( atanFOV + 0.1 ) * (( 280 ) / ( texWidth + texHeight )) * 1.2;
		local radians = radOffset + ((d-1) * ((2 * math.pi) / numLines));
		-- print("Radians: "..radians);
		local XstepUV = math.sin(radians) / texWidth  * starDef[SD_LENGTH];
		local YstepUV = math.cos(radians) / texHeight * starDef[SD_LENGTH];
		-- print("UV Step: ("..XstepUV..","..YstepUV..")");
		local sourceSurface = brightPassSurface;
		
		local numPasses = starDef[SD_PASSES];
		for p=1,numPasses do
			if p == numPasses then
				destSurface = starSurfaces[d + 4];
			else
				destSurface = starSurfaces[workSurface];
			end
			
			for s=1,MAX_STAR_SAMPLES do
				local lum = math.pow(starDef[SD_ATTEN], attnPowScale * (s-1));
				local arrayIndex = (s-1)*4;
				
				local sampleOffsetX = XstepUV * (s-1);
				local sampleOffsetY = YstepUV * (s-1);
				if math.abs( sampleOffsetX ) >= 0.9 or math.abs( sampleOffsetY ) >= 0.9 then
					for c=1,4 do
						SetFloatArray(starSampleWeights, arrayIndex+c-1, 0);
					end
					SetFloatArray(starSampleOffsets, arrayIndex+0, 0);
					SetFloatArray(starSampleOffsets, arrayIndex+1, 0);
				else
					for c=1,4 do
						local sampleWeight = aberrColors[starDef[SD_PASSES]-p+1][s][c] * lum * p * 0.5;
						SetFloatArray(starSampleWeights, arrayIndex+c-1, sampleWeight);
					end
					SetFloatArray(starSampleOffsets, arrayIndex+0, sampleOffsetX);
					SetFloatArray(starSampleOffsets, arrayIndex+1, sampleOffsetY);
				end
			end
			
			SetEffectParam(bloomEffect,0,starSampleOffsets);
			SetEffectParam(bloomEffect,1,starSampleWeights);
			SetEffectParam(bloomEffect,2,MAX_STAR_SAMPLES);
			
			ShadeSurface(sourceSurface, bloomEffect, destSurface);
			
			XstepUV = XstepUV * MAX_STAR_SAMPLES;
			YstepUV = YstepUV * MAX_STAR_SAMPLES;
			attnPowScale = attnPowScale * MAX_STAR_SAMPLES;
			
			sourceSurface = starSurfaces[workSurface];
			workSurface = workSurface + 1;
			if workSurface > 3 then workSurface = 2; end
		end
	end
	
	ClearSurface(starSurfaces[1]);
	
	SetEffectParam(multiplyEffect, 0, 1);
	SetEffectParam(multiplyEffect, 1, 1);
	SetEffectParam(multiplyEffect, 2, 1);
	SetEffectParam(multiplyEffect, 3, 1/numLines);
	
	for d=1,numLines do
		ShadeSurface(starSurfaces[4+d], multiplyEffect, starSurfaces[1]);
	end
end

function ShadeBloom(brightPassSurface)
	-- Generate the Bloom effect
	if glareDef[GD_GLARE] > 0 and glareDef[GD_BLOOM] > 0 then
		-- This really should be a downsample 2x2!!
		ShadeSurface(brightPassSurface, downSampleEffect, bloomSource); 
		ShadeSurface(bloomSource, blurEffect, bloomSourceBlurred);
		
		SetEffectParam(bloomEffect, 0, bloomSampleOffsets);
		SetEffectParam(bloomEffect, 1, bloomSampleWeights);
		SetEffectParam(bloomEffect, 2, 16);
		
		ClearSurface(bloomTarget);
		
		SetupBloomParameters(screenWidth/8, bloomSampleOffsets, bloomSampleWeights, 3, 2, true);
		
		ShadeSurface(bloomSourceBlurred, bloomEffect, bloomTempTarget);
		
		SetupBloomParameters(screenHeight/8, bloomSampleOffsets, bloomSampleWeights, 3, 2, false);
		
		ShadeSurface(bloomTempTarget, bloomEffect, bloomTarget);
	end
end

function ShadeHDR(sourceSurface)
	if shadersLoaded then	
		BeginTimestamp("tonemapping",true,true);
		ShadeTonemapping(sourceSurface);
		EndTimestamp("tonemapping",true,true);
		
		BeginTimestamp("brightpass",true,true);
		ShadeBrightPass(toneMappingTarget);
		EndTimestamp("brightpass",true,true);
		
		BeginTimestamp("star",true,true);
		ShadeStar(blurTarget);
		EndTimestamp("star",true,true);
		
		BeginTimestamp("bloom",true,true);
		ShadeBloom(blurTarget);
		EndTimestamp("bloom",true,true);
		
		-- Add star and bloom textures to the tone map target
		SetBlendMode("additive");
		
		BeginTimestamp("merge",true,true);
		SetEffectParam(multiplyEffect, 0, 0.5);
		SetEffectParam(multiplyEffect, 1, 0.5);
		SetEffectParam(multiplyEffect, 2, 0.5);
		SetEffectParam(multiplyEffect, 3, 1.0);
		ShadeSurface(bloomTarget, multiplyEffect, toneMappingTarget);

		SetEffectParam(multiplyEffect, 0, 1.0);
		SetEffectParam(multiplyEffect, 1, 1.0);
		SetEffectParam(multiplyEffect, 2, 1.0);
		SetEffectParam(multiplyEffect, 3, 1.0);
		ShadeSurface(starSurfaces[1], multiplyEffect, toneMappingTarget);
		EndTimestamp("merge",true,true);
		
		SetBlendMode("none");
		
		return toneMappingTarget;
	end
	
	return sourceSurface;
end

function EndHDR()
end