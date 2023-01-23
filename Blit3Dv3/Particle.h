#pragma once

/*
Generic Particle class. Although I am using b2Vec2s form Box2D here,
all values such as speed and coords are pixel-relative.
*/

#include "Blit3D.h"
#include "Physics.h"

//tweening types
enum class EaseMode {LINEAR, EASEIN, EASEOUT};

class Particle
{
private:
	float percentDone; //how far are we towards being completed? 0.0-1.0
	float currentSpeed; //how fast we are moving
	float currentScaleX; //current scaling
	float currentScaleY;
	float timeAliveSoFar; //our current age in seconds
	int currentFrameNum; //current frame index to display
public:
	std::vector<Sprite *> spriteList; //list of sprites for animation
	float alpha; //transparency 0.0-1.0
	float totalTimeToLive; //seconds to be alive
	float angle; //current angle of sprite
	b2Vec2 direction; //normal vector for movement
	float startingSpeed; //speed we started at
	float targetSpeed; //speed we want to end at
	float rotationSpeed; //degrees per second
	b2Vec2 coords; //where we are
	bool useAlpha; //true if we wish to fade the particle out
	//float timePerFrame; //default framerate, unused atm
	EaseMode easeMode; //tweening type

	//scaling vars
	float startingScaleX;
	float startingScaleY;
	float targetScaleX;
	float targetScaleY;

	Particle();
	void Draw();
	bool Update(float seconds); //returns true if this particle should be removed
};