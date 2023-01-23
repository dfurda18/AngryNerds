#include "ShotEntity.h"
#include "Blit3D.h"
#include "CollisionMask.h"
#include "ShotsInfo.h"

extern Blit3D *blit3D;
extern b2World *world;
extern Sprite *cannonballSprite;
extern Sprite* cannonballFuseSprite;
extern Sprite* multiBallSprite;
extern Sprite* multiBallShadowSprite;
extern Sprite* smallBallShadowSprite;
extern std::vector<ShotsInfo*> shotsInfo;

ShotEntity * MakeShot(int type)
{
	//make a shot entity
	ShotEntity *shotEntity = new ShotEntity();

	// Define the Shot body. 
	//We set its position and call the body factory.
	b2BodyDef ShotBodyDef;
	ShotBodyDef.bullet = true; //shots move fast: make sure Continuos Collision Detection (CCD) is on
	ShotBodyDef.type = b2_dynamicBody; //make it a dynamic body i.e. one moved by physics
	ShotBodyDef.position.Set(0.0f, 0.0f); //set its position in the world

	//friction won't work on a rolling circle in Box2D, so apply angular damping to the body
	//to make it slow down as it rolls
	ShotBodyDef.angularDamping = 1.8f;

	//make the userdata point back to this entity
	ShotBodyDef.userData.pointer = reinterpret_cast<uintptr_t>(shotEntity);

	shotEntity->body = world->CreateBody(&ShotBodyDef); //create the body and add it to the world
	// Define a shot shape for our body.
	//A circle shape for our shot
	b2CircleShape dynamicShot;
	shotEntity->type = type;

	//dynamicShot.m_radius = 13.f / (2 * PTM_RATIO);

	dynamicShot.m_radius = shotsInfo[shotEntity->type]->spriteSize / (2 * PTM_RATIO);
	
	/*if (shotEntity->type == SMALLBALL)
	{
		dynamicShot.m_radius = 13.f / (2 * PTM_RATIO); //shot has diameter of 13 pixels
	}
	else
	{
		dynamicShot.m_radius = 36.f / (2 * PTM_RATIO); //shot has diameter of 36 pixels
	}*/
	//create the fixture definition - we don't need to save this
	b2FixtureDef fixtureDef;
	//add a sprite to the shot entity
	shotEntity->sprite = shotsInfo[shotEntity->type]->sprite;
	shotEntity->fixedSprite = shotsInfo[shotEntity->type]->fixedSprite;
	fixtureDef.density = shotsInfo[shotEntity->type]->density;
	/*switch (shotEntity->type)
	{
	case CANNON:
		shotEntity->sprite = NULL;
		shotEntity->fixedSprite = cannonballSprite;
		fixtureDef.density = 2.0f;
		break;
	case BOMB:
		shotEntity->sprite = cannonballFuseSprite;
		shotEntity->fixedSprite = cannonballSprite;
		fixtureDef.density = 2.0f;
		break;
	case MULTIBALL:
		shotEntity->sprite = multiBallSprite;
		shotEntity->fixedSprite = multiBallShadowSprite;
		fixtureDef.density = 2.0f;
		break;
	case SMALLBALL:
		shotEntity->sprite = smallBallShadowSprite;
		shotEntity->fixedSprite = NULL;
		fixtureDef.density = 8.0f;
		break;
	}*/
	//collison masking
	fixtureDef.filter.categoryBits = CMASK_SHOT;  //this is a shot
	fixtureDef.filter.maskBits = CMASK_SHOT | CMASK_EDGES | CMASK_BLOCK | CMASK_GROUND | CMASK_ALIEN;//it collides wth lotsa stuff

	// Define the dynamic body fixture.
	fixtureDef.shape = &dynamicShot;


	

	// Override the default friction.
	fixtureDef.friction = 0.4f;

	//restitution makes us bounce; use 0 for no bounce, 1 for perfect bounce
	fixtureDef.restitution = 0.2f;

	// Add the shape to the body. 
	shotEntity->body->CreateFixture(&fixtureDef);
	shotEntity->body->ApplyAngularImpulse(0.2f,true);
	

	

	return shotEntity;
}