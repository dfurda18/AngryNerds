#include "AlienEntity.h"

extern b2World* world;
extern Sprite* normalAlienSprite;
extern Sprite* bossAlienSprite;

AlienEntity* MakeAlien(AlienType aType, b2Vec2 pixelCoords,	float angleInDegrees)
{
	AlienEntity* alienEntity = new AlienEntity();
	alienEntity->alienType = aType;

	b2BodyDef bodyDef;

	bodyDef.type = b2_dynamicBody; //make it a dynamic body i.e. one moved by physics
	bodyDef.position = Pixels2Physics(pixelCoords); //set its position in the world
	bodyDef.angle = deg2rad(angleInDegrees);

	bodyDef.angularDamping = 1.8f;

	bodyDef.userData.pointer = reinterpret_cast<uintptr_t> (alienEntity);

	alienEntity->body = world->CreateBody(&bodyDef); //create the body and add it to the world

	b2FixtureDef fixtureDef;

	// Define a shape for our body.
	b2PolygonShape polygon;
	b2CircleShape circle;
	// This defines a triangle in CCW order.
	b2Vec2 vertices[3];

	switch (aType)
	{
	case AlienType::NORMAL:
		circle.m_radius = 65 / (2 * PTM_RATIO); //ball has diameter ? pixels
		alienEntity->maxHP = alienEntity->hp = 100;
		alienEntity->sprite = normalAlienSprite;
		break;
	case AlienType::BOSS:
		circle.m_radius = 109 / (2 * PTM_RATIO); //ball has diameter ? pixels
		alienEntity->maxHP = alienEntity->hp = 300;
		alienEntity->sprite = bossAlienSprite;
		break;
	}//end switch(aType)

	fixtureDef.shape = &circle;

	fixtureDef.density = 0.2f;
	fixtureDef.restitution = 0.02;
	fixtureDef.friction = 0.1;
	
	alienEntity->body->CreateFixture(&fixtureDef);

	return alienEntity;
}