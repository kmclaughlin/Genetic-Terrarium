#include "stdafx.h"
#include "WorldMap.h"

using namespace std;
#define RANDOM_NORMALISED_FLOAT static_cast <float> (xor128()) / static_cast <float> (UINT_MAX)

WorldMap::WorldMap(int screenX, int screenY, float plantCoverPct) {
	width = screenX;
	height = screenY;
	map = new MapCell[width * height];

	//initialise array, all cells assigned an energy value between 0 and MAX_PLANT_ENERGY
	for (int i = 0; i < width * height; i++) {
		if (RANDOM_NORMALISED_FLOAT < plantCoverPct) {//* 0.35) {
			map[i].plantValue = MAX_PLANT_ENERGY;// *RANDOM_NORMALISED_FLOAT;
		}
		else {
			map[i].plantValue = 0;
		}
	}
}

WorldMap::~WorldMap() {
	//delete the map and buffer arrays that are stored in the heap
	delete[] map;
}

MapCell WorldMap::getCell(int x, int y) {
	//check input is within bounds then return cell contents
	if (x > 0 && x < width && y > 0 && y < height) {
		return map[y * width + x];
	}
	else {
		MapCell mc;
		return mc;
	}
}

float WorldMap::eatCell(int x, int y, float eatPercentage) {
	//don't neeed to bounds check because the thing doing the eating must be inside the bounds on the resource map
	//may be poor practice though to make that assumption, so consider adding boundss check with a todo to remove for optimisation
	//The amount of energy that is trying to be eaten
	float returnValue = MAX_PLANT_ENERGY * eatPercentage;
	//check that the value requested does not exceed the value in the cell
	if (returnValue > map[y * width + x].plantValue) {
		returnValue = map[y * width + x].plantValue;
		map[y * width + x].plantValue = 0;
	}
	else {
		map[y * width + x].plantValue -= returnValue;
	}
	return returnValue;
}

void WorldMap::updatePlants() {
	//update each cell in the resource map
	for (int y = 0; y < height; y++) {
		for (int x = 0; x < width; x++) {
			//if the plant is at 0 energy no operations need to happen
			if (map[y * width + x].plantValue > 0) {
				//create random number between 0 and 1, only need to do this once per cell
				float randomNumber = RANDOM_NORMALISED_FLOAT;
				//float randomNumber = static_cast <float> (rand()) / static_cast <float> (RAND_MAX);

				// if map cell contains a more than 0 but less than MAX_PLANT_ENERGY have it update with growChance and pass it into bufferMap
				// if checking 2 events check the least likely first for efficiency
				if (randomNumber < GROW_CHANCE) {
					if (randomNumber < GROW_CHANCE && map[y * width + x].plantValue < MAX_PLANT_ENERGY) {
						map[y * width + x].plantValue += SEEDLING_ENERGY; //plant grows
						// TODO is this check necessary? can't grow once above max plant anyway, would possibly being above max plant break anything?
						if (map[y * width + x].plantValue > MAX_PLANT_ENERGY) {
							map[y * width + x].plantValue = MAX_PLANT_ENERGY;
						}
					}
					//the more mature plants close by the higher the chance of spawning a new seedling
					//only do this if the cell is empty
					//check if the seedling has a reasonable chance of spawning before doing all the expensive calculations, 
					//ie check it against the maximum output of the calculation
					//TODO - could lower the max here as an upper limit of spawn chance, where extra plants make no diference ie 15*15
					else if (randomNumber < SEEDLING_SPAWN_CHANCE && map[y * width + x].plantValue >= MAX_PLANT_ENERGY) {
						int randX = x + static_cast <int> (xor128() % 5) - 2;
						int randY = y + static_cast <int> (xor128() % 5) - 2;
						//std::cout << static_cast <int> (xor128() % 20) - 9 <<  std::endl;
						if (randX > 0 && randY > 0 && randX < width && randY < height && map[randY * width + randX].plantValue <= 0)
							//std::cout << "INSIDE " << x << " " << randX << std::endl;
							map[randY * width + randX].plantValue = SEEDLING_ENERGY;
					}
				}
				
			}
		}
	}
}


bool WorldMap::isCellFree(int x, int y) {
	//check input is within bounds then return cell contents
	if (x > 0 && x < width && y > 0 && y < height && map[y * width + x].creature == NULL) {
		return true;
	}
	else {
		return false;
	}
}

bool WorldMap::addCreature(Creature* creature, int x, int y) {
	//check the desired cell is within the bounds of the array and is empty
	if (x > 0 && x < width && y > 0 && y < height && map[y*width + x].creature == NULL) {
		map[y*width + x].creature = creature;
		return true;
	}
	else {
		//returns false to say creature was not added to map
		return false;
	}
}

bool WorldMap::moveCreature(int currentX, int currentY, char dir) {
	//check that the received point is within the bounds of the map and that the cell pointed to contains a creature
	bool returnValue = false;
	if (currentX > 0 && currentX < width && currentY > 0 && currentY < height && map[currentY * width + currentX].creature != NULL) {
		switch (dir) {
		case 'u':
			//check the cell to move to within bounds and empty
			if (currentY + 1 < height && map[(currentY + 1) * width + currentX].creature == NULL) {
				map[(currentY + 1) * width + currentX].creature = map[(currentY)* width + currentX].creature;
				map[(currentY)* width + currentX].creature = NULL;
				returnValue = true;
			}
			break;
		case 'd':
			//check the cell to move to within bounds and empty
			if (currentY - 1 > 0 && map[(currentY - 1) * width + currentX].creature == NULL) {
				map[(currentY - 1) * width + currentX].creature = map[(currentY)* width + currentX].creature;
				map[(currentY)* width + currentX].creature = NULL;
				returnValue = true;
			}
			break;
		case 'l':
			//check the cell to move to within bounds and empty
			if (currentX - 1 > 0 && map[(currentY)* width + currentX - 1].creature == NULL) {
				map[(currentY)* width + currentX - 1].creature = map[(currentY)* width + currentX].creature;
				map[(currentY)* width + currentX].creature = NULL;
				returnValue = true;
			}
			break;
		case 'r':
			//check the cell to move to within bounds and empty
			if (currentX + 1 < width && map[(currentY)* width + currentX + 1].creature == NULL) {
				map[(currentY)* width + currentX + 1].creature = map[(currentY)* width + currentX].creature;
				map[(currentY)* width + currentX].creature = NULL;
				returnValue = true;
			}
			break;
		}
	}
	else {
		cout << "Error - Cannot move creature, CreatureMap cell is either null or out of bounds" << endl;
	}
	return returnValue;
}

void WorldMap::removeCreature(int x, int y) {
	//check the cell to remove within bounds and not empty 
	if (x > 0 && x < width && y > 0 && y < height && map[y* width + x].creature != NULL) {
		map[y* width + x].creature = NULL;
	}
	else {
		cout << "Error - there is no creature in that posision to remove." << endl;
	}
}
