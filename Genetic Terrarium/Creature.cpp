#include "stdafx.h"
#include "Creature.h"
#include "CreatureList.h"

#define RANDOM_NORMALISED_FLOAT static_cast <float> (xor128()) / static_cast <float> (UINT_MAX)

using namespace std;

/*
TODO
update flesh out and finish
craeture creation and destruction - need full initiation of variables that are dependant on other variables
is "isoutcomeACtion" neccessary??
creature species signature calculate
split decision trees out int its own object

*/
//id's of actions (terminators) and decisions (nodes) for update switch and filling decision tree arrays
//actions id's are < 100 and decision ids are *100 so they are identifiable in array list
//as decisions and actions are sequential only actually need to know how many there are, can get random decision by (1 + xor128() % NUM_OF_DECISIONS) * 100
const  int Creature::NUM_OF_DECISIONS = 31;
const  int Creature::NUM_OF_ACTIONS = 6;

ResourceMap* Creature::resourceMap = NULL;
CreatureMap* Creature::creatureMap = NULL;
CreatureList* Creature::creatureList = NULL;

Creature::Creature(bool carnivore) :carnivore(carnivore) {
	active = false;
	actionTaken = false;
	lookedAround = false;
	//create random decision tree between 20 and 60 total nodes
	decisionTree = NULL;
	decisionTreeLength = 0;
	maxMass = 15;// 10 + RANDOM_NORMALISED_FLOAT * (MAX_MAX_MASS - 10);
	mass = 0.9f * maxMass;
	energyThreshold = maxMass * MAX_ENERGY_CONST * RANDOM_NORMALISED_FLOAT;
	growthRate = RANDOM_NORMALISED_FLOAT * MAX_GROWTH_RATE;
	numOffspringRange = xor128() % MAX_NUM_OFFSPRING_RANGE;
	numOffspringMedian = xor128() % MAX_NUM_OFFSPRING_MEDIAN;
	lengthOfPregnancy = xor128() % MAX_LENGTH_OF_PREGNANCY;
	mapX = NULL;
	mapY = NULL;

	setCreatureID();
	//creatures start with a fixed percentage of their max energy
	energy = maxEnergy * START_ENERGY_PERCENTAGE;
	pregnant = false;
	inHeat = false;
	alive = false;
			
	//set age to -1 as it is just about to be incremented in updateCreatureVariables();
	age = -1;
	generation = 0;
	//update creature variables
	updateCreatureVariables();

	//curentTreeNode holds the current decision sub node, which at this point is the whole tree, needs to be cpoied
	currentTreeNodeStart = 0;
	currentTreeNodeEnd = decisionTreeLength - 1;
}

Creature::~Creature(){
	//clean up, remember to delete from creature map
	delete decisionTree;
	for (int i = 0; i < 1; i++) {
		if (baby[i] != NULL) {
			//the creature is being deleted - should its unborn babies also be deleted?
			baby[i]->kill();
			baby[i] = NULL;
		}
	}
}
//initialise the creature from its parents

void Creature::setCreatureAttributes(int* tree, int treeLength, bool _carnivore, float _maxMass, float _mass, float _energy, float _energyThreshold, 
	float _growthRate, int _numOffspringRange, int _numOffspringMedian, int _lengthOfPregnancy, int _generation) {

	delete[] decisionTree;

	active = false;
	actionTaken = false;
	lookedAround = false;

	decisionTree = tree;
	decisionTreeLength = treeLength;
	carnivore = _carnivore;
	maxMass = _maxMass;
	mass = _mass;
	energy = _energy;
	energyThreshold = _energyThreshold;
	growthRate = _growthRate;
	numOffspringRange = _numOffspringRange;
	numOffspringMedian = _numOffspringMedian;
	lengthOfPregnancy = _lengthOfPregnancy;
	generation = _generation;
	mapX = NULL;
	mapY = NULL;

	checkVariablesWithinBounds();
	setCreatureID();


	pregnant = false;
	inHeat = false;
	alive = true;

	//set age to -1 as it is just about to be incremented in updateCreatureVariables();
	age = -1;
	//update creature variables
	updateCreatureVariables();
	//creatures start with a fixed percentage of their max energy
	if (energy <= 0) {
		energy = maxEnergy * START_ENERGY_PERCENTAGE;
	}

	//curentTreeNode holds the current decision sub node, which at this point is the whole tree, needs to be cpoied
	currentTreeNodeStart = 0;
	currentTreeNodeEnd = decisionTreeLength - 1;
}

void Creature::setCreatureAttributes(Creature* creature) {
	delete[] decisionTree;
	active = false;
	actionTaken = false;
	lookedAround = false;

	//create random decision tree between 20 and 60 total nodes
	randomTree(20 + xor128() % 40);
	carnivore = creature->isCarnivore();
	//get the variables of the provided creature and vary them by +-5%
	maxMass = creature->getMaxMass() + ((RANDOM_NORMALISED_FLOAT - 0.5f) * 0.02f) * creature->getMaxMass();
	energyThreshold = creature->getEnergyThreshold() + ((RANDOM_NORMALISED_FLOAT - 0.5f) * 0.02f) * creature->getEnergyThreshold();
	growthRate = creature->getGrowthRate() + ((RANDOM_NORMALISED_FLOAT - 0.5f) * 0.02f) * creature->getGrowthRate();
	numOffspringRange = creature->getNumOffspringRange() + (xor128() % 3) - 1;
	numOffspringMedian = creature->getNumOffspringMedian() + (xor128() % 3) - 1;
	lengthOfPregnancy = creature->getLengthOfPregnancy() + (xor128() % 5) - 2;
	mapX = NULL;
	mapY = NULL;

	checkVariablesWithinBounds();
	setCreatureID();
	mass = 0.9f * maxMass;

	pregnant = false;
	inHeat = false;
	alive = true;

	//set age to -1 as it is just about to be incremented in updateCreatureVariables();
	age = -1;
	//update creature variables
	updateCreatureVariables();

	//creatures start with a fixed percentage of their max energy
	energy = maxEnergy * START_ENERGY_PERCENTAGE;
	if (carnivore) {
		energy = maxEnergy * START_ENERGY_PERCENTAGE * 20;
	}

	//curentTreeNode holds the current decision sub node, which at this point is the whole tree, needs to be cpoied
	currentTreeNodeStart = 0;
	currentTreeNodeEnd = decisionTreeLength - 1;
}

bool Creature::update(){
	decisionsBeforeAction = -1;
	bool returnValue = true;
	if (!alive) {
		//remove the creature from the craeture map
		creatureMap->removeCreature(mapX, mapY);
		for (int i = 0; i < 1; i++) {
			if (baby[i] != NULL) {
				baby[i]->kill();
				baby[i] = NULL;
			}
		}
		active = false;
		returnValue = false;
	}
	else {
		//reduce energy every tick regardless of action, it takes energy to live
		energy -= movementCost;
		pregnancyCheck();

		//update creature variables
		updateCreatureVariables();
		//if pregnant and brought to term babies are born without action?? could just set action taken so long as there is a free adjacent space for them to be born into
		//consult decision tree, work through each step of the decision tree until an action is made
		actionTaken = false;
		//record current time, once time passed exceeds allotted time for 1 cycle return with no action and pick up in same place next tick
		clock_t startTime = clock();
		//TODO - to the time limit here properly with microseconds
		while (!actionTaken) {//&& clock() - startTime < MAX_TIME){
			switch (decisionTree[currentTreeNodeStart]) {
				//decisions are 100s, actions < 100
			case 1:
				//take no action;
				//function to reset the decision tree
				takeAction();
				break;
			case 2:
				//move up
				if (creatureMap->moveCreature(mapX, mapY, 'u')) {
					mapY++;
					energy -= movementCost;
				}
				//if the current creature is a carnivore and the target square has a creature in it
				else if (carnivore && creatureMap->getCell(mapX, mapY + 1) != NULL) {//&& !creatureMap->getCell(mapX, mapY + 1)->isCarnivore()) {
					energy += creatureMap->getCell(mapX, mapY + 1)->getEnergy();
					creatureMap->getCell(mapX, mapY + 1)->kill();
				}
				takeAction();
				break;
			case 3:
				//move down
				if (creatureMap->moveCreature(mapX, mapY, 'd')) {
					mapY--;
					energy -= movementCost;
				}
				//if the current creature is a carnivore and the target square has a creature in it
				else if (carnivore && creatureMap->getCell(mapX, mapY - 1) != NULL ){//&& !creatureMap->getCell(mapX, mapY - 1)->isCarnivore()) {
					energy += creatureMap->getCell(mapX, mapY - 1)->getEnergy();
					creatureMap->getCell(mapX, mapY - 1)->kill();
				}
				takeAction();
				break;
			case 4:
				//move left
				if (creatureMap->moveCreature(mapX, mapY, 'l')) {
					mapX--;
					energy -= movementCost;
				}
				//if the current creature is a carnivore and the target square has a creature in it
				else if (carnivore && creatureMap->getCell(mapX - 1, mapY) != NULL) {//&& !creatureMap->getCell(mapX - 1, mapY)->isCarnivore()) {
					energy += creatureMap->getCell(mapX - 1, mapY)->getEnergy();
					creatureMap->getCell(mapX - 1, mapY)->kill();
				}
				takeAction();
				break;
			case 5:
				//move right
				if (creatureMap->moveCreature(mapX, mapY, 'r')) {
					mapX++;
					energy -= movementCost;
				}
				//if the current creature is a carnivore and the target square has a creature in it
				else if (carnivore && creatureMap->getCell(mapX + 1, mapY) != NULL) {//&& !creatureMap->getCell(mapX + 1, mapY)->isCarnivore()) {
					energy += creatureMap->getCell(mapX + 1, mapY)->getEnergy();
					creatureMap->getCell(mapX + 1, mapY)->kill();
				}
				takeAction();
				break;
			case 6:
				if (!pregnant && mature) {
					inHeat = true;
				}
				takeAction();
				break;
				//become in heat
				/*case 6:
				//move up and left
				move('q');
				takeAction();
				break;
				case 7:
				//move up and right
				move('e');
				takeAction();
				break;
				case 8:
				//move down and left
				move('z');
				takeAction();
				break;
				case 9:
				//move down and right
				move('c');
				takeAction();
				break;
				/*
				case 10:
				//breed? can become pregnant, move adjacent to mature creature of same species costs 1 movement energy
				//action to become in heat, and action to leave it?
				takeAction();
				break;
				case 11:
				fight?
				takeAction();
				break;
				*/
			case 100:
				//is there most food in the up direction
				nextTreeNode(plantsCompare('u', 1));
				break;
			case 200:
				//is there most food in the down direction
				nextTreeNode(plantsCompare('d', 1));
				break;
			case 300:
				//is there most food in the left direction
				nextTreeNode(plantsCompare('l', 1));
				break;
			case 400:
				//is there most food in the right direction
				nextTreeNode(plantsCompare('r', 1));
				break;
			case 500:
				nextTreeNode(plantsCompare('u', 2));
				break;
			case 600:
				nextTreeNode(plantsCompare('d', 2));
				break;
			case 700:
				nextTreeNode(plantsCompare('l', 2));
				break;
			case 800:
				nextTreeNode(plantsCompare('r', 2));
				break;
			case 900:
				nextTreeNode(herbivoreCompare('u', 1));
				break;
			case 1000:
				nextTreeNode(herbivoreCompare('d', 1));
				break;
			case 1100:
				nextTreeNode(herbivoreCompare('l', 1));
				break;
			case 1200:
				nextTreeNode(herbivoreCompare('r', 1));
				break;
			case 1300:
				nextTreeNode(herbivoreCompare('u', 2));
				break;
			case 1400:
				nextTreeNode(herbivoreCompare('d', 2));
				break;
			case 1500:
				nextTreeNode(herbivoreCompare('l', 2));
				break;
			case 1600:
				nextTreeNode(herbivoreCompare('r', 2));
				break;
			case 1700:
				nextTreeNode(energy > energyThreshold);
				break;
			case 1800:
				nextTreeNode(pregnant);
				break;
			case 1900:
				nextTreeNode(mature);
				break;
			case 2000:
				//check if cell up is free
				nextTreeNode(creatureMap->isCellFree(mapX, mapY + 1));
				break;
			case 2100:
				//check if cell down is free
				nextTreeNode(creatureMap->isCellFree(mapX, mapY - 1));
				break;
			case 2200:
				//check if cell left is free
				nextTreeNode(creatureMap->isCellFree(mapX + 1, mapY));
				break;
			case 2300:
				//check if cell right is free
				nextTreeNode(creatureMap->isCellFree(mapX - 1, mapY));
				break;
			case 2400:
				nextTreeNode(carnivoreCompare('u', 1));
				break;
			case 2500:
				nextTreeNode(carnivoreCompare('d', 1));
				break;
			case 2600:
				nextTreeNode(carnivoreCompare('l', 1));
				break;
			case 2700:
				nextTreeNode(carnivoreCompare('r', 1));
				break;
			case 2800:
				nextTreeNode(carnivoreCompare('u', 2));
				break;
			case 2900:
				nextTreeNode(carnivoreCompare('d', 2));
				break;
			case 3000:
				nextTreeNode(carnivoreCompare('l', 2));
				break;
			case 3100:
				nextTreeNode(carnivoreCompare('r', 2));
				break;
				/*
				case 1800:
				nextTreeNode(inHeat);
				break;
				is in heat
				if pregnant
				if more food, predators, sameSpecies up, right left down
				if energy > energy threshold
				if mature
				if cell is free up down left right
				if
				*/
			default:
				//should never get here, log an error
				cout << "Error - hit default action - should never happen. " << decisionTree[currentTreeNodeStart] << endl;
			}
			decisionsBeforeAction++;
		}
		//TODO - check if creature has < 0 energy and therefore dead, if so clean up and delete leave corpse with energy based on mass and energy at death if killed by predators
		// if dead return false
		if (energy < 0) {
			alive = false;
		}
		if (RANDOM_NORMALISED_FLOAT < age * CHANCE_OF_DEATH) {
			alive = false;
		}
		//check at the start of the next update if the craeture is dead or not
		returnValue = true;
	}
	return returnValue;
}

void Creature::nextTreeNode(bool lastDecision){
	//current tree node doesn't need to create tons of array copies, it can be 2 ints one that tracks the start of the current node in the 
	//decision tree and one that tracks the end, then they just collapse in as the decision is made, pointing to smaller and smaller sub trees

	//the current node of the decision tree has 2 outcomes, true and false. each outcome leads to a sub tree of unknown length 
	int decisions = 0;
	int actions = 0;
	int falseSubTreeStart = currentTreeNodeStart;
	//the next element in the decision tree is the positive outcome of the current decision. first need to record where the end of that sub tree is
	// in the decision tree array.
	do {
		if (falseSubTreeStart >= decisionTreeLength) {
			cout << "Error - tree node search exceeded bounds of tree" << endl;
		}
		else if (decisionTree[falseSubTreeStart] % 100 == 0) {
			decisions++;
		}
		else {
			actions++;
		}
		falseSubTreeStart++;
	} while (decisions > actions);

	//if the decision was true point to the start and end of the true sub tree
	if (lastDecision){
		//record the length of the new sub tree
		currentTreeNodeStart = currentTreeNodeStart + 1;
		currentTreeNodeEnd = falseSubTreeStart - 1;
	}
	else{
		currentTreeNodeStart = falseSubTreeStart;
		//current tree node end stays the same;
	}

}

//check if the outcome of the current decision is an action or another node
/*bool Creature::isOutcomeAction(bool decision){
	if (decision){
		if (currentTreeNode[1] % 100 == 0){
			return false;
		}
		else {
			return true;
		}
	}
	else {
		//the current node of the decision tree has 2 outcomes, true and false. each outcome leads to a sub tree of unknown length 
		int decisions = 1;
		int actions = 0;
		int falseSubTreeStart = 1;
		//the next element in the decision tree is the positive outcome of the current decision. first need to record where the end of that sub tree is
		// in the decision tree array.
		while (decisions > actions){
			if (currentTreeNode[falseSubTreeStart] % 100 == 0){
				decisions++;
			}
			else {
				actions++;
			}
			falseSubTreeStart++;
		}
		if (currentTreeNode[falseSubTreeStart] % 100 == 0){
			return false;
		}
		else {
			return true;
		}
	}
}*/

void Creature::updateCreatureVariables(){
	
	//**************
	//TODO - fix these mass dependant variables
	//************
	if (mass < maxMass){
		float massGained = growthRate * (maxMass - mass);
		mass += massGained;
		energy -= massGained * ENERGY_TO_MASS_CONST;
	}
	
	movementCost = mass * MOVEMENT_COST_CONST; //increse exponentially with mass
	
	if (mass / maxMass > 0.8f){
		mature = true;
	}
	else {
		mature = false;
	}
	
	//defense = MAX_DEFENSE/(mass*mass);//increases exponentially with mass 
	maxEnergy = mass * MAX_ENERGY_CONST; //dependant on mass, increase linearly with mass? - set this quite high i think
	if (energy > maxEnergy) {
		energy = maxEnergy;
	}
	
	age++;
}

void Creature::takeAction(){
	actionTaken = true;
	if (!carnivore) {
		energy += resourceMap->eatCell(mapX, mapY, mass / MAX_MAX_MASS);
	}
	//TODO - maybe move this to the end of the update and have happen every update not just when action takes?
	lookedAround = false;
	//reset the current tree node bounds to the whole tree
	currentTreeNodeStart = 0;
	currentTreeNodeEnd = decisionTreeLength - 1;
}

void Creature::lookAround() {
	//very simple algorithm for food checking, does not check for mature vs seedling plants, despite 8x energy difference
	//could square or cube the value to take this into account
	// weights 1 2 3 2 1
	//         2 3 4 3 2
	//         3 4 X 4 3
	//         2 3 4 2 2
	//         1 2 3 2 1
	int herbivoreWeights[25] = { 1, 2, 3, 2, 1, 
								 2, 3, 4, 3, 2,
								 3, 4, 0, 4, 3,
								 2, 3, 4, 3, 2,
								 1, 2, 3, 2, 1 };

	int carnivoreWeights[441] = {  1,  2,  3,  4,  5,  6,  7,  8,  9, 10, 11, 10,  9,  8,  7,  6,  5,  4,  3,  2,  1,
								   2,  3,  4,  5,  6,  7,  8,  9, 10, 11, 12, 11, 10,  9,  8,  7,  6,  5,  4,  3,  2,
								   3,  4,  5,  6,  7,  8,  9, 10, 11, 12, 13, 12, 11, 10,  9,  8,  7,  6,  5,  4,  3,
								   4,  5,  6,  7,  8,  9, 10, 11, 12, 13, 14, 13, 12, 11, 10,  9,  8,  7,  6,  5,  4,
								   5,  6,  7,  8,  9, 10, 11, 12, 13, 14, 15, 14, 13, 12, 11, 10,  9,  8,  7,  6,  5,
								   6,  7,  8,  9, 10, 11, 12, 13, 14, 15, 16, 15, 14, 13, 12, 11, 10,  9,  8,  7,  6,
								   7,  8,  9, 10, 11, 12, 13, 14, 15, 16, 17, 16, 15, 14, 13, 12, 11, 10,  9,  8,  7,
								   8,  9, 10, 11, 12, 13, 14, 15, 16, 17, 18, 17, 16, 15, 14, 13, 12, 11, 10,  9,  8,
								   9, 10, 11, 12, 13, 14, 15, 16, 17, 18, 19, 18, 17, 16, 15, 14, 13, 12, 11, 10,  9,
								  10, 11, 12, 13, 14, 15, 16, 17, 18, 19, 20, 19, 18, 17, 16, 15, 14, 13, 12, 11, 10,
								  11, 12, 13, 14, 15, 16, 17, 18, 19, 20,  0, 20, 19, 18, 17, 16, 15, 14, 13, 12, 11,
								  10, 11, 12, 13, 14, 15, 16, 17, 18, 19, 20, 19, 18, 17, 16, 15, 14, 13, 12, 11, 10,
								   9, 10, 11, 12, 13, 14, 15, 16, 17, 18, 19, 18, 17, 16, 15, 14, 13, 12, 11, 10,  9,
								   8,  9, 10, 11, 12, 13, 14, 15, 16, 17, 18, 19, 16, 15, 14, 13, 12, 11, 10,  9,  8,
								   7,  8,  9, 10, 11, 12, 13, 14, 15, 16, 17, 16, 15, 14, 13, 12, 11, 10,  9,  8,  7,
								   6,  7,  8,  9, 10, 11, 12, 13, 14, 15, 16, 15, 14, 13, 12, 11, 10,  9,  8,  7,  6,
								   5,  6,  7,  8,  9, 10, 11, 12, 13, 14, 15, 14, 13, 12, 11, 10,  9,  8,  7,  6,  5,
								   4,  5,  6,  7,  8,  9, 10, 11, 12, 13, 14, 13, 12, 11, 10,  9,  8,  7,  6,  5,  4,
								   3,  4,  5,  6,  7,  8,  9, 10, 11, 12, 13, 12, 11, 10,  9,  8,  7,  6,  5,  4,  3,
								   2,  3,  4,  5,  6,  7,  8,  9, 10, 11, 12, 11, 10,  9,  8,  7,  6,  5,  4,  3,  2,
								   1,  2,  3,  4,  5,  6,  7,  8,  9, 10, 11, 10,  9,  8,  7,  6,  5,  4,  3,  2,  1 };
	//reset food animal comparators
	for (int i = 0; i < 4; i++) {
		//TODO - change food nearby to a float and have it track energy content not plant maturity, then also add a food in current position comparison
		//and weighting, might need to increase food and food rank to 5 instead of 4?
		plantsNearby[i] = 0;
		carnivoreNearby[i] = 0;
		herbivoreNearby[i] = 0;
	}

	//
	int weightsIndex = 0;
	if (carnivore) {
		for (int y = mapY - 10; y < mapY + 11; y++) {
			for (int x = mapX - 10; x < mapX + 11; x++) {
				if (y > mapY) {
					//check cell contents
					plantsNearby[0] += carnivoreWeights[weightsIndex] * resourceMap->getCell(x, y);
					if (creatureMap->getCell(x, y) != NULL) {
						if (creatureMap->getCell(x, y)->isCarnivore()) {
							carnivoreNearby[0] += carnivoreWeights[weightsIndex];
						}
						else {
							herbivoreNearby[0] += carnivoreWeights[weightsIndex];
						}
					}
					//friendlyNearby[0]++;
				}
				if (y < mapY) {
					plantsNearby[1] += carnivoreWeights[weightsIndex] * resourceMap->getCell(x, y);
					if (creatureMap->getCell(x, y) != NULL) {
						if (creatureMap->getCell(x, y)->isCarnivore()) {
							carnivoreNearby[1] += carnivoreWeights[weightsIndex];
						}
						else {
							herbivoreNearby[1] += carnivoreWeights[weightsIndex];
						}
					}
					//friendlyNearby[1]++;
				}
				if (x < mapX) {
					plantsNearby[2] += carnivoreWeights[weightsIndex] * resourceMap->getCell(x, y);
					if (creatureMap->getCell(x, y) != NULL) {
						if (creatureMap->getCell(x, y)->isCarnivore()) {
							carnivoreNearby[2] += carnivoreWeights[weightsIndex];
						}
						else {
							herbivoreNearby[2] += carnivoreWeights[weightsIndex];
						}
					}
					//friendlyNearby[2]++;
				}
				if (x > mapX) {
					plantsNearby[3] += carnivoreWeights[weightsIndex] * resourceMap->getCell(x, y);
					if (creatureMap->getCell(x, y) != NULL) {
						if (creatureMap->getCell(x, y)->isCarnivore()) {
							carnivoreNearby[3] += carnivoreWeights[weightsIndex];
						}
						else {
							herbivoreNearby[3] += carnivoreWeights[weightsIndex];
						}
					}
					//friendlyNearby[3]++;
				}
				weightsIndex++;
			}
		}
	}
	else {
		for (int y = mapY - 2; y < mapY + 3; y++) {
			for (int x = mapX - 2; x < mapX + 3; x++) {
				if (y > mapY) {
					//check cell contents
					plantsNearby[0] += herbivoreWeights[weightsIndex] * resourceMap->getCell(x, y);
					if (creatureMap->getCell(x, y) != NULL) {
						if (creatureMap->getCell(x, y)->isCarnivore()) {
							carnivoreNearby[0] += herbivoreWeights[weightsIndex];
						}
						else {
							herbivoreNearby[0] += herbivoreWeights[weightsIndex];
						}
					}
					//friendlyNearby[0]++;
				}
				if (y < mapY) {
					plantsNearby[1] += herbivoreWeights[weightsIndex] * resourceMap->getCell(x, y);
					if (creatureMap->getCell(x, y) != NULL) {
						if (creatureMap->getCell(x, y)->isCarnivore()) {
							carnivoreNearby[1] += herbivoreWeights[weightsIndex];
						}
						else {
							herbivoreNearby[1] += herbivoreWeights[weightsIndex];
						}
					}
					//friendlyNearby[1]++;
				}
				if (x < mapX) {
					plantsNearby[2] += herbivoreWeights[weightsIndex] * resourceMap->getCell(x, y);
					if (creatureMap->getCell(x, y) != NULL) {
						if (creatureMap->getCell(x, y)->isCarnivore()) {
							carnivoreNearby[2] += herbivoreWeights[weightsIndex];
						}
						else {
							herbivoreNearby[2] += herbivoreWeights[weightsIndex];
						}
					}
					//friendlyNearby[2]++;
				}
				if (x > mapX) {
					plantsNearby[3] += herbivoreWeights[weightsIndex] * resourceMap->getCell(x, y);
					if (creatureMap->getCell(x, y) != NULL) {
						if (creatureMap->getCell(x, y)->isCarnivore()) {
							carnivoreNearby[3] += herbivoreWeights[weightsIndex];
						}
						else {
							herbivoreNearby[3] += herbivoreWeights[weightsIndex];
						}
					}
					//friendlyNearby[3]++;
				}
				weightsIndex++;
			}
		}
	}
	//rank food carnivores atc
	//todo - consider changing this to a feed frward type system
	//ie compare to next, if bigger decrease this, if smaller decrease that
	for (int i = 0; i < 4; i++) {
		plantsRank[i] = 1;
		herbivoreRank[i] = 1;
		carnivoreRank[i] = 1;
		for (int j = 0; j < 4; j++) {
			if (plantsNearby[i] < plantsNearby[j]) {
				plantsRank[i] ++;
			}
			if (herbivoreNearby[j] < herbivoreNearby[i]) {
				herbivoreRank[i] --;
			}
			if (carnivoreNearby[j] < carnivoreNearby[i]) {
				carnivoreRank[i] --;
			}
		}
	}
	lookedAround = true;
}

bool Creature::plantsCompare(char dir, int rank){
	//this function could be called multiple times in a decision tree, but the food distribution will always be the same
	// so check to see if it has been done already
	//TODO - should i move this into a different funtion? with a moves since looked check and have it be an action to look?
	// also for looking conasdier making the looking weights different for each thing looked at and have them evolve with the creature, also consider 
	//doing this for the look distance
	if (!lookedAround){
		lookAround();
	}
	switch (dir){
		//
	case 'u':
		if (plantsRank[0] == rank) {
			return true;
		}
		break;
	case 'd':
		if (plantsRank[1] == rank) {
			return true;
		}
		break;
	case 'l':
		if (plantsRank[2] == rank) {
			return true;
		}
		break;
	case 'r':
		if (plantsRank[3] == rank) {
			return true;
		}
		break;
	}
	return false;
}

bool Creature::herbivoreCompare(char dir, int rank) {
	//this function could be called multiple times in a decision tree, but the food distribution will always be the same
	// so check to see if it has been done already
	if (!lookedAround) {
		lookAround();
	}
	switch (dir) {
		//
	case 'u':
		if (herbivoreRank[0] == rank) {
			return true;
		}
		break;
	case 'd':
		if (herbivoreRank[1] == rank) {
			return true;
		}
		break;
	case 'l':
		if (herbivoreRank[2] == rank) {
			return true;
		}
		break;
	case 'r':
		if (herbivoreRank[3] == rank) {
			return true;
		}
		break;
	}
	return false;
}

bool Creature::carnivoreCompare(char dir, int rank) {
	//this function could be called multiple times in a decision tree, but the food distribution will always be the same
	// so check to see if it has been done already
	if (!lookedAround) {
		lookAround();
	}
	switch (dir) {
		//
	case 'u':
		if (carnivoreRank[0] == rank) {
			return true;
		}
		break;
	case 'd':
		if (carnivoreRank[1] == rank) {
			return true;
		}
		break;
	case 'l':
		if (carnivoreRank[2] == rank) {
			return true;
		}
		break;
	case 'r':
		if (carnivoreRank[3] == rank) {
			return true;
		}
		break;
	}
	return false;
}

void Creature::randomTree(int length){
	// there are only 5 rules for a valid tree from a decision tree array
	// 1. the first element must be a decision
	// 2. the last 2 elements must be actions
	// 3. the total number of actions in the tree = the total number of decisions + 1
	// 4. at any point before the end the sum of actions cannot exceed the sum of decisions to that point
	// 5. the length of the array must be odd

	//randomly set the length of the new decision tree
	if (length % 2 == 0) {
		decisionTreeLength = length + 1;
	}
	else {
		decisionTreeLength = length;
	}
	decisionTree = new int[decisionTreeLength];

	//set the first element to a random decision to obey rule 1
	decisionTree[0] = (1 + xor128() % NUM_OF_DECISIONS) * 100;
	//set the last 2 elements to random actions to obey rule 2
	decisionTree[decisionTreeLength - 1] = 1 + xor128() % NUM_OF_ACTIONS;
	decisionTree[decisionTreeLength - 2] = 1 + xor128() % NUM_OF_ACTIONS;

	//randomly fill the rest of the decision tree, making sure the final tree obeys rule 3
	int actions = 0;
	int decisions = 1;
	for (int i = 1; i < decisionTreeLength - 2; i++){
		//if the difference in number of decisions and actions already in the tree is less than the number of spaces left to fill and the number 
		// of actions is less than the number of decisions (to obey rule 4) then keep filling at random  
		if (decisions - actions < decisionTreeLength - 2 - i && actions < decisions){
			if (xor128() % 2 == 0){
				decisionTree[i] = 1 + xor128() % NUM_OF_ACTIONS;
				actions++;
			}
			else {
				decisionTree[i] = (1 + xor128() % NUM_OF_DECISIONS) * 100;
				decisions++;
			}
		}
		//once the difference in decisions and actions is equal to the number of positions in the tree to fill they all must be actions
		else if (decisions - actions >= decisionTreeLength - 2 - i){
			decisionTree[i] = 1 + xor128() % NUM_OF_ACTIONS;
			actions++;
		}
		else {
			decisionTree[i] = (1 + xor128() % NUM_OF_DECISIONS) * 100;
			decisions++;
		}
	}
}

void Creature::checkVariablesWithinBounds(){
	//TODO these all need checked and fixed
	if (maxMass < 10){
		maxMass = 10;
	}
	else if (maxMass > MAX_MAX_MASS){
		maxMass = MAX_MAX_MASS;
	}
	
	if (energyThreshold < 0){
		energyThreshold = 0;
	}
	else if (energyThreshold > maxMass * MAX_ENERGY_CONST){
		energyThreshold = maxMass * MAX_ENERGY_CONST;
	}

	if (growthRate < 0){
		growthRate = 0;
	}
	else if (growthRate > MAX_GROWTH_RATE){
		growthRate = MAX_GROWTH_RATE;
	}

	if (numOffspringRange < 0){
		numOffspringRange = 0;
	}
	else if (numOffspringRange > MAX_NUM_OFFSPRING_RANGE){
		numOffspringRange = MAX_NUM_OFFSPRING_RANGE;
	}

	if (numOffspringMedian < 0){
		numOffspringMedian = 0;
	}
	else if (numOffspringMedian > MAX_NUM_OFFSPRING_MEDIAN){
		numOffspringMedian = MAX_NUM_OFFSPRING_MEDIAN;
	}

	if (lengthOfPregnancy < 0){
		lengthOfPregnancy = 0;
	}
	else if (lengthOfPregnancy > MAX_LENGTH_OF_PREGNANCY){
		lengthOfPregnancy = MAX_LENGTH_OF_PREGNANCY;
	}
}

void Creature::setCreatureID() {
	/*
	//red deals with carnivore, what else?
	creatureID[0] = 255;
	//green always 0? or only mapped to 127?
	//holds pregnancy details, asexual, num, range of offspring and len of preg
	//TODO - if asexual start at 1
	creatureID[1] = (1 for asexual +
		(numOffspringRange / MAX_NUM_OFFSPRING_RANGE) +
		(numOffspringMedian / MAX_NUM_OFFSPRING_MEDIAN) +
		(lengthOfPregnancy / MAX_LENGTH_OF_PREGNANCY)) * 255 / 4;
	//creatureID[2] = 255;
	//blue covers maxMass, energyThreshold, growthrate
	creatureID[2] = (2 * (maxMass / MAX_MAX_MASS) + 
		(growthRate / MAX_GROWTH_RATE)) * 255 / 3;
	creatureID[2] = 0;
	creatureID[1] = 0;*/
	if (carnivore) {
		creatureID[0] = 255;
		creatureID[2] = 0;
	}
	else {
		creatureID[0] = 0;// 127;
		creatureID[2] = 255;
	}
	creatureID[1] = 0;
}

void Creature::pregnancyCheck() {
	if (inHeat) {
		replicate();
		pregnant = true;
		inHeat = false;
	}
	else if (pregnant) {
		//give birth
		int x = -1;
		int y = -1;
		while (baby[0] != NULL && x < 2) {
			if (creatureMap->addCreature(baby[0], mapX + x, mapY + y)) {
				baby[0]->born(mapX + x, mapY + y);
				baby[0] = NULL;
			}
			y++;
			if (y > 1) {
				y = -1;
				x++;
			}
		}
		// TODO make sure the whole babies list is empty before declaring not pregnant
		pregnant = false;
	}
}

void Creature::born(int x, int y) {
	//TODO - need anything else here?
	active = true;
	mapX = x;
	mapY = y;
}

void Creature::generateOffspringDecisionTree(int* &babyDecisionTree, int &babyTreeLength, int* mateDecisionTree, int* mateTreeLength) {

	//then have it mutate with a small chance
	//very small chance to copy a node of decision tree and its sub tree and replace any random node and its sub tree with the copied one
	
	if (RANDOM_NORMALISED_FLOAT < MUTATION_RATE * MUTATION_RATE * MUTATION_RATE) {
		//find the start and end of a random node to copy
		int nodeToCopyStart = xor128() % decisionTreeLength;
		int nodeToCopyEnd = nodeToCopyStart;

		int decisions = 1;
		int actions = 0;
		do {
			if (nodeToCopyEnd >= decisionTreeLength) {
				cout << "Error - tree node search exceeded bounds of tree" << endl;
			}
			else if (decisionTree[nodeToCopyEnd] % 100 == 0) {
				decisions++;
			}
			else {
				actions++;
			}
			nodeToCopyEnd++;
		} while (decisions > actions);

		//find the start and end of a random node to replace
		int nodeToReplaceStart = xor128() % decisionTreeLength;
		int nodeToReplaceEnd = nodeToReplaceStart;
		
		decisions = 1;
		actions = 0;
		do {
			if (nodeToReplaceEnd >= decisionTreeLength) {
				cout << "Error - tree node search exceeded bounds of tree" << endl;
			}
			else if (decisionTree[nodeToReplaceEnd] % 100 == 0) {
				decisions++;
			}
			else {
				actions++;
			}
			nodeToReplaceEnd++;
		} while (decisions > actions);

		//the length of the new decision tree is the length of the original minus the replaced node plus the copied node
		babyTreeLength = decisionTreeLength - (nodeToReplaceEnd - nodeToReplaceStart) + (nodeToCopyEnd - nodeToCopyStart);

		babyDecisionTree = new int[babyTreeLength];

		//copy the decision tree to the baby tree up to the point you wish to replace
		for (int i = 0; i < nodeToReplaceStart; i++) {
			babyDecisionTree[i] = decisionTree[i];
		}
		//then copy the node to be copied into the space for the one to be replaced (don't have to be the same length)
		for (int i = 0; i < nodeToCopyEnd - nodeToCopyStart; i++) {
			babyDecisionTree[nodeToReplaceStart + i] = decisionTree[nodeToCopyStart + i];
		}
		//copy the rest of the decision tree to the baby tree from the end of the replaced node onwards
		for (int i = 0; i < babyTreeLength - (nodeToReplaceStart + nodeToCopyEnd - nodeToCopyStart); i++) {
			babyDecisionTree[nodeToReplaceStart + nodeToCopyEnd - nodeToCopyStart + i] = decisionTree[nodeToReplaceEnd + i];
		}
	}
	//chance of some random action or decision being randomised to some other action or decision (doesn't change action to decision or vice versa)
	else if (RANDOM_NORMALISED_FLOAT < MUTATION_RATE) {
		babyTreeLength = decisionTreeLength;
		babyDecisionTree = new int[babyTreeLength];

		//copy decision tree array
		for (int i = 0; i < decisionTreeLength; i++) {
			babyDecisionTree[i] = decisionTree[i];
		}

		if (mateDecisionTree == NULL) {
			int randomNode = xor128() % babyTreeLength;
			//check if the node to change is an decision, if so set to random decision, otherwise set to random action
			if (0 == babyDecisionTree[randomNode] % 100) {
				babyDecisionTree[randomNode] = (1 + xor128() % NUM_OF_DECISIONS) * 100;
			}
			else {
				babyDecisionTree[randomNode] = 1 + xor128() % NUM_OF_ACTIONS;
			}
		}
	}
	//if no mutation just copy the tree
	else {
		babyTreeLength = decisionTreeLength;
		babyDecisionTree = new int[babyTreeLength];

		//copy decision tree array
		for (int i = 0; i < decisionTreeLength; i++) {
			babyDecisionTree[i] = decisionTree[i];
		}
	}
}

void Creature::replicate() {
	/*
	list of babies - start with only 1
	when replicate add to list of babies
	babies get mass each tick from their parent - does parent toal movement mass then increase??
	randomise 
	*/
	//int babyTreeLength = decisionTreeLength;
	//int* babyTree = new int[babyTreeLength];

	int babyTreeLength = 0;
	int* babyTree = NULL;

	generateOffspringDecisionTree(babyTree, babyTreeLength, NULL, NULL);

	float babyMaxMass = maxMass + ((RANDOM_NORMALISED_FLOAT - 0.5f) * 0.02f) * maxMass;
	//TODO - doubled max energy constant (from 0.75) and changed the 3 values below. increased max plant energy from 1 to 1.5
	float babyMass = mass * 0.30f;//0.35
	float babyEnergy = energy * 0.30f;
	mass = mass * 0.70f;//0.8
	energy = energy * 0.70f;//0.8
	float babyEnergyThreshold = energyThreshold + ((RANDOM_NORMALISED_FLOAT - 0.5f) * 0.02f) * energyThreshold;
	float babyGrowthRate = growthRate + ((RANDOM_NORMALISED_FLOAT - 0.5f) * 0.02f) * growthRate;
	int babyNumOffspringRange = numOffspringRange + (xor128() % 3) - 1;
	int babyNumOffspringMedian = numOffspringMedian + (xor128() % 3) - 1;
	int babyLengthOfPregnancy = lengthOfPregnancy + (xor128() % 5) - 2;
	bool babyCarnivore = carnivore;
	//if (RANDOM_NORMALISED_FLOAT < 0.1) {
		//babyCarnivore = !carnivore;
	//}

	baby[0] = creatureList->getPoolCreature();
	baby[0]->setCreatureAttributes(babyTree, babyTreeLength, babyCarnivore, babyMaxMass, babyMass, babyEnergy, babyEnergyThreshold, babyGrowthRate,
		babyNumOffspringRange, babyNumOffspringMedian, babyLengthOfPregnancy, generation+1);
}

bool Creature::isSameSpecies(float* statsToCheck) {
	//check if the other creature falls within the bounds of what is considered to be the same species
	//TODO possibly add more here?
	float speciesVar = 0.03;

	if (statsToCheck[0] != carnivore) {
		return false;
	}
	else if (statsToCheck[1] > maxMass * (1.0f + speciesVar) || statsToCheck[1] < maxMass * (1.0f - speciesVar)) {
		return false;
	}
	else if (statsToCheck[2] > energyThreshold * (1.0f + speciesVar) || statsToCheck[2] < energyThreshold * (1.0f - speciesVar)) {
		return false;
	}
	else if (statsToCheck[3] > growthRate * (1.0f + speciesVar) || statsToCheck[3] < growthRate * (1.0f - speciesVar)) {
		return false;
	}
	return true;
}