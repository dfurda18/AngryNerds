#pragma once

#include "Entity.h"

enum class AlienType { NORMAL = 0, BOSS };

class AlienEntity : public Entity
{
public:
	AlienType alienType;
	int hp;
	int maxHP;

	AlienEntity()
	{
		typeID = ENTITYALIEN;
		alienType = AlienType::NORMAL;
		maxHP = hp = 100;
	}

	//Damage() returns true if we should kill this object
	bool Damage(int damage)
	{
		hp -= damage;
		if (hp < 1) return true;
		return false;
	}
};

AlienEntity* MakeAlien(AlienType aType, b2Vec2 pixelCoords,
	float angleInDegrees);