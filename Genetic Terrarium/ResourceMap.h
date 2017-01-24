#ifndef RESOURCEMAP_H
#define RESOURCEMAP_H

class ResourceMap{
public:
	//constructor destructor
	//TODO - change initialiser to take percentage full
	ResourceMap(int screenX, int screenY, float percentage);
	~ResourceMap();

	//public methods
	void update();
	float getMaxEnergy() { return MAX_PLANT_ENERGY; };
	float getCell(int x, int y);	//returns cell contents
	float eatCell(int x, int y, float eatPercentage); //eats the percentage of cell energy specified 

private:
	int width, height;
	float* map;
	float* bufferMap;
	//probability a seedling will mature each cycle
	const float GROW_CHANCE = 0.05f;//0.1f;
	const float SEEDLING_SPAWN_CHANCE = 0.0002f;
	const float SEEDLING_SPAWN_THRESHOLD = SEEDLING_SPAWN_CHANCE * 25 * 25;
	const float MAX_PLANT_ENERGY = 1.0f;
	const float SEEDLING_ENERGY = 0.1f;
};

#endif
