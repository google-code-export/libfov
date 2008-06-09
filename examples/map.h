/*
 * Copyright (C) 2006, Greg McIntyre
 * All rights reserved. See the file named COPYING in the distribution
 * for more details.
 */

#ifndef MAP_H
#define MAP_H

#include "display.h"

#define MAPWIDTH	80
#define MAPHEIGHT	40


class CELL
{
	friend class MAP;
private:
	char tile;
	bool seen;
	bool remembered;

public:
};

class MAP
{
private:
	CELL cells[MAPWIDTH][MAPHEIGHT];

public:
	MAP(unsigned seed);

	void display(void);
	void setSeen(unsigned int x, unsigned int y);
	bool onMap(unsigned int x, unsigned int y);
	bool blockLOS(unsigned int x, unsigned int y);
	
};


#endif
