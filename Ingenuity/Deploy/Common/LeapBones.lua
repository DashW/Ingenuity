
leap = {}

leap.left = {}
leap.right = {}

leap.bonesPerFinger = 4;
leap.fingersPerHand = 5;
leap.bonesPerHand = (leap.bonesPerFinger * leap.fingersPerHand) + 2;

local h = 0;
for hindex,hand in [leap.left,leap.right] do
	hand.pinky = {}
	hand.ring = {}
	hand.middle = {}
	hand.index = {}
	hand.thumb = {}
	
	local f = 0;
	for findex,finger in [hand.pinky,hand.ring,hand.middle,hand.index,hand.thumb] do
		finger.meta  = (h * leap.bonesPerHand) + (f * leap.bonesPerFinger) + 0;
		finger.prox  = (h * leap.bonesPerHand) + (f * leap.bonesPerFinger) + 1;
		finger.inter = (h * leap.bonesPerHand) + (f * leap.bonesPerFinger) + 2;
		finger.dist  = (h * leap.bonesPerHand) + (f * leap.bonesPerFinger) + 3;
		hand[findex] = finger;
		f = f + 1;
	end
	
	hand.palm = (h * 22) + 20;
	hand.arm = (h * 22) + 21;
	leap[hindex] = hand;
	
	h = h + 1;
end
