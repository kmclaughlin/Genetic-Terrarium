#include "stdafx.h"
#include "ResourceMap.h"

#define RANDOM_NORMALISED_FLOAT static_cast <float> (xor128()) / static_cast <float> (UINT_MAX)

ResourceMap::ResourceMap(int screenX, int screenY, float percentage){
	width = screenX;
	height = screenY;
	map = new float[width * height];
	bufferMap = new float[width * height];

	//initialise array, all cells assigned an energy value between 0 and MAX_PLANT_ENERGY
	for (int i = 0; i < width * height; i++){
		if (RANDOM_NORMALISED_FLOAT < percentage * 0.35) {
			map[i] = MAX_PLANT_ENERGY;// *RANDOM_NORMALISED_FLOAT;
		}
		else {
			map[i] = 0;
		}
	}
}

ResourceMap::~ResourceMap(){
	//delete the map and buffer arrays that are stored in the heap
	delete[] map;
	delete[] bufferMap;
}

float ResourceMap::getCell(int x, int y){
	//check input is within bounds then return cell contents
	if (x > 0 && x < width && y > 0 && y < height){
		return map[y * width + x];
	}
	else {
		return NULL;
	}
}

float ResourceMap::eatCell(int x, int y, float eatPercentage){
	//The amount of energy that is trying to be eaten
	float returnValue = MAX_PLANT_ENERGY * eatPercentage;
	//check that the value requested does not exceed the value in the cell
	if (returnValue > map[y * width + x]) {
		returnValue = map[y * width + x];
		map[y * width + x] = 0;
	}
	else {
		map[y * width + x] -= returnValue;
	}
	return returnValue;
}

void ResourceMap::update(){
	//update each cell in the resource map
	for (int y = 0; y < height; y++){
		for (int x = 0; x < width; x++){
			//first just copy what is in map cell to bufferMap cell
			bufferMap[y * width + x] = map[y * width + x];
			//if the plant is at ax energy no operations need to happen
			if (map[y * width + x] < MAX_PLANT_ENERGY) {
				//create random number between 0 and 1, only need to do this once per cell
				float randomNumber = RANDOM_NORMALISED_FLOAT;
				//float randomNumber = static_cast <float> (rand()) / static_cast <float> (RAND_MAX);

				// if map cell contains a more than 0 but less than MAX_PLANT_ENERGY have it update with growChance and pass it into bufferMap
				// if checking 2 events check the least likely first for efficiency
				if (randomNumber < GROW_CHANCE && map[y * width + x] > 0) {
						bufferMap[y * width + x] += SEEDLING_ENERGY; //plant grows
						if (bufferMap[y * width + x] > MAX_PLANT_ENERGY) {
							bufferMap[y * width + x] = MAX_PLANT_ENERGY;
						}
				}
				//the more mature plants close by the higher the chance of spawning a new seedling
				//only do this if the cell is empty
				//check if the seedling has a reasonable chance of spawning before doing all the expensive calculations, 
				//ie check it against the maximum output of the calculation
				//TODO - could lower the max here as an upper limit of spawn chance, where extra plants make no diference ie 15*15
				else if (randomNumber < SEEDLING_SPAWN_THRESHOLD && map[y * width + x] <= 0) {
					//count up mature plants around the current spot
					int plantDensity = 0;
					for (int i = x - 2; i < x + 3; i++) {
						for (int j = y - 2; j < y + 3; j++) {
							//check that i and j are within the bounds of the array
							if (i > 0 && j > 0 && i < width && j < height) {
								//consider having this at 90% max energy
								if (map[j * width + i] >= MAX_PLANT_ENERGY) {
									//may want to weight this in favor of the inner 8 over the outer 16
									plantDensity++;
								}
							}
						}
					}
					if (randomNumber < SEEDLING_SPAWN_CHANCE * plantDensity * plantDensity) {
						bufferMap[y * width + x] = SEEDLING_ENERGY;
					}
				}
			}
		}
	}
	//swap the pointers of map and bufferMap so map is always the primary resource map
	float* tempMapPtr = map;
	map = bufferMap;
	bufferMap = tempMapPtr;
}
