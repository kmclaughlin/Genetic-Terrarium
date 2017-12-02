#ifndef WORLDMAP_H
#define WORLDMAP_H

#include "Xorshift.h"
#include <iostream>

//cannot have creature and creatureMap include each other, but can forward declare creature here and only use pointers to creaturemap
class Creature;

//cell struct to hold all the information needed for each cell of the map
struct MapCell {
	float plantValue = NULL;
	Creature* creature = NULL;;
};

class WorldMap {
public:
	//constructor destructor
	//TODO - change initialiser to take percentage full
	WorldMap(int screenX, int screenY, float plantCoverPct);
	~WorldMap();

	//public methods
	void updatePlants();
	float getMaxEnergy() { return MAX_PLANT_ENERGY; };
	MapCell getCell(int x, int y);	//returns cell contents
	float eatCell(int x, int y, float eatPercentage); //eats the percentage of cell energy specified 

	bool isCellFree(int x, int y);
	bool addCreature(Creature* creature, int x, int y);
	bool moveCreature(int currentX, int currentY, char dir);
	void removeCreature(int x, int y);

	//void addToPixelMap(sf::Uint8* pixels, int x, int y);

private:
	int width, height;
	MapCell* map;
	//probability a seedling will mature each cycle
	const float GROW_CHANCE = 0.05f;//0.1f;
	const float SEEDLING_SPAWN_CHANCE = 0.01f;
	const float MAX_PLANT_ENERGY = 1.5f;
	const float SEEDLING_ENERGY = 0.1f;
};

#endif