#pragma once

//_USE_MATH_DEFINES is for some nice extra defines at the bottom of math.h,
//like M_PI
#define _USE_MATH_DEFINES
#include <math.h>

#include <Box2d/Box2D.h>

//define a pixel to metres ratio for the Physics engine
#define PTM_RATIO 100.f 

//multiplier for converting radians to degrees
#define RAD2DEG (180.0/M_PI)
#define DEG2RAD (M_PI/180.0)

b2Vec2 rad2vec(float r, float m = 1);

b2Vec2 deg2vec(float r, float m = 1);

float vec2rad(b2Vec2 v);

float vec2deg(b2Vec2 v);

float rad2deg(float r);

float deg2rad(float d);

b2Vec2 Physics2Pixels(b2Vec2 input);

b2Vec2 Pixels2Physics(b2Vec2 input);
