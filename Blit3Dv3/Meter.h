#pragma once

#include "Blit3D.h"
#include "Physics.h"

class Meter
{
public:
	b2Vec2 positionPixels;	
	Sprite *sprite;
	float scale;
	float angle;
	int dir;
	bool shooting;

	Meter()
	{
		positionPixels = b2Vec2(0, 0);
		scale = 0.1f;
		angle = scale * 90;
		sprite = NULL;
		dir = 1;
		shooting = false;
	}

	void Draw()
	{
		sprite->angle = angle;
		sprite->Blit(positionPixels.x, positionPixels.y);
	}

	void Update(float seconds)
	{
		if (shooting)
		{
			scale += seconds * dir;

			if (scale < 0.1f)
			{
				scale = 0.1f;
				dir = 1;
			}
			else if (scale > 1.0f)
			{
				scale = 1.0f;
				dir = -1;
			}
		}
		angle = scale * 90;
	}
};