#pragma once

#include "Entity.h"

class GroundEntity : public Entity
{
public:
	GroundEntity()
	{
		typeID = ENTITYGROUND;
	}

	void Draw() {}; //empty draw, as ground does not draw
};