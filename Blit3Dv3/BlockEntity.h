#pragma once

#include "Entity.h"

enum class BlockType {RECTANGLE = 0, LARGE_RECTANGLE, LARGE_TRIANGLE, SHIP};

enum class MaterialType {GLASS = 0, WOOD, METAL, SHIP};

extern Sprite* bossAlienSprite;

class BlockEntity : public Entity
{
public:
	BlockType blockType;
	MaterialType materialType;
	int hp;
	int maxHP;
	std::vector<Sprite *> spriteList;

	BlockEntity()
	{
		typeID = ENTITYBLOCK;
		blockType = BlockType::LARGE_TRIANGLE;
		materialType = MaterialType::WOOD;
		maxHP = hp = 100;
	}

	//Damage() returns true if we should kill this object
	bool Damage(int damage)
	{
		hp -= damage;
		if (hp < 1) return true;
		if (hp < maxHP/3) sprite = spriteList[1];
		else if (hp < maxHP * 0.66) sprite = spriteList[0];

		return false;
	}
	void Draw()
	{
		position = body->GetPosition();
		position = Physics2Pixels(position);

		sprite->angle = rad2deg(body->GetAngle());

		// Draw the objects
		if (blockType == BlockType::SHIP)
		{
			bossAlienSprite->angle = rad2deg(body->GetAngle());
			bossAlienSprite->Blit(position.x + 15, position.y);
		}
		sprite->Blit(position.x, position.y);
	}
};

BlockEntity * MakeBlock(BlockType btype, MaterialType mtype, b2Vec2 pixelCoords, 
	float angleInDegrees);