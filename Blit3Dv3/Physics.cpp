#include "Physics.h"

b2Vec2 rad2vec(float r, float m)
{
	b2Vec2 retval(cos(r), sin(r));
	retval *= m;
	return retval;
};

b2Vec2 deg2vec(float r, float m)
{
	return rad2vec(r*DEG2RAD, m);
};

float vec2rad(b2Vec2 v)
{
	return atan2(v.y, v.x);
};

float vec2deg(b2Vec2 v)
{
	return vec2rad(v)*RAD2DEG;
};

float rad2deg(float r)
{
	return r * RAD2DEG;
};

float deg2rad(float d)
{
	return d * DEG2RAD;
}

b2Vec2 Physics2Pixels(b2Vec2 input)
{
	//our physics world is 1/PTM_RATIOth the pixel world
	input *= PTM_RATIO;
	return input;
}

b2Vec2 Pixels2Physics(b2Vec2 input)
{
	//our physics world is 1/PTM_RATIOth the pixel world
	input.x = input.x / PTM_RATIO;
	input.y = input.y / PTM_RATIO;
	return input;
}