#include "ShotsInfo.h"
#include "Blit3D.h"

extern Blit3D* blit3D;

ShotsInfo::ShotsInfo(std::string keyString, std::string key, std::string maxShots, std::string density, std::string sprite, std::string spriteSize, std::string fixedSprite, std::string spriteFixedSize, std::string characterSprite, std::string characterSizeX, std::string characterSizeY, std::string score, std::string actionId)
{
	this->actionId = actionId == "NONE" ? 0 : stoi(actionId);
	this->keyString = keyString == "NONE" ? "" : keyString;
	this->key = key == "NONE" ? -1 : stoi(key);
	this->maxShots = maxShots == "NONE" ? 0 : stoi(maxShots);
	this->density = density == "NONE" ? 0.0f : stof(density);
	this->score = score == "NONE" ? 0 : stoi(score);
	this->spriteSize = spriteFixedSize == "NONE" ? 0 : stoi(spriteFixedSize);
	this->sprite = sprite == "NONE" ? NULL : blit3D->MakeSprite(0, 0, spriteSize == "NONE" ? 0 : stoi(spriteSize), spriteSize == "NONE" ? 0 : stoi(spriteSize), sprite);
	this->fixedSprite = fixedSprite == "NONE" ? NULL : blit3D->MakeSprite(0, 0, spriteFixedSize == "NONE" ? 0 : stoi(spriteFixedSize), spriteFixedSize == "NONE" ? 0 : stoi(spriteFixedSize), fixedSprite);
	this->characterSprite = characterSprite == "NONE" ? NULL : blit3D->MakeSprite(0, 0, characterSizeX == "NONE" ? 0 : stoi(characterSizeX), characterSizeY == "NONE" ? 0 : stoi(characterSizeY), characterSprite);
}