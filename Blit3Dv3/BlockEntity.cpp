#include "BlockEntity.h"

extern b2World *world;
extern std::vector<Sprite *> blockSprites;

BlockEntity * MakeBlock(BlockType btype, MaterialType mtype, b2Vec2 pixelCoords,
	float angleInDegrees)
{
	BlockEntity * blockEntity = new BlockEntity();
	blockEntity->blockType = btype;
	blockEntity->materialType = mtype;

	b2BodyDef bodyDef;
	
	bodyDef.type = b2_dynamicBody; //make it a dynamic body i.e. one moved by physics
	bodyDef.position = Pixels2Physics(pixelCoords); //set its position in the world
	bodyDef.angle = deg2rad(angleInDegrees);

	bodyDef.angularDamping = 1.8f;

	bodyDef.userData.pointer = reinterpret_cast<uintptr_t> (blockEntity);

	blockEntity->body = world->CreateBody(&bodyDef); //create the body and add it to the world

	b2FixtureDef fixtureDef;

	// Define a shape for our body.
	b2PolygonShape polygon;
	b2CircleShape circle;

	switch (btype)
	{
	case BlockType::RECTANGLE:
		polygon.SetAsBox(39 / PTM_RATIO, 58 / PTM_RATIO, b2Vec2(0, 0), 0);
		fixtureDef.shape = &polygon;
		break;
	case BlockType::LARGE_RECTANGLE:
		polygon.SetAsBox(186.5 / PTM_RATIO, 10.5 / PTM_RATIO, b2Vec2(0, 0), 0);
		fixtureDef.shape = &polygon;
		break;
	break;
	case BlockType::LARGE_TRIANGLE:
	{
		// This defines a triangle in CCW order.
		b2Vec2 vertices[3];
		
		float cx = 580 + 140 / 2;
		float cy = 1840 + 70 / 2;

		vertices[0].Set((650.f - cx) / PTM_RATIO, (cy - 1841.0f) / PTM_RATIO);
		vertices[1].Set((581.0f - cx) / PTM_RATIO, (cy - 1909.0f) / PTM_RATIO);
		vertices[2].Set((719.0f - cx) / PTM_RATIO, (cy - 1909.0f) / PTM_RATIO);

		int32 count = 3;		

		polygon.Set(vertices, count);

		fixtureDef.shape = &polygon;
	}
		break;
	case BlockType::SHIP:
	{
		// This defines a triangle in CCW order.
		b2Vec2 vertices[3];

		circle.m_radius = 164 / (2 * PTM_RATIO); //ball has diameter ? pixels

		fixtureDef.shape = &circle;
	}
	break;
	}//end switch(btype)

	switch (mtype)
	{
	case MaterialType::GLASS:
		fixtureDef.density = 0.3f;
		fixtureDef.restitution = 0.02;
		fixtureDef.friction = 0.1;
		blockEntity->maxHP = blockEntity->hp = 100;
		break;
	case MaterialType::WOOD:
		fixtureDef.density = 0.3f;
		fixtureDef.restitution = 0.05;
		fixtureDef.friction = 0.8;
		blockEntity->maxHP = blockEntity->hp = 200;
		break;
	case MaterialType::METAL:
		fixtureDef.density = 0.2f;
		fixtureDef.restitution = 0.1;
		fixtureDef.friction = 0.8;
		blockEntity->maxHP = blockEntity->hp = 300;
		break;
	case MaterialType::SHIP:
		fixtureDef.density = 0.2f;
		fixtureDef.restitution = 0.1;
		fixtureDef.friction = 0.8;
		blockEntity->maxHP = blockEntity->hp = 300;
		break;
	}//end switch(mtype)

	blockEntity->body->CreateFixture(&fixtureDef);

	int materialCounter = (int)mtype;
	if (materialCounter > 2)
	{
		materialCounter = 0;
	}
	//add sprites
	blockEntity->sprite = blockSprites[(int)btype * 9 + materialCounter *3];
	blockEntity->spriteList.push_back(blockSprites[(int)btype * 9 + materialCounter * 3 + 1]);
	blockEntity->spriteList.push_back(blockSprites[(int)btype * 9 + materialCounter * 3 + 2]);

	

	return blockEntity;	
}