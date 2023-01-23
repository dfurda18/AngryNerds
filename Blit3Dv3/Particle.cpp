#include "Particle.h"

Particle::Particle()
{
	alpha = 1.f;
	totalTimeToLive = 0.3f;
	timeAliveSoFar = 0.f;
	angle = 90.f;
	direction = b2Vec2(0.f, 1.f);
	currentSpeed = startingSpeed = 100.f;
	targetSpeed = startingSpeed / 2;
	rotationSpeed = 0.f;
	percentDone = 0;;
	coords = b2Vec2(0,0);
	useAlpha = true;
	currentFrameNum = 0;
	startingScaleX = 1;
	startingScaleY = 1;
	targetScaleX = 1;
	targetScaleY = 1;
	currentScaleX = currentScaleY = 1;
	//timePerFrame = 1.f / 30.f; //default framerate, unused atm
	easeMode = EaseMode::EASEOUT;
}

void Particle::Draw()
{
	spriteList[currentFrameNum]->angle = angle;
	//TODO: add scaling ease types and calculations
	spriteList[currentFrameNum]->Blit(coords.x, coords.y, currentScaleX, currentScaleY, alpha);
}

bool Particle::Update(float seconds)
{
	timeAliveSoFar += seconds;
	if(timeAliveSoFar >= totalTimeToLive) return true; //early exit for dead particles

	//still alive, so update our position
	//assert(timeAliveSoFar > 0.f);
	percentDone = timeAliveSoFar / totalTimeToLive  ;

	//apply tween easing to speed
	switch(easeMode)
	{
	case EaseMode::EASEIN:
		currentSpeed = startingSpeed + (targetSpeed - startingSpeed) * (percentDone * percentDone * percentDone);
		currentScaleX = startingScaleX + (targetScaleX - startingScaleX) * (percentDone * percentDone * percentDone);
		currentScaleY = startingScaleY + (targetScaleY - startingScaleY) * (percentDone * percentDone * percentDone);
		break;

	case EaseMode::EASEOUT:
		currentSpeed = startingSpeed + (targetSpeed - startingSpeed) * (1.f - percentDone * percentDone * percentDone);
		currentScaleX = startingScaleX + (targetScaleX - startingScaleX) * (1.f - percentDone * percentDone * percentDone);
		currentScaleY = startingScaleY + (targetScaleY - startingScaleY) * (1.f - percentDone * percentDone * percentDone);
		break;

	default:
		currentSpeed = startingSpeed + (targetSpeed - startingSpeed) * percentDone;
		currentScaleX = startingScaleX + (targetScaleX - startingScaleX) * percentDone;
		currentScaleY = startingScaleY + (targetScaleY - startingScaleY) * percentDone;
		break;
	}

	//update position
	coords += currentSpeed * seconds * direction;

	//update rotation
	angle += rotationSpeed * seconds;

	if(useAlpha)
	{
		alpha = 1.f - percentDone * percentDone * percentDone * percentDone * percentDone; //strong ease in
	}

	//update animation frames - skip using timePerFrame for now
	//TODO: add timePerFrame calculations if using that
	currentFrameNum = spriteList.size() * percentDone;
	if (currentFrameNum > spriteList.size() - 1) currentFrameNum = spriteList.size() - 1;

	return false;
}