#include "stdafx.h"
#include "CreatureMap.h"
#include <iostream>

using namespace std;

CreatureMap::CreatureMap(int screenX, int screenY){
	width = screenX;
	height = screenY;
	//create 1D array of creature pointers to hold the 2d map grid. Each pointer will point to the creature in that poisition
	map = new Creature*[width*height];
	//initialise all pointer to empty
	for (int i = 0; i < width*height; i++){
		map[i] = NULL;
	}
}

CreatureMap::~CreatureMap(){
	delete[] map;
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

bool CreatureMap::isCellFree(int x, int y) {
	//check input is within bounds then return cell contents
	if (x > 0 && x < width && y > 0 && y < height && map[y * width + x] == NULL) {
		return true;
	}
	else {
		return false;
	}
}

bool CreatureMap::addCreature(Creature* creature, int x, int y){
	//check the desired cell is within the bounds of the array and is empty
	if (x > 0 && x < width && y > 0 && y < height && map[y*width + x] == NULL){
		map[y*width + x] = creature;
		return true;
	}
	else {
		//returns false to say creature was not added to map
		return false;
	}
}

bool CreatureMap::moveCreature(int currentX, int currentY, char dir){
	//check that the received point is within the bounds of the map and that the cell pointed to contains a creature
	bool returnValue = false;
	if (currentX > 0 && currentX < width && currentY > 0 && currentY < height && map[currentY * width + currentX] != NULL){
		switch (dir){
		case 'u':
			//check the cell to move to within bounds and empty
			if (currentY + 1 < height && map[(currentY + 1) * width + currentX] == NULL){
				map[(currentY + 1) * width + currentX] = map[(currentY) * width + currentX];
				map[(currentY) * width + currentX] = NULL;
				returnValue = true;
			}
			break;
		case 'd':
			//check the cell to move to within bounds and empty
			if (currentY - 1 > 0 && map[(currentY - 1) * width + currentX] == NULL){
				map[(currentY - 1) * width + currentX] = map[(currentY)* width + currentX];
				map[(currentY)* width + currentX] = NULL;
				returnValue = true;
			}
			break;
		case 'l':
			//check the cell to move to within bounds and empty
			if (currentX - 1 > 0 && map[(currentY) * width + currentX - 1] == NULL){
				map[(currentY) * width + currentX - 1] = map[(currentY)* width + currentX];
				map[(currentY)* width + currentX] = NULL;
				returnValue = true;
			}
			break;
		case 'r':
			//check the cell to move to within bounds and empty
			if (currentX + 1 < width && map[(currentY)* width + currentX + 1] == NULL){
				map[(currentY)* width + currentX + 1] = map[(currentY)* width + currentX];
				map[(currentY)* width + currentX] = NULL;
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

void CreatureMap::removeCreature(int x, int y) {
	//check the cell to remove within bounds and not empty 
	if (x > 0 && x < width && y > 0 && y < height && map[y* width + x] != NULL) {
		map[y* width + x] = NULL;
	}
	else {
		cout << "Error - there is no creature in that posision to remove." << endl;
	}
}
