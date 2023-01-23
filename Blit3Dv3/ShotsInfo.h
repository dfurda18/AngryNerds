#pragma once

#include "Blit3D.h"
#include <string>

class ShotsInfo
{
public:
	std::string keyString;
	int key;
	int maxShots;
	int score;
	int actionId;
	float density;
	float spriteSize;
	Sprite* sprite;
	Sprite* fixedSprite;
	Sprite* characterSprite;

	/**
	* Creates a shot info element
	* @param std::string The Shot's key name
	* @param std::string The Shot's key
	* @param std::string The Shot's max shots
	* @param std::string The Shot's density
	* @param std::string The Shot's rotating sprite
	* @param std::string The rotating sprite size
	* @param std::string The Shot's fixed sprite
	* @param std::string The fixed sprite size
	* @param std::string The Shot's character's sprite
	* @param std::string The Charaacter sprite size x
	* @param std::string The Charaacter sprite size y
	* @param std::string The shot's score
	* @param std::string The action id
	* @return An instance of the shot info element.
	*/
	ShotsInfo(std::string, std::string, std::string, std::string, std::string, std::string, std::string, std::string, std::string, std::string, std::string, std::string, std::string);
};

