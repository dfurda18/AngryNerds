#pragma once

#include "Blit3D.h"
#include "Physics.h"

class Cannon
{
public:
	b2Vec2 positionPixels;
	float angle;
	float levelAngle;
	Sprite *cannonSprite;
	Sprite* baseSprite;
	Sprite* bodySprite;
	Sprite* angleSprite;
	float rotateDir;

	Cannon() 
	{
		positionPixels = b2Vec2(0, 0);
		angle = 0;
		levelAngle = 0;
		cannonSprite = NULL;
		baseSprite = NULL;
		bodySprite = NULL;
		angleSprite = NULL;
		rotateDir = 0;
	}

	void Draw()
	{
		baseSprite->Blit(positionPixels.x, positionPixels.y - 50);
		cannonSprite->angle = angle;
		cannonSprite->Blit(positionPixels.x, positionPixels.y);
		bodySprite->Blit(positionPixels.x, positionPixels.y);
		angleSprite->angle = levelAngle;
		angleSprite->Blit(positionPixels.x + 3, positionPixels.y - 25);
	}

	void Update(float seconds)
	{
		angle += rotateDir * seconds * 20;
		
		if (angle < -10) angle = -10;
		if (angle > 90) angle = 90;
		levelAngle = angle * 16;
	}
};