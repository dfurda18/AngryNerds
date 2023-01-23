/*
	"Angry Birds" style example
*/
#include "Blit3D.h"
#include <random>

#include "Box2d/Box2d.h"
#include "GroundEntity.h"
#include "ShotEntity.h"
#include "Particle.h"
#include "Physics.h"
#include "MyContactListener.h" 
#include "EdgeEntity.h"
#include "Cannon.h"
#include "Meter.h"
#include "Camera.h"
#include "BlockEntity.h"
#include "AlienEntity.h"
#include "Explosion.h"
#include "ShotsInfo.h"

#include "CollisionMask.h"

// Import LUA
#include <lua.hpp>
#include <stdlib.h>
#include <crtdbg.h>



//memory leak detection
#define CRTDBG_MAP_ALLOC
#ifdef _DEBUG
#ifndef DBG_NEW
#define DBG_NEW new ( _NORMAL_BLOCK , __FILE__ , __LINE__ )
#define new DBG_NEW
#endif
#endif  // _DEBUG



//GLOBAL DATA
Blit3D* blit3D = NULL;
lua_State* L;		// Instantiate Lua
b2World *world;
// Prepare for simulation. Typically we use a time step of 1/60 of a
// second (60Hz) and ~10 iterations. This provides a high quality simulation
// in most game scenarios.
int32 velocityIterations = 8;
int32 positionIterations = 3;
float timeStep = 1.f / 60.f; //one 60th of a second
float elapsedTime = 0; //used for calculating time passed
float settleTime = 0;

//contact listener to handle collisions between important objects
MyContactListener *contactListener;

enum GameState { START, PLAYING, SETTLING, GAMEOVER };
GameState gameState = SETTLING;
bool attachedShot = true; //is the ball ready to be launched from the paddle?
int lives = 3;

std::vector<Entity *> blockEntityList; //bricks go here
std::vector<Entity*> alienEntityList; //bricks go here
std::vector<Entity *> shotEntityList; //track the balls seperately from everything else
std::vector<Entity *> entityList; //other entities in our game go here
std::vector<Entity *> deadEntityList; //dead entities

std::vector<Particle *> particleList;

std::vector<ShotsInfo*> shotsInfo;

/*Sprite* cannonballSprite = NULL;
Sprite* cannonballFuseSprite = NULL;
Sprite* multiBallSprite = NULL;
Sprite* multiBallShadowSprite = NULL;
Sprite* smallBallShadowSprite = NULL;
Sprite* bombCharacterSprite = NULL;
Sprite* multiCharacterSprite = NULL;
*/
Sprite *groundSprite = NULL;
Sprite *cactusSprite = NULL;
Sprite* normalAlienSprite = NULL;
Sprite* bossAlienSprite = NULL;
Sprite* gameOverScreen = NULL;
Sprite* titleScreen = NULL;

std::vector<Sprite*> explosionSpriteList;

Cannon cannon;

bool fireShotNow = false;
bool followingShot = false; //is the camera tracking the shot?

Meter meter;

Camera2D *camera; //pans the view

std::vector<Sprite *> blockSprites;

std::vector<Sprite *> debrisList;

std::vector<Sprite*> cloudList;

std::vector<Explosion*> explosionList;

int score = 0;

std::vector<int> remainingShots;
int currentShotType;
bool justChangedBall;
bool characterIsShowing;
bool characterGoingForward;
bool shooting;
bool mustDestroyCurrentBall;
bool bossSpawned;
ShotEntity* currentShot;
int characterXposition;

//font
AngelcodeFont* font = NULL;

//simple class to hold graphics-only elements of the level
class GraphicElement
{
public:
	Sprite* sprite;
	glm::vec2 pos;
	void Draw()
	{
		sprite->Blit(pos.x, pos.y);
	};
};

std::vector<GraphicElement> elementList;

std::mt19937 rng;

/*
* Check if certain entity is within a circle range of the current shot
*/
bool IsWithinRange(Entity* entity, float range)
{
	b2Vec2 distance = entity->body->GetPosition() - currentShot->body->GetPosition();
	if (distance.Length() < range) // 5
	{
		return true;
	}
	return false;
}

/*
* Method that Lua can call to apply a force to objects within certain range
*/
int ApplyForceWithinRange(lua_State* L)
{
	float range = lua_tonumber(L, 1);
	for (auto& b : blockEntityList) {
		if (IsWithinRange(b, range))
		{
			b2Vec2 force = b->body->GetPosition() - currentShot->body->GetPosition();
			float forceLength = force.Length();
			force.Normalize();
			force *= 1 / force.Length();
			b->body->ApplyLinearImpulse(force, b->body->GetPosition(), true);
		}
	}
	for (auto& b : alienEntityList) {
		if (IsWithinRange(b, range))
		{
			b2Vec2 force = currentShot->body->GetPosition() - b->body->GetPosition();
			float forceLength = force.Length();
			force.Normalize();
			force *= 1 / force.Length();
			b->body->ApplyLinearImpulse(force, b->body->GetPosition(), true);
		}
	}
	lua_pushnumber(L, 1);
	return 1;
}
/*
* Method Lua can call to show a explosion
*/
int ShowExplosion(lua_State* L)
{
	explosionList.push_back(new Explosion(currentShot->body->GetPosition(), explosionSpriteList, 0.f));
	lua_pushnumber(L, 1);
	return 1;
}
/*
* Method Lua can call to destroy the current shot
*/
int DestroyCurrentShot(lua_State* L) 
{
	mustDestroyCurrentBall = true;
	lua_pushnumber(L, 1);
	return 1;
}
/*
* Method Lua can call to make a new ball
*/
int MakeNewBall(lua_State* L)
{
	int ballNumber = lua_tonumber(L, 1);
	int type = lua_tonumber(L, 2);
	int intensity = lua_tonumber(L, 3);
	ShotEntity* shot = MakeShot(type);
	b2Vec2 pos = currentShot->body->GetPosition();

	shot->body->SetTransform(pos, currentShot->body->GetAngle());
	shot->lastPosition = pos;
	shot->stopped = false;
	b2Vec2 dir = deg2vec(currentShot->body->GetAngle() + (-50 + (ballNumber * 10)), currentShot->body->GetLinearVelocity().Length() / 100);
	dir *= intensity; // 2
	shot->body->ApplyLinearImpulse(dir, shot->body->GetPosition(), true);
	shotEntityList.push_back(shot);
	shooting = true;
	lua_pushnumber(L, 1);
	return 1;
}
/*
* Get the shots info from lua
*/
void LuaGetShotsInfo()
{
	std::vector<std::string> tempInfo;
	int numberOfShots;
	int elementSize;
	//the function name
	lua_getglobal(L, "getShotsSize");

	//call the function with 2 arguments, return 1 result
	lua_call(L, 0, 1);

	//get the result
	numberOfShots = (int)lua_tointeger(L, -1);

	//remove the result from the lua stack
	lua_pop(L, 1);

	shotsInfo.clear();

	for (int counter = 1; counter <= numberOfShots; counter++)
	{
		//the function name
		lua_getglobal(L, "getShotInfo");
		lua_pushnumber(L, counter);

		//call the function with 2 arguments, return 1 result
		lua_call(L, 1, 13);

		tempInfo.clear();

		for (int i = 0; i < 13; i++)
		{
			tempInfo.push_back(lua_tostring(L, -1));
			lua_pop(L, 1);
		}

		shotsInfo.push_back(new ShotsInfo(tempInfo[0], tempInfo[1], tempInfo[2], tempInfo[3], tempInfo[4], tempInfo[5], tempInfo[6], tempInfo[7], tempInfo[8], tempInfo[9], tempInfo[10], tempInfo[11], tempInfo[12]));
	}
}
/*
* Activate ability from lua
*/
void ActivateAbility()
{
	//the function name
	lua_getglobal(L, "activateAbility");

	lua_pushnumber(L, shotsInfo[currentShot->type]->actionId);

	//call the function with 2 arguments, return 1 result
	lua_call(L, 1, 0);
}

//ensures that entities are only added ONCE to the deadEntityList
void AddToDeadList(Entity *e)
{
	bool unique = true;

	for (auto ent : deadEntityList)
	{
		if (ent == e)
		{
			unique = false;
			break;
		}
	}

	if (unique) deadEntityList.push_back(e);
}

//Function that applies damage etc when a block collides with anything
void BlockCollide(Entity *A, float maxImpulseAB)
{
	BlockEntity *blockEntity = (BlockEntity *)A;
	//damage?
	if (maxImpulseAB > 0.2f) //cutoff for no damage
	{
		//apply some damage
		if (blockEntity->Damage((int)(maxImpulseAB *30.f)))
		{
			//Damage() returned true, need to kill this block
			AddToDeadList(A);

			if (blockEntity->blockType == BlockType::SHIP && !bossSpawned)
			{
				bossSpawned = true;
				AlienEntity* alien = MakeAlien(AlienType::BOSS, blockEntity->position, 0.f);
				alienEntityList.push_back(alien);
			}

			switch (blockEntity->materialType)
			{
			case MaterialType::GLASS:
				score += 50;
				break;
			case MaterialType::WOOD:
				score += 100;
				break;
			case MaterialType::METAL:
				score += 200;
				break;
			case MaterialType::SHIP:
				score += 1000;
				break;
			}

			//spawn particles here
			//debrisList
			for (int particleCount = 0; particleCount < 10; ++particleCount)
			{

				Particle *p = new Particle();
				p->coords = Physics2Pixels(A->body->GetPosition());
				p->angle = rand() % 360;
				p->direction = deg2vec(rand() % 360);
				p->rotationSpeed = (float)(rand() % 1000) / 100 - 5;
				p->startingSpeed = rand() % 200;
				p->targetSpeed = rand() % 200;
				p->totalTimeToLive = 0.3f;

				p->startingScaleX = (float)(rand() % 100) / 200 + 0.1;
				p->startingScaleY = (float)(rand() % 100) / 200 + 0.1;
				p->targetScaleX = (float)(rand() % 100) / 1000 + 0.05;
				p->targetScaleY = (float)(rand() % 100) / 1000 + 0.05;

				int materialType = (int)blockEntity->materialType;
				if (materialType == 3)
				{
					materialType = 2;
				}
				p->spriteList.push_back(debrisList[(rand() % 3) + materialType * 3]);
				particleList.push_back(p);
			}
		}
	}
}

//Function that applies damage etc when an alien collides with anything
void AlienCollide(Entity* A, float maxImpulseAB)
{
	AlienEntity* alienEntity = (AlienEntity*)A;
	//damage?
	if (maxImpulseAB > 0.2f) //cutoff for no damage
	{
		//apply some damage
		if (alienEntity->Damage((int)(maxImpulseAB * 30.f)))
		{
			//Damage() returned true, need to kill this block
			AddToDeadList(A);

			switch (alienEntity->alienType)
			{
			case AlienType::NORMAL:
				score += 500;
				break;
			case  AlienType::BOSS:
				score += 2000;
				break;
			}

			//spawn particles here
			//debrisList
			for (int particleCount = 0; particleCount < 10; ++particleCount)
			{

				Particle* p = new Particle();
				p->coords = Physics2Pixels(A->body->GetPosition());
				p->angle = rand() % 360;
				p->direction = deg2vec(rand() % 360);
				p->rotationSpeed = (float)(rand() % 1000) / 100 - 5;
				p->startingSpeed = rand() % 200;
				p->targetSpeed = rand() % 200;
				p->totalTimeToLive = 0.3f;

				p->startingScaleX = (float)(rand() % 100) / 200 + 0.1;
				p->startingScaleY = (float)(rand() % 100) / 200 + 0.1;
				p->targetScaleX = (float)(rand() % 100) / 1000 + 0.05;
				p->targetScaleY = (float)(rand() % 100) / 1000 + 0.05;

				p->spriteList.push_back(debrisList[(rand() % 3) + 9]);
				particleList.push_back(p);
			}
		}
	}
}
bool NoAlienShip()
{
	bool noShip = true;
	for (auto& b : blockEntityList)
	{
		if (((BlockEntity*)b)->blockType == BlockType::SHIP)
		{
			noShip = false;
		}
	}
	return noShip;
}

void Init()
{
	LuaGetShotsInfo();
	gameState = START;
	//seed our rng
	std::random_device rd;
	rng.seed(rd());

	blit3D->ShowCursor(false);

	font = blit3D->MakeAngelcodeFontFromBinary32("Media\\Font\\atma.fnt");

	//make a camera
	camera = new Camera2D();

	//set it's valid pan area
	camera->minX = blit3D->screenWidth / 2;
	camera->minY = blit3D->screenHeight / 2;
	camera->maxX = blit3D->screenWidth * 2 - blit3D->screenWidth/2;
	camera->maxY = blit3D->screenHeight / 2 + 400;

	//load the sprites
	cannon.cannonSprite = blit3D->MakeSprite(0, 0, 440, 82, "Media\\Cannon\\Cannon.png");
	cannon.baseSprite = blit3D->MakeSprite(0, 0, 167, 70, "Media\\Cannon\\Base.png");
	cannon.bodySprite = blit3D->MakeSprite(0, 0, 138, 138, "Media\\Cannon\\Body.png");
	cannon.angleSprite = blit3D->MakeSprite(0, 0, 50, 58, "Media\\Cannon\\LevelRotate.png");

	cannon.positionPixels = b2Vec2(100, 150);

	meter.sprite = blit3D->MakeSprite(0, 0, 16, 186, "Media\\Cannon\\StrengthLevel.png");
	meter.positionPixels = cannon.positionPixels;

	// load the screen backgrounds
	gameOverScreen = blit3D->MakeSprite(0, 0, 1920, 1080, "Media\\GameOverScreen.png");
	titleScreen = blit3D->MakeSprite(0, 0, 1920, 1080, "Media\\TitleScreen.png");

	//ground block + cactus
	groundSprite = blit3D->MakeSprite(280, 70, 70, 70, "Media\\spritesheet_tiles.png");
	cactusSprite = blit3D->MakeSprite(22, 14, 182, 196, "Media\\spritesheet_tiles.png");

	// load alien sprites
	normalAlienSprite = blit3D->MakeSprite(0, 0, 81, 65, "Media\\Alien.png");
	bossAlienSprite = blit3D->MakeSprite(0, 0, 137, 109, "Media\\AlienBoss.png");

	//load the block sprites
	// Rectangle Glass
	blockSprites.push_back(blit3D->MakeSprite(0, 240, 79, 120, "Media\\Materials.png"));
	blockSprites.push_back(blit3D->MakeSprite(80, 240, 79, 120, "Media\\Materials.png"));
	blockSprites.push_back(blit3D->MakeSprite(159, 240, 79, 120, "Media\\Materials.png"));
	// Rectangle Wood
	blockSprites.push_back(blit3D->MakeSprite(0, 120, 79, 120, "Media\\Materials.png"));
	blockSprites.push_back(blit3D->MakeSprite(80, 120, 79, 120, "Media\\Materials.png"));
	blockSprites.push_back(blit3D->MakeSprite(159, 120, 79, 120, "Media\\Materials.png"));
	// Rectangle Metal
	blockSprites.push_back(blit3D->MakeSprite(0, 0, 79, 120, "Media\\Materials.png"));
	blockSprites.push_back(blit3D->MakeSprite(80, 0, 79, 120, "Media\\Materials.png"));
	blockSprites.push_back(blit3D->MakeSprite(159, 0, 79, 120, "Media\\Materials.png"));
	// LARGE_RECTANGLE Glass
	blockSprites.push_back(NULL);
	blockSprites.push_back(NULL);
	blockSprites.push_back(NULL);
	// LARGE_RECTANGLE Wood
	blockSprites.push_back(NULL);
	blockSprites.push_back(NULL);
	blockSprites.push_back(NULL);
	// LARGE_RECTANGLE Metal
	blockSprites.push_back(blit3D->MakeSprite(0, 360, 373, 21, "Media\\Materials.png"));
	blockSprites.push_back(blit3D->MakeSprite(0, 381, 373, 21, "Media\\Materials.png"));
	blockSprites.push_back(blit3D->MakeSprite(0, 402, 373, 21, "Media\\Materials.png"));
	// LARGE_TRIANGLE Glass
	blockSprites.push_back(NULL);
	blockSprites.push_back(NULL);
	blockSprites.push_back(NULL);
	// LARGE_TRIANGLE Wood
	blockSprites.push_back(blit3D->MakeSprite(237, 0, 138, 68, "Media\\Materials.png"));
	blockSprites.push_back(blit3D->MakeSprite(237, 70, 138, 68, "Media\\Materials.png"));
	blockSprites.push_back(blit3D->MakeSprite(237, 139, 138, 68, "Media\\Materials.png"));
	// LARGE_TRIANGLE Metal
	blockSprites.push_back(NULL);
	blockSprites.push_back(NULL);
	blockSprites.push_back(NULL);
	// SHIP
	blockSprites.push_back(blit3D->MakeSprite(0, 0, 338, 207, "Media\\Ship.png"));
	blockSprites.push_back(blit3D->MakeSprite(0, 0, 338, 207, "Media\\ShipD1.png"));
	blockSprites.push_back(blit3D->MakeSprite(0, 0, 338, 207, "Media\\ShipD2.png"));

	//load debris sprites
	// Debris Glass
	debrisList.push_back(blit3D->MakeSprite(0,1, 64, 54, "Media\\spritesheet_debris.png"));
	debrisList.push_back(blit3D->MakeSprite(0, 220, 64, 52, "Media\\spritesheet_debris.png"));
	debrisList.push_back(blit3D->MakeSprite(0, 387, 64, 60, "Media\\spritesheet_debris.png"));
	// Debris Wood
	debrisList.push_back(blit3D->MakeSprite(0, 57, 64, 54, "Media\\spritesheet_debris.png"));
	debrisList.push_back(blit3D->MakeSprite(0, 168, 64, 52, "Media\\spritesheet_debris.png"));
	debrisList.push_back(blit3D->MakeSprite(0, 447, 64, 60, "Media\\spritesheet_debris.png"));
	// Debris Metal
	debrisList.push_back(blit3D->MakeSprite(0, 113, 64, 54, "Media\\spritesheet_debris.png"));
	debrisList.push_back(blit3D->MakeSprite(0, 272, 64, 52, "Media\\spritesheet_debris.png"));
	debrisList.push_back(blit3D->MakeSprite(0, 325, 64, 60, "Media\\spritesheet_debris.png"));
	// Debris blood
	debrisList.push_back(blit3D->MakeSprite(0, 507, 64, 54, "Media\\spritesheet_debris.png"));
	debrisList.push_back(blit3D->MakeSprite(0, 562, 64, 52, "Media\\spritesheet_debris.png"));
	debrisList.push_back(blit3D->MakeSprite(0, 614, 64, 60, "Media\\spritesheet_debris.png"));
	// Debris Shots
	debrisList.push_back(blit3D->MakeSprite(0, 675, 64, 54, "Media\\spritesheet_debris.png"));
	debrisList.push_back(blit3D->MakeSprite(0, 729, 64, 52, "Media\\spritesheet_debris.png"));
	debrisList.push_back(blit3D->MakeSprite(0, 781, 64, 60, "Media\\spritesheet_debris.png"));

	//load cloud sprites
	cloudList.push_back(blit3D->MakeSprite(0, 0, 174, 157, "Media\\cumulus-big1.png"));
	cloudList.push_back(blit3D->MakeSprite(0, 0, 190, 118, "Media\\cumulus-big2.png"));
	cloudList.push_back(blit3D->MakeSprite(0, 0, 238, 128, "Media\\cumulus-big3.png"));
	cloudList.push_back(blit3D->MakeSprite(0, 0, 512, 211, "Media\\cumulus-huge.png"));
	cloudList.push_back(blit3D->MakeSprite(0, 0, 54, 43, "Media\\cumulus-small1.png"));
	cloudList.push_back(blit3D->MakeSprite(0, 0, 76, 51, "Media\\cumulus-small2.png"));
	cloudList.push_back(blit3D->MakeSprite(0, 0, 64, 41, "Media\\cumulus-small3.png"));

	cloudList.push_back(blit3D->MakeSprite(12, 12, 101, 28, "Media\\cumulus-tiny.png"));
	cloudList.push_back(blit3D->MakeSprite(12, 55, 38, 16, "Media\\cumulus-tiny.png"));
	cloudList.push_back(blit3D->MakeSprite(9, 85, 107, 22, "Media\\cumulus-tiny.png"));
	cloudList.push_back(blit3D->MakeSprite(10, 122, 53, 17, "Media\\cumulus-tiny.png"));
	cloudList.push_back(blit3D->MakeSprite(1, 148, 118, 28, "Media\\cumulus-tiny.png"));
	cloudList.push_back(blit3D->MakeSprite(4, 188, 68, 23, "Media\\cumulus-tiny.png"));
	cloudList.push_back(blit3D->MakeSprite(10, 225, 52, 16, "Media\\cumulus-tiny.png"));

	// load all the explosion sprites
	for (int i = 0; i < 10; i++)
	{
		explosionSpriteList.push_back(blit3D->MakeSprite(0 + (i * 1066), 0, 1066, 1091, "Media\\Explosion.png"));
	}
	//add the graphical elements to the level
	GraphicElement ge;
	//add some cactii
	ge.sprite = cactusSprite;
	ge.pos = glm::vec2(500, 168);
	elementList.push_back(ge);
	ge.pos = glm::vec2(1000, 168);
	elementList.push_back(ge);
	ge.pos = glm::vec2(1800, 168);
	elementList.push_back(ge);
	ge.pos = glm::vec2(2150, 168);
	elementList.push_back(ge);
	ge.pos = glm::vec2(3000, 168);
	elementList.push_back(ge);

	//add some clouds
	std::uniform_real_distribution<float> distCloudsx(0, 4000);
	std::uniform_real_distribution<float> distCloudsy(700, 2000);
	std::uniform_int_distribution<int> distCloudsIndex(0, cloudList.size() - 1);

	for (int numClouds = 0; numClouds < 300; ++numClouds)
	{
		ge.sprite = cloudList[distCloudsIndex(rng)];
		ge.pos.x = distCloudsx(rng);
		ge.pos.y = distCloudsy(rng);
		elementList.push_back(ge);
	}

	// Define the gravity vector.
	b2Vec2 gravity(0.f, -9.8f);

	// Construct a world object, which will hold and simulate the rigid bodies.
	world = new b2World(gravity);
	//world->SetGravity(gravity); <-can call this to change gravity at any time
	world->SetAllowSleeping(true); //set true to allow the physics engine to 'sleep" objects that stop moving

								   //_________GROUND OBJECT_____________
								   //make an entity for the ground
	GroundEntity *groundEntity = new GroundEntity();
	//A bodyDef for the ground
	b2BodyDef groundBodyDef;
	// Define the ground body.
	groundBodyDef.position.Set(0, 0);

	//add the userdata
	groundBodyDef.userData.pointer = reinterpret_cast<uintptr_t>(groundEntity);

	// Call the body factory which allocates memory for the ground body
	// from a pool and creates the ground box shape (also from a pool).
	// The body is also added to the world.
	groundEntity->body = world->CreateBody(&groundBodyDef);

	//an EdgeShape object, for the ground
	b2EdgeShape groundBox;

	// Define the ground as 1 edge shape at the bottom of the screen.
	b2FixtureDef boxShapeDef;

	boxShapeDef.shape = &groundBox;

	//collison masking
	boxShapeDef.filter.categoryBits = CMASK_GROUND;  //this is the ground
	boxShapeDef.filter.maskBits = CMASK_SHOT | CMASK_BLOCK | CMASK_ALIEN;		//it collides wth balls and powerups

																				//bottom
	groundBox.SetTwoSided(b2Vec2(0, 70 / PTM_RATIO), b2Vec2(blit3D->screenWidth * 2 / PTM_RATIO, 70 / PTM_RATIO));
	//Create the fixture
	groundEntity->body->CreateFixture(&boxShapeDef);
	
	//add to the entity list
	entityList.push_back(groundEntity);

	//now make the other 3 edges of the screen on a seperate entity/body
	EdgeEntity * edgeEntity = new EdgeEntity();

	groundBodyDef.userData.pointer = reinterpret_cast<uintptr_t>(edgeEntity);
	edgeEntity->body = world->CreateBody(&groundBodyDef);

	boxShapeDef.filter.categoryBits = CMASK_EDGES;  //this is the ground
	boxShapeDef.filter.maskBits = CMASK_SHOT | CMASK_BLOCK | CMASK_ALIEN;		//it collides wth shot, blocks and aliens

																				//left
	groundBox.SetTwoSided(b2Vec2(0, blit3D->screenHeight * 2 / PTM_RATIO), b2Vec2(0, 70 / PTM_RATIO));
	edgeEntity->body->CreateFixture(&boxShapeDef);

	//top
	groundBox.SetTwoSided(b2Vec2(0, blit3D->screenHeight * 2 / PTM_RATIO),
		b2Vec2(blit3D->screenWidth * 2 / PTM_RATIO, blit3D->screenHeight * 2 / PTM_RATIO));
	edgeEntity->body->CreateFixture(&boxShapeDef);

	//right
	groundBox.SetTwoSided(b2Vec2(blit3D->screenWidth * 2 / PTM_RATIO, 70 / PTM_RATIO),
		b2Vec2(blit3D->screenWidth * 2 / PTM_RATIO, blit3D->screenHeight * 2 / PTM_RATIO));
	edgeEntity->body->CreateFixture(&boxShapeDef);

	entityList.push_back(edgeEntity);

	// Create contact listener and use it to collect info about collisions
	contactListener = new MyContactListener();
	world->SetContactListener(contactListener);

	
}

void DeInit(void)
{
	if (camera != NULL) delete camera;
	
	//delete all particles
	for (auto &p : particleList) delete p;
	particleList.clear();

	//delete all the entities
	for (auto &e : entityList) delete e;
	entityList.clear();

	for (auto &s : shotEntityList) delete s;
	shotEntityList.clear();

	for (auto &b : blockEntityList) delete b;
	blockEntityList.clear();

	for (auto& a : alienEntityList) delete a;
	alienEntityList.clear();

	for (auto& a : explosionList) delete a;
	explosionList.clear();

	for (auto& a : shotsInfo) delete a;
	shotsInfo.clear();

	//delete the contact listener
	delete contactListener;

	//Free all physics game data we allocated
	delete world;
	//any sprites still allocated are freed automatcally by the Blit3D object when we destroy it
}

void Update(double seconds)
{
	
	
	switch (gameState)
	{
	case SETTLING:
	{
		camera->Update((float)seconds);
		elapsedTime += seconds;
		while (elapsedTime >= timeStep)
		{
			//update the physics world
			world->Step(timeStep, velocityIterations, positionIterations);

			// Clear applied body forces. 
			world->ClearForces();

			settleTime += timeStep;
			elapsedTime -= timeStep;
		}

		if (settleTime > 2) gameState = PLAYING;
	}
	break;
	case PLAYING:
	{
		elapsedTime += seconds;

		if (justChangedBall)
		{
			characterIsShowing = true;
			characterXposition = -250;
			justChangedBall = false;
			characterGoingForward = true;
		}

		if (characterIsShowing)
		{
			if (characterGoingForward)
			{
				characterXposition += 10;
				if (characterXposition >= 300)
				{
					characterGoingForward = false;
				}
			}
			else
			{
				characterXposition -= 10;
				if (characterXposition <= -300)
				{
					characterIsShowing = false;
				}
			}
			
		}

		if (fireShotNow)
		{

			fireShotNow = false;
			followingShot = true;
			mustDestroyCurrentBall = false;
			if (remainingShots[currentShotType] > 0)
			{
				remainingShots[currentShotType]--;
				score -= shotsInfo[currentShotType]->score;

				//fire a shot!
				ShotEntity *shot = MakeShot(currentShotType);
				b2Vec2 pos = Pixels2Physics(cannon.positionPixels);

				b2Vec2 dirCannon = Pixels2Physics(deg2vec(cannon.angle, 220));

				shot->body->SetTransform(pos + dirCannon, 0);
				shot->lastPosition = pos;
				shot->stopped = false;
				b2Vec2 dir = deg2vec(cannon.angle, meter.scale * 3 + 1);

				shot->body->ApplyLinearImpulse(dir, shot->body->GetPosition(), true);
				shotEntityList.push_back(shot);
				currentShot = shot;
				shooting = true;

			}
		}

		

		//don't apply physics unless at least a timestep worth of time has passed
		while (elapsedTime >= timeStep)
		{
			//update the physics world
			world->Step(timeStep, velocityIterations, positionIterations);

			// Clear applied body forces. 
			world->ClearForces();

			elapsedTime -= timeStep;

			//update game logic/animation
			for (auto &e : entityList) e->Update(timeStep);
			for (auto& b : shotEntityList)
			{
				
				if (((ShotEntity*)b)->stopped && ((ShotEntity*)b) != currentShot)
				{
					AddToDeadList(b);
				}
				b->Update(timeStep);
			}
			for (auto &b : blockEntityList) b->Update(timeStep);

			for (auto& b : explosionList)
			{
				if (b->GetFrame() >= 9) {
					AddToDeadList(b);
				}
				// Animate the explosion
				if (b->GetFrame() < 10)
				{
					b->Update(timeStep);
				}
			}

			if (mustDestroyCurrentBall || shooting && currentShot->stopped)
			{
				mustDestroyCurrentBall = false;
				AddToDeadList(currentShot);
				Particle* p = new Particle();
				p->coords = Physics2Pixels(currentShot->body->GetPosition());
				p->angle = rand() % 360;
				p->direction = deg2vec(rand() % 360);
				p->rotationSpeed = (float)(rand() % 1000) / 100 - 5;
				p->startingSpeed = rand() % 200;
				p->targetSpeed = rand() % 200;
				p->totalTimeToLive = 0.3f;

				p->startingScaleX = (float)(rand() % 100) / 200 + 0.1;
				p->startingScaleY = (float)(rand() % 100) / 200 + 0.1;
				p->targetScaleX = (float)(rand() % 100) / 1000 + 0.05;
				p->targetScaleY = (float)(rand() % 100) / 1000 + 0.05;

				p->spriteList.push_back(debrisList[(rand() % 3) + 12]);
				particleList.push_back(p);
				shooting = false;

				camera->PanTorwards(blit3D->screenWidth / 2, blit3D->screenHeight / 2);
				followingShot = false;
			}

			//update shot meter
			meter.Update(timeStep);

			//update cannon
			cannon.Update(timeStep);

			//update camera
			if (followingShot)
			{
				//make sure there is a shot to follow
				int size = shotEntityList.size();
				if (size > 0)
				{
					//last shot on list is the current active shot,
					//so follow it
					b2Vec2 pos = shotEntityList[size - 1]->body->GetPosition();
					pos = Physics2Pixels(pos);
					camera->PanTo(pos.x, pos.y);
				}				
			}
			camera->Update(timeStep);

			//update the particle list and remove dead particles
			for (int i = particleList.size() - 1; i >= 0; --i)
			{
				if (particleList[i]->Update(timeStep))
				{
					//time to die!
					delete particleList[i];
					particleList.erase(particleList.begin() + i);
				}
			}

			//loop over contacts
			for (int pos = 0; pos < contactListener->contacts.size(); ++pos)
			{
				MyContact contact = contactListener->contacts[pos];

				//fetch the entities from the body userdata
				Entity *A = (Entity *)contact.fixtureA->GetBody()->GetUserData().pointer;
				Entity *B = (Entity *)contact.fixtureB->GetBody()->GetUserData().pointer;

				if (A != NULL && B != NULL) //if there is an entity for these objects...
				{
					if (A->typeID == EntityTypes::ENTITYBLOCK)
					{
						BlockCollide(A, contact.maxImpulseAB);
					}
					else if (A->typeID == EntityTypes::ENTITYALIEN)
					{
						AlienCollide(A, contact.maxImpulseAB);
					}

					if (B->typeID == EntityTypes::ENTITYBLOCK)
					{
						BlockCollide(B, contact.maxImpulseAB);
					}
					else if (B->typeID == EntityTypes::ENTITYALIEN)
					{
						AlienCollide(B, contact.maxImpulseAB);
					}
					
				}
			}

			//clean up dead entities
			for (auto &e : deadEntityList)
			{
				//remove body from the physics world and free the body data
				world->DestroyBody(e->body);
				//remove the entity from the appropriate entityList
				if (e->typeID == ENTITYSHOT)
				{
					for (int i = 0; i < shotEntityList.size(); ++i)
					{
						if (e == shotEntityList[i])
						{
							delete shotEntityList[i];
							shotEntityList.erase(shotEntityList.begin() + i);
							break;
						}
					}
				}
				else if (e->typeID == ENTITYBLOCK)
				{
					for (int i = 0; i < blockEntityList.size(); ++i)
					{
						if (e == blockEntityList[i])
						{
							delete blockEntityList[i];
							blockEntityList.erase(blockEntityList.begin() + i);
							break;
						}
					}
				}
				else if (e->typeID == ENTITYALIEN)
				{
					for (int i = 0; i < alienEntityList.size(); ++i)
					{
						if (e == alienEntityList[i])
						{
							delete alienEntityList[i];
							alienEntityList.erase(alienEntityList.begin() + i);
							break;
						}
					}
				}
				else if (e->typeID == EXPLOSION)
				{
					for (int i = 0; i < explosionList.size(); ++i)
					{
						if (e == explosionList[i])
						{
							delete explosionList[i];
							explosionList.erase(explosionList.begin() + i);
							break;
						}
					}
				}
				else
				{
					for (int i = 0; i < entityList.size(); ++i)
					{
						if (e == entityList[i])
						{
							delete entityList[i];
							entityList.erase(entityList.begin() + i);
							break;
						}
					}
				}
			}

			deadEntityList.clear();
			if ((remainingShots[0] <= 0 && remainingShots[1] <= 0 && remainingShots[1] <= 0) || (alienEntityList.empty() && NoAlienShip()))
			{
				gameState = GAMEOVER;
			}
		}
	}
	break;

	case START:
		camera->StopShaking();
		break;
	case GAMEOVER:
		camera->StopShaking();
		break;
	default:
		//do nada here
		break;
	}//end switch(gameState)
}

void Draw(void)
{
	glClearColor(0.25f, 0.25f, 0.5f, 0.0f);	//clear colour: r,g,b,a 	
											// wipe the drawing surface clear
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
	std::string tempString;
	int fontSize;
	//draw stuff here
	switch (gameState)
	{
	case START:
		camera->UnDraw();
		titleScreen->Blit(blit3D->screenWidth / 2, blit3D->screenHeight / 2);
		tempString = "Press ENTER";
		fontSize = font->WidthText(tempString);
		font->BlitText(((blit3D->screenWidth - fontSize) / 2), 450, tempString);
		tempString = "by Dario Urdapilleta";
		fontSize = font->WidthText(tempString);
		font->BlitText(((blit3D->screenWidth - fontSize) / 2), 250, tempString);
		break;
	case GAMEOVER:
		camera->UnDraw();
		gameOverScreen->Blit(blit3D->screenWidth / 2, blit3D->screenHeight / 2);
		tempString = "Total Score: " + std::to_string(score);
		fontSize = font->WidthText(tempString);
		font->BlitText(((blit3D->screenWidth - fontSize) / 2), 450, tempString);
		break;

	case SETTLING:
	case PLAYING:
		//MUST draw camera first!
		camera->Draw();

		//Draw the level background
		for (int i = 0; i < ((1920 * 2) / 70) + 2; ++i)
		{
			groundSprite->Blit(i * 70.f - 35.f, 35.f);
		}

		//throw in a few graphical things, clouds and cactii
		for (auto& e : elementList) e.Draw();

		//loop over all entities and draw them
		for (auto &e : entityList) e->Draw();
		for (auto &b : blockEntityList) b->Draw();
		for (auto& b : alienEntityList) b->Draw();
		for (auto &b : shotEntityList) b->Draw();
		for (auto &p : particleList) p->Draw();
		for (auto& p : explosionList)
		{
			if (p->GetFrame() < 10 && p->GetFrame() >= 0)
			{
				p->Draw();
			}
		}
		cannon.Draw();
		meter.Draw();
		tempString = "Score: " + std::to_string(score);
		camera->UnDraw();
		font->BlitText((blit3D->screenWidth / 2 - 180), 1050, tempString);

		for (int i = 0; i < shotsInfo.size();i++)
		{
			if (shotsInfo[i]->keyString != "")
			{
				if (shotsInfo[i]->sprite != NULL)
				{
					shotsInfo[i]->sprite->angle = 0;
					shotsInfo[i]->sprite->Blit(50.f + (i * 180.f), 1015.f, 1.5, 1.5);
				}
				if (shotsInfo[i]->fixedSprite != NULL)
				{
					shotsInfo[i]->fixedSprite->Blit(50.f + (i * 180.f), 1015.f, 1.5, 1.5);
				}
				font->BlitText(100 + (i * 180.f), 1050, std::to_string(remainingShots[i]));
				font->BlitText(40 + (i * 180.f), 1000, shotsInfo[i]->keyString);
			}
			
		}

		if (characterIsShowing)
		{
			if (shotsInfo[currentShotType] != NULL && shotsInfo[currentShotType]->characterSprite != NULL)
			{
				shotsInfo[currentShotType]->characterSprite->Blit(characterXposition, 220);
			}
		}

		break;
	}
}

//the key codes/actions/mods for DoInput are from GLFW: check its documentation for their values
void DoInput(int key, int scancode, int action, int mods)
{
	if (key == GLFW_KEY_ESCAPE && action == GLFW_PRESS)
		blit3D->Quit(); //start the shutdown sequence
	switch (gameState)
	{
	case START:
		if (key == GLFW_KEY_ENTER && action == GLFW_PRESS)
		{
			camera->PanTo(blit3D->screenWidth / 2, blit3D->screenHeight / 2);
			gameState = SETTLING;
			score = 12000;
			remainingShots.clear();
			for (int i = 0; i < shotsInfo.size(); i++)
			{
				if (shotsInfo[i]->maxShots > 0)
				{
					remainingShots.push_back(shotsInfo[i]->maxShots);
				}
			}
			justChangedBall = false;
			shooting = false;
			mustDestroyCurrentBall = false;
			bossSpawned = false;
			characterXposition = -10;
			
			for (auto& s : shotEntityList)
			{
				world->DestroyBody(s->body);
				delete s;
			}
			shotEntityList.clear();

			for (auto& b : blockEntityList)
			{
				world->DestroyBody(b->body);
				delete b;
			}
			blockEntityList.clear();

			for (auto& a : alienEntityList)
			{
				world->DestroyBody(a->body);
				delete a;
			}
			alienEntityList.clear();

			for (auto& a : explosionList) delete a;
			explosionList.clear();

			BlockEntity* block = MakeBlock(BlockType::RECTANGLE, MaterialType::METAL, b2Vec2(2200.f, 120), 0.f);
			blockEntityList.push_back(block);
			block = MakeBlock(BlockType::RECTANGLE, MaterialType::GLASS, b2Vec2(2495.f, 120.f), 0.f);
			blockEntityList.push_back(block);
			block = MakeBlock(BlockType::LARGE_RECTANGLE, MaterialType::METAL, b2Vec2(2347.f, 200.f), 0.f);
			blockEntityList.push_back(block);
			block = MakeBlock(BlockType::RECTANGLE, MaterialType::METAL, b2Vec2(2200.f, 269.f), 0.f);
			blockEntityList.push_back(block);
			block = MakeBlock(BlockType::RECTANGLE, MaterialType::WOOD, b2Vec2(2495.f, 269.f), 0.f);
			blockEntityList.push_back(block);
			block = MakeBlock(BlockType::LARGE_RECTANGLE, MaterialType::METAL, b2Vec2(2347.f, 339.f), 0.f);
			blockEntityList.push_back(block);
			block = MakeBlock(BlockType::LARGE_TRIANGLE, MaterialType::WOOD, b2Vec2(2250.f, 375.f), 0.f);
			blockEntityList.push_back(block);
			block = MakeBlock(BlockType::LARGE_TRIANGLE, MaterialType::WOOD, b2Vec2(2440.f, 375.f), 0.f);
			blockEntityList.push_back(block);
			block = MakeBlock(BlockType::SHIP, MaterialType::SHIP, b2Vec2(2347.f, 420.f), 0.f);
			blockEntityList.push_back(block);

			block = MakeBlock(BlockType::RECTANGLE, MaterialType::METAL, b2Vec2(2800.f, 120.f), 0.f);
			blockEntityList.push_back(block);
			block = MakeBlock(BlockType::RECTANGLE, MaterialType::WOOD, b2Vec2(2800.f, 240.f), 0.f);
			blockEntityList.push_back(block);
			block = MakeBlock(BlockType::RECTANGLE, MaterialType::GLASS, b2Vec2(2800.f, 360.f), 0.f);
			blockEntityList.push_back(block);
			block = MakeBlock(BlockType::RECTANGLE, MaterialType::METAL, b2Vec2(2800.f, 480.f), 0.f);
			blockEntityList.push_back(block);

			AlienEntity* alien = MakeAlien(AlienType::NORMAL, b2Vec2(2300.f, 200), 0.f);
			alienEntityList.push_back(alien);
			alien = MakeAlien(AlienType::NORMAL, b2Vec2(2300.f, 300), 0.f);
			alienEntityList.push_back(alien);
			alien = MakeAlien(AlienType::NORMAL, b2Vec2(2400.f, 200), 0.f);
			alienEntityList.push_back(alien);
			alien = MakeAlien(AlienType::NORMAL, b2Vec2(2400.f, 300), 0.f);
			alienEntityList.push_back(alien);
		}
		break;
	case GAMEOVER:
		if (key == GLFW_KEY_ENTER && action == GLFW_PRESS)
		{
			gameState = START;
		}
		break;

	case SETTLING:
		break;
	case PLAYING:
		if (key == GLFW_KEY_D && (action == GLFW_PRESS || action == GLFW_REPEAT))
			cannon.rotateDir = -1.f;

		if (key == GLFW_KEY_A && (action == GLFW_PRESS || action == GLFW_REPEAT))
			cannon.rotateDir = 1.f;

		if ((key == GLFW_KEY_A || key == GLFW_KEY_D) && action == GLFW_RELEASE)
			cannon.rotateDir = 0;


		if (key == GLFW_KEY_SPACE && action == GLFW_PRESS)
		{
			meter.shooting = true;
		}

		if (key == GLFW_KEY_SPACE && action == GLFW_RELEASE)
		{
			meter.shooting = false;
			fireShotNow = true;
		}
	
		else
		{
			if (key == GLFW_KEY_ENTER && action == GLFW_PRESS)
			{
				if (currentShot != NULL)
				{
					ActivateAbility();
				}
			}
		}
		

		if (key == GLFW_KEY_RIGHT && (action == GLFW_PRESS || action == GLFW_REPEAT))
		{
			camera->Pan(1, 0);
			followingShot = false;
		}

		if (key == GLFW_KEY_LEFT && (action == GLFW_PRESS || action == GLFW_REPEAT))
		{
			camera->Pan(-1, 0);
			followingShot = false;
		}

		if ((key == GLFW_KEY_LEFT || key == GLFW_KEY_RIGHT) && action == GLFW_RELEASE)
			camera->Pan(0, 0);

		if (key == GLFW_KEY_UP && (action == GLFW_PRESS || action == GLFW_REPEAT))
		{
			camera->Pan(0, 1);
			followingShot = false;
		}

		if (key == GLFW_KEY_DOWN && (action == GLFW_PRESS || action == GLFW_REPEAT))
		{
			camera->Pan(0, -1);
			followingShot = false;
		}

		if ((key == GLFW_KEY_DOWN || key == GLFW_KEY_UP) && action == GLFW_RELEASE)
			camera->Pan(0, 0);

		for (int i = 0; i < shotsInfo.size(); i++)
		{
			if (shotsInfo[i]->key > 0)
			{
				if ((key == shotsInfo[i]->key) && action == GLFW_RELEASE)
				{
					currentShotType = i;
					justChangedBall = true;
				}
			}
		}
		break;
	}
	
}
/*
* REturns the shots information from the lua script
*/
int GetShotsInfo(lua_State* L) {
	return 1;
}

/*
Starts Lua
*/
void InitLua()
{
	//initialize Lua
	L = lua_open();

	//load Lua base libraries
	luaL_openlibs(L);

	// Get the shots info
	lua_register(L, "GetShotsInfo", GetShotsInfo);

	// Register the random method
	lua_register(L, "ApplyForceWithinRange", ApplyForceWithinRange);

	lua_register(L, "ShowExplosion", ShowExplosion);

	lua_register(L, "DestroyCurrentShot", DestroyCurrentShot);

	lua_register(L, "MakeNewBall", MakeNewBall);

	// run the script
	luaL_dofile(L, "shots.lua");


}



int main(int argc, char *argv[])
{
	//memory leak detection
	_CrtSetDbgFlag(_CRTDBG_ALLOC_MEM_DF | _CRTDBG_LEAK_CHECK_DF);

	blit3D = new Blit3D(Blit3DWindowModel::BORDERLESSFULLSCREEN_1080P, 720, 480);

	InitLua();

	//set our callback funcs
	blit3D->SetInit(Init);
	blit3D->SetDeInit(DeInit);
	blit3D->SetUpdate(Update);
	blit3D->SetDraw(Draw);
	blit3D->SetDoInput(DoInput);


	//Run() blocks until the window is closed
	blit3D->Run(Blit3DThreadModel::SINGLETHREADED);


	if (blit3D)
	{
		//cleanup Lua
		lua_close(L);
		delete blit3D;
	}
}