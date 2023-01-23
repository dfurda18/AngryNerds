#pragma once

/*
The bitshift operator '<<' makes it easy to make bitmasks
by shifting bit 1 to the left.
*/


#define CMASK_BLOCK 	(1 << 0)
#define CMASK_SHOT 		(1 << 1)
#define CMASK_EDGES 	(1 << 2)
#define CMASK_GROUND 	(1 << 3)
#define CMASK_ALIEN		(1 << 4)

