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
const  int Creature::NUM_OF_DECISIONS = 39;
const  int Creature::NUM_OF_ACTIONS = 6;

WorldMap* Creature::worldMap = NULL;
CreatureList* Creature::creatureList = NULL;

Creature::Creature(float carnivorism) :carnivorism(carnivorism) {
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
	mapX = NULL;
	mapY = NULL;
	mutationRate = 0.2f + RANDOM_NORMALISED_FLOAT * 0.4f;

	setCreatureID();
	//creatures start with a fixed percentage of their max energy
	energy = maxEnergy * START_ENERGY_PERCENTAGE;
	expendedEnergy = 0;
	pregnant = false;
	inHeat = false;
	alive = false;
	ticksSinceShit = 0;

	//set age to -1 as it is just about to be incremented in updateCreatureVariables();
	age = -1;
	generation = 0;
	//update creature variables
	updateCreatureVariables();

	//curentTreeNode holds the current decision sub node, which at this point is the whole tree, needs to be cpoied
	currentTreeNodeStart = 0;
	currentTreeNodeEnd = decisionTreeLength - 1;
}

Creature::~Creature() {
	//clean up, remember to delete from creature map
	delete[] decisionTree;
	for (int i = 0; i < 1; i++) {
		if (baby[i] != NULL) {
			//the creature is being deleted - should its unborn babies also be deleted?
			delete baby[i];
		}
	}
}
//initialise the creature from its parents

void Creature::setCreatureAttributes(int* tree, int treeLength, float _carnivorism, float _maxMass, float _mass, float _energy,
	float _energyThreshold, float _mutationRate, float _growthRate, int _generation) {

	active = false;
	actionTaken = false;
	lookedAround = false;

	decisionTree = tree;
	decisionTreeLength = treeLength;
	carnivorism = _carnivorism;
	maxMass = _maxMass;
	mass = _mass;
	energy = _energy;
	expendedEnergy = 0;
	energyThreshold = _energyThreshold;
	mutationRate = _mutationRate;
	growthRate = _growthRate;
	generation = _generation;
	mapX = NULL;
	mapY = NULL;

	checkVariablesWithinBounds();
	setCreatureID();


	pregnant = false;
	inHeat = false;
	alive = true;
	ticksSinceShit = 0;

	//set age to -1 as it is just about to be incremented in updateCreatureVariables();
	age = -1;
	//update creature variables
	updateCreatureVariables();

	//curentTreeNode holds the current decision sub node, which at this point is the whole tree, needs to be cpoied
	currentTreeNodeStart = 0;
	currentTreeNodeEnd = decisionTreeLength - 1;
}

void Creature::setCreatureAttributes(Creature* creature) {
 
	active = false;
	actionTaken = false;
	lookedAround = false;

	//create random decision tree between 20 and 60 total nodes
	randomTree(20 + xor128() % 40);
	mutationRate2 = creature->getMutationRate() * creature->getMutationRate();
	carnivorism = creature->getCarnivorism() + ((RANDOM_NORMALISED_FLOAT - 0.5f) * mutationRate2) * creature->getCarnivorism();
	//get the variables of the provided creature and vary them by +-5%
	maxMass = creature->getMaxMass() + ((RANDOM_NORMALISED_FLOAT - 0.5f) * mutationRate2) * creature->getMaxMass();
	energyThreshold = creature->getEnergyThreshold() + ((RANDOM_NORMALISED_FLOAT - 0.5f) * mutationRate2) * creature->getEnergyThreshold();
	mutationRate = creature->getMutationRate() + ((RANDOM_NORMALISED_FLOAT - 0.5f) * mutationRate2) * creature->getMutationRate();
	growthRate = creature->getGrowthRate() + ((RANDOM_NORMALISED_FLOAT - 0.5f) * mutationRate2) * creature->getGrowthRate();
	mapX = NULL;
	mapY = NULL;

	checkVariablesWithinBounds();
	setCreatureID();
	mass = 0.9f * maxMass;

	pregnant = false;
	inHeat = false;
	alive = true;
	ticksSinceShit = 0;

	//set age to -1 as it is just about to be incremented in updateCreatureVariables();
	age = -1;
	//update creature variables
	updateCreatureVariables();

	//creatures start with a fixed percentage of their max energy
	energy = maxEnergy * START_ENERGY_PERCENTAGE;
	expendedEnergy = 0;

	//curentTreeNode holds the current decision sub node, which at this point is the whole tree, needs to be cpoied
	currentTreeNodeStart = 0;
	currentTreeNodeEnd = decisionTreeLength - 1;
}

bool Creature::update() {
	decisionsBeforeAction = -1;
	//reduce energy every tick regardless of action, it takes energy to live
	energy -= movementCost;
	expendedEnergy += movementCost;
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
			if (worldMap->moveCreature(mapX, mapY, 'u')) {
				mapY++;
				energy -= movementCost;
				expendedEnergy += movementCost;
			}
			//if the current creature is a carnivore and the target square has a creature in it
			else if (carnivorism > 0.8 && worldMap->getCell(mapX, mapY + 1).creature != NULL) {// && !worldMap->getCell(mapX, mapY + 1).creature->isCarnivore()) {
																							   //kill the creature then move up
				worldMap->addCarcass(mapX, mapY + 1, worldMap->getCell(mapX, mapY + 1).creature->kill());
				//killing takes energy
				energy -= movementCost;
				expendedEnergy += movementCost;
				if (worldMap->moveCreature(mapX, mapY, 'u')) {
					mapY++;
					energy -= movementCost;
					expendedEnergy += movementCost;
				}
			}
			takeAction();
			break;
		case 3:
			//move down
			if (worldMap->moveCreature(mapX, mapY, 'd')) {
				mapY--;
				energy -= movementCost;
				expendedEnergy += movementCost;
			}
			//if the current creature is a carnivore and the target square has a creature in it
			else if (carnivorism > 0.8 && worldMap->getCell(mapX, mapY - 1).creature != NULL) {// && !worldMap->getCell(mapX, mapY - 1).creature->isCarnivore()) {
				worldMap->addCarcass(mapX, mapY - 1, worldMap->getCell(mapX, mapY - 1).creature->kill());
				//killing takes energy
				energy -= movementCost;
				expendedEnergy += movementCost;
				if (worldMap->moveCreature(mapX, mapY, 'd')) {
					mapY--;
					energy -= movementCost;
					expendedEnergy += movementCost;
				}
			}
			takeAction();
			break;
		case 4:
			//move left
			if (worldMap->moveCreature(mapX, mapY, 'l')) {
				mapX--;
				energy -= movementCost;
				expendedEnergy += movementCost;
			}
			//if the current creature is a carnivore and the target square has a creature in it
			else if (carnivorism > 0.8 && worldMap->getCell(mapX - 1, mapY).creature != NULL) {// && !worldMap->getCell(mapX - 1, mapY).creature->isCarnivore()) {
				worldMap->addCarcass(mapX - 1, mapY, worldMap->getCell(mapX - 1, mapY).creature->kill());
				//killing takes energy
				energy -= movementCost;
				expendedEnergy += movementCost;
				if (worldMap->moveCreature(mapX, mapY, 'l')) {
					mapX--;
					energy -= movementCost;
					expendedEnergy += movementCost;
				}
			}
			takeAction();
			break;
		case 5:
			//move right
			if (worldMap->moveCreature(mapX, mapY, 'r')) {
				mapX++;
				energy -= movementCost;
				expendedEnergy += movementCost;
			}
			//if the current creature is a carnivore and the target square has a creature in it
			else if (carnivorism > 0.8 && worldMap->getCell(mapX + 1, mapY).creature != NULL) {// && !worldMap->getCell(mapX + 1, mapY).creature->isCarnivore()) {
				worldMap->addCarcass(mapX + 1, mapY, worldMap->getCell(mapX + 1, mapY).creature->kill());
				//killing takes energy
				energy -= movementCost;
				expendedEnergy += movementCost;
				if (worldMap->moveCreature(mapX, mapY, 'r')) {
					mapX++;
					energy -= movementCost;
					expendedEnergy += movementCost;
				}
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
			nextTreeNode(compare('u', 1, 'p'));
			break;
		case 200:
			//is there most food in the down direction
			nextTreeNode(compare('d', 1, 'p'));
			break;
		case 300:
			//is there most food in the left direction
			nextTreeNode(compare('l', 1, 'p'));
			break;
		case 400:
			//is there most food in the right direction
			nextTreeNode(compare('r', 1, 'p'));
			break;
		case 500:
			nextTreeNode(compare('u', 2, 'p'));
			break;
		case 600:
			nextTreeNode(compare('d', 2, 'p'));
			break;
		case 700:
			nextTreeNode(compare('l', 2, 'p'));
			break;
		case 800:
			nextTreeNode(compare('r', 2, 'p'));
			break;
		case 900:
			nextTreeNode(compare('u', 1, 'h'));
			break;
		case 1000:
			nextTreeNode(compare('d', 1, 'h'));
			break;
		case 1100:
			nextTreeNode(compare('l', 1, 'h'));
			break;
		case 1200:
			nextTreeNode(compare('r', 1, 'h'));
			break;
		case 1300:
			nextTreeNode(compare('u', 2, 'h'));
			break;
		case 1400:
			nextTreeNode(compare('d', 2, 'h'));
			break;
		case 1500:
			nextTreeNode(compare('l', 2, 'h'));
			break;
		case 1600:
			nextTreeNode(compare('r', 2, 'h'));
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
			nextTreeNode(worldMap->isCellFree(mapX, mapY + 1));
			break;
		case 2100:
			//check if cell down is free
			nextTreeNode(worldMap->isCellFree(mapX, mapY - 1));
			break;
		case 2200:
			//check if cell left is free
			nextTreeNode(worldMap->isCellFree(mapX + 1, mapY));
			break;
		case 2300:
			//check if cell right is free
			nextTreeNode(worldMap->isCellFree(mapX - 1, mapY));
			break;
		case 2400:
			//compare carnivores
			nextTreeNode(compare('u', 1, 'c'));
			break;
		case 2500:
			nextTreeNode(compare('d', 1, 'c'));
			break;
		case 2600:
			nextTreeNode(compare('l', 1, 'c'));
			break;
		case 2700:
			nextTreeNode(compare('r', 1, 'c'));
			break;
		case 2800:
			nextTreeNode(compare('u', 2, 'c'));
			break;
		case 2900:
			nextTreeNode(compare('d', 2, 'c'));
			break;
		case 3000:
			nextTreeNode(compare('l', 2, 'c'));
			break;
		case 3100:
			nextTreeNode(compare('r', 2, 'c'));
			break;
		case 3200:
			//compare carcasses
			nextTreeNode(compare('u', 1, 'b'));
			break;
		case 3300:
			nextTreeNode(compare('d', 1, 'b'));
			break;
		case 3400:
			nextTreeNode(compare('l', 1, 'b'));
			break;
		case 3500:
			nextTreeNode(compare('r', 1, 'b'));
			break;
		case 3600:
			nextTreeNode(compare('u', 2, 'b'));
			break;
		case 3700:
			nextTreeNode(compare('d', 2, 'b'));
			break;
		case 3800:
			nextTreeNode(compare('l', 2, 'b'));
			break;
		case 3900:
			nextTreeNode(compare('r', 2, 'b'));
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
	//TODO - check if creature has < 0 energy and therefore dead, if so clean up and delete leave carcass with energy based on mass and energy at death if killed by predators
	// if dead return false
	if (energy < 0.005 * maxEnergy) {
		worldMap->addCarcass(mapX, mapY, kill());
	}
	if (RANDOM_NORMALISED_FLOAT < age * CHANCE_OF_DEATH) {
		alive = false;
	}
	//check at the start of the next update if the craeture is dead or not
	return alive;
}

void Creature::nextTreeNode(bool lastDecision) {
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
	if (lastDecision) {
		//record the length of the new sub tree
		currentTreeNodeStart = currentTreeNodeStart + 1;
		currentTreeNodeEnd = falseSubTreeStart - 1;
	}
	else {
		currentTreeNodeStart = falseSubTreeStart;
		//current tree node end stays the same;
	}

}

void Creature::updateCreatureVariables() {

	//**************
	//TODO - fix these mass dependant variables
	//************
	if (mass < maxMass) {
		float massGained = growthRate * (maxMass - mass);
		mass += massGained;
		energy -= massGained * ENERGY_TO_MASS_CONST;
	}

	movementCost = mass * MOVEMENT_COST_CONST; //increse exponentially (linearly?) with mass

	if (mass / maxMass > 0.8f) {
		mature = true;
	}
	else {
		mature = false;
	}

	//defense = MAX_DEFENSE/(mass*mass);//increases exponentially with mass 
	maxEnergy = mass * MAX_ENERGY_CONST; //dependant on mass, increase linearly with mass? - set this quite high i think
	if (energy > maxEnergy) {
		float difference = energy - maxEnergy;
		energy = maxEnergy;
		expendedEnergy += difference;
	}

	age++;
	ticksSinceShit++;
	if (ticksSinceShit > 15) {
		ticksSinceShit = 0;
		worldMap->addNutrients(mapX, mapY, expendedEnergy);
		expendedEnergy = 0;
	}
}

void Creature::takeAction() {
	actionTaken = true;
	//eat
	if (energy < maxEnergy) {
		//if the cell conatins a carcass eat a bit depending on creature size and how carnivorous
		if (worldMap->getCell(mapX, mapY).carcass != 0)
			energy += worldMap->eatCarcass(mapX, mapY, (mass / MAX_MAX_MASS) * carnivorism);
		// otherwise eat the grass depeding on inverse of carnivorous
		else
			energy = energy + worldMap->eatPlant(mapX, mapY, (mass / MAX_MAX_MASS) * (1 - carnivorism));
	}
	//TODO - maybe move this to the end of the update and have happen every update not just when action takes?
	lookedAround = false;
	//reset the current tree node bounds to the whole tree
	currentTreeNodeStart = 0;
	currentTreeNodeEnd = decisionTreeLength - 1;
}

void Creature::addToNearby(int index, int* weights, int weightsIndex, MapCell mapCell) {
	//check cell contents
	plantsNearby[index] = plantsNearby[index] + weights[weightsIndex] * mapCell.plantValue;
	carcassNearby[index] = carcassNearby[index] + weights[weightsIndex] * mapCell.carcass;
	if (mapCell.creature != NULL) {
		carnivoreNearby[index] = carnivoreNearby[index] + weights[weightsIndex] * mapCell.creature->getCarnivorism();
		herbivoreNearby[index] += herbivoreNearby[index] + weights[weightsIndex] * (1 - mapCell.creature->getCarnivorism());
	}
	//friendlyNearby[0]++;
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

	int carnivoreWeights[49] = { 1, 2,  4,  8,  4,  2,  1,
		2, 4,  8,  16, 8,  4,  2,
		4, 8,  16, 32, 16, 8,  4,
		8, 16, 32, 0,  32, 16, 8,
		4, 8,  16, 32, 16, 8,  4,
		2, 4,  8,  16, 8,  4,  2,
		1, 2,  4,  8,  4,  2,  1 };

	/*int carnivoreWeights[441] = {  1,  2,  3,  4,  5,  6,  7,  8,  9, 10, 11, 10,  9,  8,  7,  6,  5,  4,  3,  2,  1,
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
	1,  2,  3,  4,  5,  6,  7,  8,  9, 10, 11, 10,  9,  8,  7,  6,  5,  4,  3,  2,  1 };*/
	//reset food animal comparators
	for (int i = 0; i < 4; i++) {
		//TODO - change food nearby to a float and have it track energy content not plant maturity, then also add a food in current position comparison
		//and weighting, might need to increase food and food rank to 5 instead of 4?
		plantsNearby[i] = 0;
		carnivoreNearby[i] = 0;
		herbivoreNearby[i] = 0;
		carcassNearby[i] = 0;
	}

	//
	int weightsIndex = 0;
	int min, max;
	int* weights = NULL;
	if (carnivorism > 0.8) { //TODO fix
		min = -3;
		max = 4;
		weights = carnivoreWeights;
	}
	else {
		min = -2;
		max = 3;
		weights = herbivoreWeights;
	}
	for (int y = mapY + min; y < mapY + max; y++) {
		for (int x = mapX + min; x < mapX + max; x++) {
			MapCell mapCell = worldMap->getCell(x, y);
			if (y > mapY)
				addToNearby(0, weights, weightsIndex, mapCell);
			if (y < mapY)
				addToNearby(1, weights, weightsIndex, mapCell);
			if (x < mapX)
				addToNearby(2, weights, weightsIndex, mapCell);
			if (x > mapX)
				addToNearby(3, weights, weightsIndex, mapCell);
			weightsIndex++;
		}
	}
	//rank food carnivores atc
	//todo - consider changing this to a feed frward type system
	//ie compare to next, if bigger decrease this, if smaller decrease that
	for (int i = 0; i < 4; i++) {
		plantsRank[i] = 1;
		herbivoreRank[i] = 1;
		carnivoreRank[i] = 1;
		carcassRank[i] = 1;
		for (int j = 0; j < 4; j++) {
			if (plantsNearby[i] < plantsNearby[j]) {
				plantsRank[i] ++;
			}
			if (herbivoreNearby[i] < herbivoreNearby[j]) {
				herbivoreRank[i] ++;
			}
			if (carnivoreNearby[i] < carnivoreNearby[j]) {
				carnivoreRank[i] ++;
			}
			if (carcassNearby[i] < carcassNearby[j]) {
				carcassRank[i] ++;
			}

		}
	}
	lookedAround = true;
}

bool Creature::compare(char dir, int rank, char toCompare) {
	//this function could be called multiple times in a decision tree, but the distribution compared will always be the same
	// so check to see if it has been done already
	//TODO - should i move this into a different funtion? with a moves since looked check and have it be an action to look?
	// also for looking conasdier making the looking weights different for each thing looked at and have them evolve with the creature, also consider 
	//doing this for the look distance
	if (!lookedAround) {
		lookAround();
	}
	int* comparing = NULL;
	switch (toCompare) {
	case 'p':
		comparing = plantsRank;
		break;
	case 'h':
		comparing = herbivoreRank;
		break;
	case 'c':
		comparing = carnivoreRank;
		break;
	case 'b':
		comparing = carcassRank;
		break;
	}
	switch (dir) {
		//
	case 'u':
		return (comparing[0] == rank);
	case 'd':
		return (comparing[1] == rank);
	case 'l':
		return (comparing[2] == rank);
	case 'r':
		return (comparing[3] == rank);
	}
	return false;
}

void Creature::randomTree(int length) {
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
	for (int i = 1; i < decisionTreeLength - 2; i++) {
		//if the difference in number of decisions and actions already in the tree is less than the number of spaces left to fill and the number 
		// of actions is less than the number of decisions (to obey rule 4) then keep filling at random  
		if (decisions - actions < decisionTreeLength - 2 - i && actions < decisions) {
			if (xor128() % 2 == 0) {
				//increase the liklihood of replication in random tree to make morelikely to catch on
				//if (RANDOM_NORMALISED_FLOAT < 0.0)
				//	decisionTree[i] = 6;
				//else
				decisionTree[i] = 1 + xor128() % NUM_OF_ACTIONS;
				actions++;
			}
			else {
				decisionTree[i] = (1 + xor128() % NUM_OF_DECISIONS) * 100;
				decisions++;
			}
		}
		//once the difference in decisions and actions is equal to the number of positions in the tree to fill they all must be actions
		else if (decisions - actions >= decisionTreeLength - 2 - i) {
			decisionTree[i] = 1 + xor128() % NUM_OF_ACTIONS;
			actions++;
		}
		else {
			decisionTree[i] = (1 + xor128() % NUM_OF_DECISIONS) * 100;
			decisions++;
		}
	}
}

void Creature::checkVariablesWithinBounds() {
	//TODO these all need checked and fixed
	if (maxMass < 10) {
		maxMass = 10;
	}
	else if (maxMass > MAX_MAX_MASS) {
		maxMass = MAX_MAX_MASS;
	}

	if (energyThreshold < 0) {
		energyThreshold = 0;
	}
	else if (energyThreshold > maxMass * MAX_ENERGY_CONST) {
		energyThreshold = maxMass * MAX_ENERGY_CONST;
	}

	if (growthRate < 0) {
		growthRate = 0;
	}
	else if (growthRate > MAX_GROWTH_RATE) {
		growthRate = MAX_GROWTH_RATE;
	}

	if (mutationRate < 0.0f)
		mutationRate = 0.0f;
	else if (mutationRate > MAX_MUTATION_RATE)
		mutationRate = MAX_MUTATION_RATE;

	mutationRate2 = mutationRate * mutationRate;
	mutationRate3 = mutationRate2 * mutationRate;

	if (carnivorism < 0) {
		carnivorism = 0;
	}
	else if (carnivorism > 1.0f) {
		carnivorism = 1.0f;
	}
}

float Creature::getTotalEnergy() {
	float totEnergy = 0;
	if (baby[0])
		totEnergy += baby[0]->getTotalEnergy();
	return totEnergy + energy + expendedEnergy + (mass * ENERGY_TO_MASS_CONST);
}

float Creature::kill() {
	for (int i = 0; i < 1; i++) {
		if (baby[i] != NULL) {
			expendedEnergy += baby[i]->kill();
			baby[i] = NULL;
		}
	}
	float returnEnergy = energy + expendedEnergy + (mass * ENERGY_TO_MASS_CONST);
	energy = 0.0f;
	expendedEnergy = 0.0f;
	mass = 0.0f;
	if (active) {
		//remove the creature from the craeture map
		worldMap->removeCreature(mapX, mapY);
	}
	alive = false;
	active = false;
	return returnEnergy;
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
	creatureID[0] = carnivorism * 255;// 127;
	creatureID[1] = 0;
	creatureID[2] = (1 - carnivorism) * 255;
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
			if (worldMap->addCreature(baby[0], mapX + x, mapY + y)) {
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
	float randomNumber = 0.0f;
	int* tempTree = decisionTree;
	int tempTreeLength = decisionTreeLength;
	while (randomNumber < mutationRate) {
		randomNumber = RANDOM_NORMALISED_FLOAT;
		if (randomNumber < mutationRate3) {
			//find the start and end of a random node to copy
			int nodeToCopyStart = xor128() % tempTreeLength;
			int nodeToCopyEnd = nodeToCopyStart;

			int decisions = 1;
			int actions = 0;
			do {
				if (nodeToCopyEnd >= tempTreeLength) {
					cout << "Error - tree node search exceeded bounds of tree" << endl;
				}
				else if (tempTree[nodeToCopyEnd] % 100 == 0) {
					decisions++;
				}
				else {
					actions++;
				}
				nodeToCopyEnd++;
			} while (decisions > actions);

			//find the start and end of a random node to replace
			int nodeToReplaceStart = xor128() % tempTreeLength;
			int nodeToReplaceEnd = nodeToReplaceStart;

			decisions = 1;
			actions = 0;
			do {
				if (nodeToReplaceEnd >= tempTreeLength) {
					cout << "Error - tree node search exceeded bounds of tree" << endl;
				}
				else if (tempTree[nodeToReplaceEnd] % 100 == 0) {
					decisions++;
				}
				else {
					actions++;
				}
				nodeToReplaceEnd++;
			} while (decisions > actions);

			//the length of the new decision tree is the length of the original minus the replaced node plus the copied node
			babyTreeLength = tempTreeLength - (nodeToReplaceEnd - nodeToReplaceStart) + (nodeToCopyEnd - nodeToCopyStart);

			babyDecisionTree = new int[babyTreeLength];

			//copy the decision tree to the baby tree up to the point you wish to replace
			for (int i = 0; i < nodeToReplaceStart; i++) {
				babyDecisionTree[i] = tempTree[i];
			}
			//then copy the node to be copied into the space for the one to be replaced (don't have to be the same length)
			for (int i = 0; i < nodeToCopyEnd - nodeToCopyStart; i++) {
				babyDecisionTree[nodeToReplaceStart + i] = tempTree[nodeToCopyStart + i];
			}
			//copy the rest of the decision tree to the baby tree from the end of the replaced node onwards
			for (int i = 0; i < babyTreeLength - (nodeToReplaceStart + nodeToCopyEnd - nodeToCopyStart); i++) {
				babyDecisionTree[nodeToReplaceStart + nodeToCopyEnd - nodeToCopyStart + i] = tempTree[nodeToReplaceEnd + i];
			}
		}
		//chance of some random action or decision being randomised to some other action or decision (doesn't change action to decision or vice versa)
		else if (randomNumber < mutationRate) {
			babyTreeLength = tempTreeLength;
			babyDecisionTree = new int[babyTreeLength];

			//copy decision tree array
			for (int i = 0; i < tempTreeLength; i++) {
				babyDecisionTree[i] = tempTree[i];
			}

			int randomNode = xor128() % babyTreeLength;
			//check if the node to change is an decision, if so set to random decision, otherwise set to random action
			if (0 == babyDecisionTree[randomNode] % 100) {
				babyDecisionTree[randomNode] = (1 + xor128() % NUM_OF_DECISIONS) * 100;
			}
			else {
				babyDecisionTree[randomNode] = 1 + xor128() % NUM_OF_ACTIONS;
			}
		}
		//if no mutation just copy the tree
		else {
			babyTreeLength = tempTreeLength;
			babyDecisionTree = new int[babyTreeLength];

			//copy decision tree array
			for (int i = 0; i < tempTreeLength; i++) {
				babyDecisionTree[i] = tempTree[i];
			}
		}
		//swap baby tree to temp tree
		if (tempTree != decisionTree)
			delete[] tempTree;
		tempTree = babyDecisionTree;
		tempTreeLength = babyTreeLength;
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

	float babyMaxMass = maxMass + ((RANDOM_NORMALISED_FLOAT - 0.5f) * mutationRate2) * maxMass;
	//TODO - doubled max energy constant (from 0.75) and changed the 3 values below. increased max plant energy from 1 to 1.5
	float babyMass = mass * 0.30f;
	float babyEnergy = energy * 0.30f;
	mass = mass - babyMass;
	energy = energy - babyEnergy;
	float babyEnergyThreshold = energyThreshold + ((RANDOM_NORMALISED_FLOAT - 0.5f) * mutationRate2) * maxMass * MAX_ENERGY_CONST; //max energy threshold
	float babyGrowthRate = growthRate + ((RANDOM_NORMALISED_FLOAT - 0.5f) * mutationRate2) * MAX_GROWTH_RATE;
	float babyCarnivore = carnivorism + ((RANDOM_NORMALISED_FLOAT - 0.5f) * mutationRate2) * 1.0f; //max carnivorism
	float babyMutationRate = mutationRate + ((RANDOM_NORMALISED_FLOAT - 0.5f) * mutationRate2) * MAX_MUTATION_RATE; //max mutationRate
																													//if (RANDOM_NORMALISED_FLOAT < 0.1) {
																													//	babyCarnivore = true;
																													//}

	baby[0] = creatureList->getPoolCreature();
	baby[0]->setCreatureAttributes(babyTree, babyTreeLength, babyCarnivore, babyMaxMass, babyMass, babyEnergy, babyEnergyThreshold,
		babyMutationRate, babyGrowthRate, generation + 1);
}

//TODO COMPLETELY BROKEN
bool Creature::isSameSpecies(float* statsToCheck) {
	//check if the other creature falls within the bounds of what is considered to be the same species
	//TODO possibly add more here?
	float speciesVar = 0.03f;

	if (statsToCheck[0] != carnivorism) {
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