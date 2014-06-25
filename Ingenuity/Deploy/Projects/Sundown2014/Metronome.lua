
metronomeCurrentBeat = -1;

function SetMetronomeBeat(bpm,offset)
	metronomeBps = bpm/60;
	metronomeOffset = offset;
end

function SetupMetronome(sound,bpm,offset)
	metronomeSound = sound;
	metronomeCurrentBeat = -1;
	SetMetronomeBeat(bpm,offset);
end

function UpdateMetronome(delta)
	local soundProgress = GetSoundProgress(metronomeSound);
	metronomePreviousBeat = metronomeCurrentBeat;
	metronomeCurrentBeat = math.floor((soundProgress - metronomeOffset) * metronomeBps);
	if metronomePreviousBeat ~= metronomeCurrentBeat then
		metronomeBeatThisFrame = metronomeCurrentBeat;
	end
end

function GetMetronomeBeat()
	return metronomeCurrentBeat;
end

function GetMetronomeBeatTime(beatNum)
	return (beatNum / metronomeBps) + metronomeOffset;
end
