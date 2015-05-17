
-- Okay, how to do the choreography...
-- 1. More than one tweened camera movement should be possible per shot
-- 2. Should be able to jump to any specific shot through the console
-- 3. Should be able to cue actions within a shot using one or more 'magic buttons'
-- 4. Should be able to trigger restrings, special effects, physics properties etc.

-- Functions?

setupCS1 = true;
function CS1(progress) -- okay
	if setupCS1 then
		PlaySound(waterLily);
		SetLightColor(spot,0,0,0);
		SetLightColor(ambient,0,0,0);
		setupCS1 = false;
	end
	
	local cameraPos1 = CreateVector(-0.21, 0.05,-0.55, 1);
	local cameraTgt1 = CreateVector(-0.46, 0.14,-1.48, 1);
	local cameraPos2 = CreateVector( 0.47,-0.09, 0.95, 1);
	local cameraTgt2 = CreateVector(-0.32,-0.32,-0.69, 1);
	
	if progress >= 12 then Cutscene(csPanAcrossTable) end
	
	local cameraPos = Easing.linear(progress, cameraPos1, cameraPos2 - cameraPos1, 12.2);
	local cameraTgt = Easing.linear(progress, cameraTgt1, cameraTgt2 - cameraTgt1, 12.2);
	
	SetCameraPosition(performanceCamera, cameraPos.x, cameraPos.y, cameraPos.z);
	SetCameraTarget(performanceCamera, cameraTgt.x, cameraTgt.y, cameraTgt.z);
	
	if progress > 4 then
		local lightColor = Easing.linear(progress-4, 0, 1, 8.2);
		SetLightColor(spot, lightColor,lightColor,lightColor);
	end
end

function csPanAcrossTable(progress) -- weak
	cameraPos1 = CreateVector(1.311497669614,-0.087275455240438,0.83127248782394, 1);
	cameraTgt1 = CreateVector(1.10341969155019,-0.364396330297,-0.12952861900218, 1);
	cameraPos2 = CreateVector(-1.7794403279456,-0.21917106436887,0.62660954712759, 1);   
	cameraTgt2 = CreateVector(-1.5509603484754,-0.4088118956667,-0.25681631976669, 1);   

	if progress >= 9.5 then Cutscene(csDownFromSky) end
	
	local cameraPos = Easing.linear(progress, cameraPos1, cameraPos2 - cameraPos1, 10);
	local cameraTgt = Easing.linear(progress, cameraTgt1, cameraTgt2 - cameraTgt1, 10);
	
	SetCameraPosition(performanceCamera, cameraPos.x, cameraPos.y, cameraPos.z);
	SetCameraTarget(performanceCamera, cameraTgt.x, cameraTgt.y, cameraTgt.z);
end

function csDownFromSky(progress) -- good!
	cameraPos1 = CreateVector(1.5827534542447,1.214914791749,0.84395068446955,1)   
	cameraTgt1 = CreateVector(1.2603276908486,0.43734207299804,0.30411525199706,1)   
	cameraPos2 = CreateVector(-1.5789526447216,-0.89242552215229,0.72446486428968,1)   
	cameraTgt2 = CreateVector(-1.37401389805484,-1.0228492308904,-0.21987705600876,1)

	if progress >= 12 then Cutscene(csLookAroundRoom) end
	
	local cameraPos = Easing.linear(progress, cameraPos1, cameraPos2 - cameraPos1, 12);
	local cameraTgt = Easing.linear(progress, cameraTgt1, cameraTgt2 - cameraTgt1, 12);
	
	SetCameraPosition(performanceCamera, cameraPos.x, cameraPos.y, cameraPos.z);
	SetCameraTarget(performanceCamera, cameraTgt.x, cameraTgt.y, cameraTgt.z);
	SetCameraClipFov(performanceCamera, 0.01, 20, 0.985);
end

setupCsLookAroundRoom = true;
finishCsLookAroundRoom = false;
function csLookAroundRoom(progress) -- okay
	if setupCsLookAroundRoom then
		SetLightColor(ambient,0.04,0.04,0.06);
		table.insert(lights,ambient);
		setupCsLookAroundRoom = false;
	end
	
	--Right
	cameraPos1 = CreateVector(-0.042911636010209,0.55410970542778,2.0039974064918,1)   
	cameraTgt1 = CreateVector(-0.22868388177035,0.21987197830328,1.0799985891377,1)
	-- Left
	cameraPos2 = CreateVector(-0.042911636010209,0.55410970542778,2.0039974064918,1)   
	cameraTgt2 = CreateVector(0.15054756208961,0.29625967289511,1.0573787401887,1)   
	-- Door
	cameraPos3 = CreateVector(-0.042911636010209,0.55410970542778,2.0039974064918,1)   
	cameraTgt3 = CreateVector(-1.0118477624079,0.36446887412995,1.8452550426877,1)
	-- Table
	cameraTgt4 = CreateVector(0,-0.5,0,1)
	
	if progress < 3 then
		SetSoundVolume(waterLily,1-(progress/3));
	else
		PauseSound(waterLily);
	end
	
	if progress < 3.5 then
		cameraPos = Easing.inOutCubic(progress, cameraPos1, cameraPos2 - cameraPos1, 3.5);
		cameraTgt = Easing.inOutCubic(progress, cameraTgt1, cameraTgt2 - cameraTgt1, 3.5);
	elseif progress < 8 then
		cameraPos = Easing.inOutCubic(progress-3.5, cameraPos2, cameraPos3 - cameraPos2, 4.5);
		cameraTgt = Easing.inOutCubic(progress-3.5, cameraTgt2, cameraTgt3 - cameraTgt2, 4.5);
	else
		if triggeredThisFrame then
			finishCsLookAroundRoom = true;
			triggeredProgress = progress;
			triggeredCameraPos = cameraPos3 + CreateVector(-0.3 * (triggeredProgress-8), 0.05 * math.sin((triggeredProgress-8) * 4), 0.0, 1);
			triggeredCameraTgt = triggeredCameraPos + (cameraTgt3 - cameraPos3);
		end
		if not finishCsLookAroundRoom then
			progress = progress - 8;
			cameraPos = cameraPos3 + CreateVector(-0.3 * progress, 0.05 * math.sin(progress * 4), 0.0, 1);
			cameraTgt = cameraPos + (cameraTgt3 - cameraPos3);
		else
			if progress > triggeredProgress + 4.5 then
				table.removeObject(lights,ambient);
				Cutscene(csRevealPuppet);
			end
			if progress >= triggeredProgress + 3.5 then
				progress = triggeredProgress + 3.5
			end
			cameraPos = triggeredCameraPos;
			cameraTgt = Easing.inOutCubic(progress-triggeredProgress, triggeredCameraTgt, cameraTgt4-triggeredCameraTgt, 3.5);
		end
	end
	
	SetCameraPosition(performanceCamera, cameraPos.x, cameraPos.y, cameraPos.z);
	SetCameraTarget(performanceCamera, cameraTgt.x, cameraTgt.y, cameraTgt.z);
end

function csRevealPuppet(progress)
	cameraPos1 = CreateVector(0.31512872055772,-0.4872957256432,0.10963466355735,1)   
	cameraTgt1 = CreateVector(-0.081523114934515,-0.43731655637252,-0.80697288825513,1)   
	cameraPos2 = CreateVector(0.05364585454275,-0.06703215498734,0.11771549576339,1)   
	cameraTgt2 = CreateVector(0.46318000729417,-0.33298438009218,-0.7486060973461,1)
	
	-- Do we want to mess around with FOV here? Yes. Yes, I think we do.
	
	if progress >= 15 then progress = 15 end
	
	cameraPos = Easing.outQuad(progress, cameraPos1, cameraPos2 - cameraPos1, 15);
	cameraTgt = Easing.outQuad(progress, cameraTgt1, cameraTgt2 - cameraTgt1, 15);
	
	SetCameraPosition(performanceCamera, cameraPos.x, cameraPos.y, cameraPos.z);
	SetCameraTarget(performanceCamera, cameraTgt.x, cameraTgt.y, cameraTgt.z);
	
	if triggeredThisFrame then
		Cutscene(csPuppetWalk);
	end
end

function csPuppetWalk(progress)
	if not csPuppetWalkReady then
		AddDominoes();
		AddBall();
		csPuppetWalkReady = true;
	end
	
	ballPosX, ballPosY, ballPosZ = GetPhysicsPosition(physicsBall);
	if ballPosY < -0.5 or triggeredThisFrame then
		Cutscene(csBallFall);
	end
	
	SetCameraPosition(performanceCamera,-0.5,0.5,2);
    SetCameraTarget(performanceCamera,0,-0.5,0);
	
end

function csBallFall(progress)
	
	if triggeredThisFrame then
		Cutscene(csCrescendo);
	end
	
end

function csCrescendo(progress)
	if not csCrescendoReady then
		RemoveDominoes();
		RemoveBall();
		drawTools = false;
		
		csCrescendoReady = true;
	end
	
	if triggeredThisFrame then
		Cutscene(csSitDown);
	end
end

setupCsSitDown = true;
function csSitDown(progress)
	if setupCsSitDown then
		AddToPhysicsWorld(physicsWorld,physicsSeatBase,true);
		AddToPhysicsWorld(physicsWorld,physicsSeatBack,true);
		
		SetPhysicsPosition(physicsSeatBase,0,-0.44,-0.22);
		
		SetPhysicsPosition(physicsSeatBack,0,-0.3,-0.4);
		SetPhysicsRotation(physicsSeatBack,1.4,0,0);
		
		drawDebugSeat = true;
		
		setupCsSitDown = false;
	end
	
	if triggeredThisFrame then
		Cutscene(csRevealCar);
	end
end

function csRevealCar(progress)
	-- FROM
	cameraPos = CreateVector(-0.37819048771597,-0.27019777225744,0.31891883941319,1)   
	cameraTgt = CreateVector(0.11511640411956,-0.38990997954636,-0.5426595365304,1)
	-- TO
	cameraPos = CreateVector(-0.44343098748717,0.036967511362683,-0.2628467366354,1)   
	cameraTgt = CreateVector(-0.43192195867713,-0.082744695926236,0.72989518837929,1)   
	
	drawCar = true;
	
	if progress > 8 then progress = 8 end;
	
	local revealVal = Easing.linear(progress, -1, 2, 8);
	
	SetEffectParam(revealEffect,5,revealVal+1);
	SetEffectParam(revealEffect,8,revealVal-1);
	
	if triggeredThisFrame then
		Cutscene(csCrash);
	end
end

function csDriving(progress)
	
end

function csCrash(progress)
	
	
	
end

function csRevealHospital(progress)
	
end
