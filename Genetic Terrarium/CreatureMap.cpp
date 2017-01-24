#include "stdafx.h"
#include "CreatureMap.h"
#include <iostream>

using namespace std;

CreatureMap::CreatureMap(int screenX, int screenY){
	width = screenX;
	height = screenY;
	//create 2 2d array to hold resources in double buffer pattern
	//create an array of pointers of size width and point them all to arrays of ints of size height
	map = new Creature*[width*height];
	//bufferMap = new Creature*[width*height];
	for (int i = 0; i < width*height; i++){
		map[i] = NULL;
		//bufferMap[i] = NULL;
	}
}

CreatureMap::~CreatureMap(){
	delete[] map;
	//delete[] bufferMap;
}

Creature* CreatureMap::getCell(int x, int y){
	//check input is within bounds then return cell contents
	if (x > 0 && x < width && y > 0 && y < height){
		return map[y * width + x];
	}
	else {
		return NULL;
	}
}

bool CreatureMap::addCreature(Creature* creature, int x, int y){
	if (x > 0 && x < width && y > 0 && y < height && map[y*width + x] == NULL){
		map[y*width + x] = creature;
		return true;
	}
	else {
		//TODO - bring back this error message?
		//cout << "Error - Cannot add creature, CreatureMap cell is either occupied or out of bounds" << endl;
		return false;
	}
}

bool CreatureMap::moveCreature(int currentX, int currentY, char dir){
	//all movement updates go into buffer map then all get transfered to map at once with the update method
	//check that the received point is within the bounds of the map and that the cell pointed to contains a creature
	if (currentX > 0 && currentX < width && currentY > 0 && currentY < height && map[currentY * width + currentX] != NULL){
		switch (dir){
		case 'u':
			//check the cell to move to is not occupied or outside the map
			if (currentY + 1 < height && map[(currentY + 1) * width + currentX] == NULL){
				map[(currentY + 1) * width + currentX] = map[(currentY) * width + currentX];
				map[(currentY) * width + currentX] = NULL;
				return true;
			}
			else {
				return false;
			}
		case 'd':
			//check the cell to move to is not occupied or outside the map
			if (currentY - 1 > 0 && map[(currentY - 1) * width + currentX] == NULL){
				map[(currentY - 1) * width + currentX] = map[(currentY)* width + currentX];
				map[(currentY)* width + currentX] = NULL;
				return true;
			}
			else {
				return false;
			}
		case 'l':
			//check the cell to move to is not occupied or outside the map
			if (currentX - 1 > 0 && map[(currentY) * width + currentX - 1] == NULL){
				map[(currentY) * width + currentX - 1] = map[(currentY)* width + currentX];
				map[(currentY)* width + currentX] = NULL;
				return true;
			}
			else {
				return false;
			}
		case 'r':
			//check the cell to move to is not occupied or outside the map
			if (currentX + 1 < width && map[(currentY)* width + currentX + 1] == NULL){
				map[(currentY)* width + currentX + 1] = map[(currentY)* width + currentX];
				map[(currentY)* width + currentX] = NULL;
				return true;
			}
			else {
				return false;
			}
		default:
			return false;
		}
	}
	else {
		cout << "Error - Cannot move creature, CreatureMap cell is either null or out of bounds" << endl;
		return false;
	}
}

void CreatureMap::removeCreature(int x, int y) {
	if (map[y* width + x] != NULL) {
		map[y* width + x] = NULL;
	}
	else {
		cout << "Error - there is no creature in that posision to remove." << endl;
	}
}
