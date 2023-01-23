#pragma once

#include "Entity.h"

//enum ShotTypes { CANNON = 0, BOMB, MULTIBALL, SMALLBALL };

class ShotEntity : public Entity
{
public:
	Sprite* fixedSprite;
	int type;
	bool stopped;
	b2Vec2 lastPosition;
	ShotEntity()
	{
		typeID = ENTITYSHOT;
		stopped = false;
	}
	void Draw()
	{
		position = body->GetPosition();
		position = Physics2Pixels(position);

		if (sprite != NULL)
		{
			sprite->angle = rad2deg(body->GetAngle());

			// Draw the objects
			sprite->Blit(position.x, position.y);
		}
		if (fixedSprite != NULL)
		{
			fixedSprite->Blit(position.x, position.y);
		}
	}
	void Update(float seconds)
	{
		b2Vec2 movement = body->GetPosition() - lastPosition;
		if (movement.Length() < 0.00001f)
		{
			stopped = true;
		}
		lastPosition = body->GetPosition();
	}
};

ShotEntity * MakeShot(int type);