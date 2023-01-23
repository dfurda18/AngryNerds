#include "Explosion.h"
extern b2World* world;
/**
* Explosion object constructor method.
* @param glm::vec2 The explosion's position
* @param std::vector<Sprite*> The collection of sprites that are the graphical representation of the explosion.
* @param float The explosion's radius size.
* @return An instance of the Explosion class.
*/
Explosion::Explosion(b2Vec2 newPosition, std::vector<Sprite*> newSpriteList, float newRadius)
{
	this->frameNumber = 0;
	this->typeID = EXPLOSION;
	this->position = newPosition;
	this->spriteList = newSpriteList;
	this->radius = newRadius * 1.2f;
	this->radiusOrtho = this->radius / 220 / sqrt(2);
	b2BodyDef myBodyDef;
	myBodyDef.type = b2_staticBody;
	myBodyDef.position.Set(newPosition.x, newPosition.y);
	this->body = world->CreateBody(&myBodyDef);
}
/**
* This method returns the explosion's current frame
* @return int the explosion's current frame
*/
int Explosion::GetFrame()
{
	return this->frameNumber;
}
/**
* Updates the explosion's values after certain time period.
* @param float The time period that has occurred since last uptade.
* @return Returns true.
*/
void Explosion::Update(float deltaTime)
{
	// Progress the animation if certain amount of time has passed.
	this->animationTimer += deltaTime;
	if (this->animationTimer > 0.1f) {
		this->frameNumber++;
		this->animationTimer -= 0.1f;
	}

	if (this->frameNumber >= 10) {
		this->frameNumber = 10;
	}
}
/**
* Draws the exlosion on the screen.
*/
void Explosion::Draw()
{
	
	position = body->GetPosition();
	position = Physics2Pixels(position);

 	this->spriteList[this->frameNumber]->Blit(position.x, position.y, .6, .6);

}