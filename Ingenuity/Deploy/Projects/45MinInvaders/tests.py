""" Ingenuity Test Script """

import ingenuity

positionX = 0.0

class Sprite:
	def __init__(self, path, centerX, centerY, size):
		self.x = 0.0
		self.y = 0.0
		self.centerX = centerX
		self.centerY = centerY
		self.size = size
		self.pixelSpace = 0
		self.blackAsAlpha = 0
		tex = ingenuity.CreateTextureFromFile(path)
		self.objectRef = ingenuity.CreateSprite(tex, centerX, centerY, size)
		ingenuity.SetSpriteFlags(self.objectRef,0,0)
		return
	def move(self):
		ingenuity.SetSpritePosition(self.objectRef, self.x, self.y)
	def draw(self):
		ingenuity.DrawSprite(self.objectRef)
		return

def begin():
	global font, tex, invader, tank, rocket, back
	
	print("Starting Up")
	# if __name__ == '__main__':
		# fifty = 50;
		# print(fifty)
	
	print("Creating Font");
	font = ingenuity.CreateFont(80, "Arial")
	if(font):
		print("Font Created")
	
	invader = Sprite("invader.png", 64, 64, 1.0)
	tank = Sprite("tank.png", 64, 64, 1.0)
	rocket = Sprite("rocket.png", 32, 32, 1.0)
	back = Sprite("back.png", 1024, 1024, 0.675)
	
	tank.x = 0.0
	tank.y = 0.7
	tank.move(tank.x, tank.y)
	
	rocket.alive = 0
	
	invader.x = -1.0
	invader.y = -0.7
	invader.alive = 1
	invader.direction = 1 #right
	return
	
def distance(x1, y1, x2, y2):
	# FIXME
	xDist = x2 - x1
	if(xDist < 0):
		xDist *= -1
	yDist = y2 - y1
	if(yDist < 0):
		yDist *= -1
	return (xDist + yDist)
	
def moveinvader(delta):
	if(invader.alive):
		if(invader.direction > 0):
			invader.x += delta
			if(invader.x > 1.0):
				invader.direction = 0
		if(invader.direction < 1):
			invader.x -= delta
			if(invader.x < -1.0):
				invader.direction = 1
		invader.move()
	return

def update(delta):
	global positionX, invader, tank
	if(ingenuity.GetKeyHeld(0x27)):
		tank.x += delta
		tank.move(tank.x,tank.y)
	if(ingenuity.GetKeyHeld(0x25)):
		tank.x -= delta
		tank.move(tank.x,tank.y)
	if(ingenuity.GetKeyDown(0x26) and rocket.alive < 1):
		rocket.x = tank.x
		rocket.y = tank.y - 0.2
		rocket.alive = 1
	if(rocket.alive):
		rocket.y -= delta
		rocket.move()
		if(rocket.y < -1.0):
			rocket.alive = 0
	moveinvader(delta)
	
	if(invader.alive and rocket.alive):
		if(distance(rocket.x,rocket.y,invader.x,invader.y) < 0.17):
			invader.alive = 0
			rocket.alive = 0
	
	return

def draw():
	global font, positionX, invader, tank, rocket, back
	back.draw()
	if(rocket.alive): 
		rocket.draw()
	if(invader.alive):
		invader.draw()
	else:
		ingenuity.DrawText(font,"Congratumalations!!!",0,0,0)
	tank.draw()
	return

def end():
	return

print("Script file loaded!")

