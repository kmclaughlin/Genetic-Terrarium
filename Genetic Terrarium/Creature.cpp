#include "stdafx.h"
#include "Creature.h"
#include "CreatureList.h"

#define RANDOM_NORMALISED_FLOAT static_cast <float> (rand()) / static_cast <float> (RAND_MAX)

using namespace std;

/*
TODO
update flesh out and finish
move - needs creature map interaction to update creature position and check if movement position is allowed - need to check both current creature map and the updated one
foodcompare - get food check working and interaction with resourceMap
craeture creation and destruction - need full initiation of variables that are dependant on other variables
check getnext tree node
is "isoutcomeACtion" neccessary??
creature species signature calculate

*/
//id's of actions (terminators) and decisions (nodes) for update switch and filling decision tree arrays
//actions id's are < 100 and decision ids are *100 so they are identifiable in array list
//as decisions and actions are sequential only actually need to know how many there are, can get random decision by (1 + rand() % NUM_OF_DECISIONS) * 100
const  int Creature::NUM_OF_DECISIONS = 17;
const  int Creature::NUM_OF_ACTIONS = 6;

ResourceMap* Creature::resourceMap = NULL;
CreatureMap* Creature::creatureMap = NULL;
CreatureList* Creature::creatureList = NULL;

//initialise the creature from its parents
Creature::Creature(int* tree, int treeLength, bool carnivore, float maxMass, float mass, float energyThreshold, float growthRate,
				int numOffspringRange, int numOffspringMedian, int lengthOfPregnancy, int mapX, int mapY)
				:decisionTree(tree), decisionTreeLength(treeLength), carnivore(carnivore), maxMass(maxMass), mass(mass), 
				energyThreshold(energyThreshold), growthRate(growthRate), numOffspringRange(numOffspringRange), numOffspringMedian(numOffspringMedian),
				lengthOfPregnancy(lengthOfPregnancy), mapX(mapX), mapY(mapY){

	pregnant = false;
	inHeat = false;
	alive = true;

	checkVariablesWithinBounds();
	setCreatureID();
	//set age to -1 as it is just about to be incremented in updateCreatureVariables();
	age = -1;
	//update creature variables
	updateCreatureVariables();
	//creatures start with a fixed percentage of their max energy
	energy = maxEnergy * START_ENERGY_PERCENTAGE;

	//curentTreeNode holds the current decision sub node, which at this point is the whole tree, needs to be cpoied
	currentTreeNodeStart = 0;
	currentTreeNodeEnd = decisionTreeLength - 1;
}

Creature::Creature(Creature* creature, int x, int y){
	//create random decision tree between 20 and 60 total nodes
	randomTree(20 + rand() % 40);
	carnivore = creature->isCarnivore();
	//get the variables of the provided creature and vary them by +-5%
	maxMass = creature->getMaxMass() + ((RANDOM_NORMALISED_FLOAT - 0.5f) * 0.02f) * creature->getMaxMass();
	energyThreshold = creature->getEnergyThreshold() + ((RANDOM_NORMALISED_FLOAT - 0.5f) * 0.02f) * creature->getEnergyThreshold();
	growthRate = creature->getGrowthRate() + ((RANDOM_NORMALISED_FLOAT - 0.5f) * 0.02f) * creature->getGrowthRate();
	numOffspringRange = creature->getNumOffspringRange() + (rand() % 3) - 1;
	numOffspringMedian = creature->getNumOffspringMedian() + (rand() % 3) - 1;
	lengthOfPregnancy = creature->getLengthOfPregnancy() + (rand() % 5) - 2;
	mapX = x;
	mapY = y;
	
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

	//curentTreeNode holds the current decision sub node, which at this point is the whole tree, needs to be cpoied
	currentTreeNodeStart = 0;
	currentTreeNodeEnd = decisionTreeLength - 1;
}

Creature::Creature(bool carnivore) :carnivore(carnivore) {
	//create random decision tree between 20 and 60 total nodes
	randomTree(20 + rand() % 40);
	maxMass = 35;// 10 + RANDOM_NORMALISED_FLOAT * (MAX_MAX_MASS - 10);
	mass = 0.9f * maxMass;
	energyThreshold = maxMass * MAX_ENERGY_CONST * RANDOM_NORMALISED_FLOAT;
	growthRate = RANDOM_NORMALISED_FLOAT * MAX_GROWTH_RATE;
	numOffspringRange = rand() % MAX_NUM_OFFSPRING_RANGE;
	numOffspringMedian = rand() % MAX_NUM_OFFSPRING_MEDIAN;
	lengthOfPregnancy = rand() % MAX_LENGTH_OF_PREGNANCY;
	mapX = NULL;
	mapY = NULL;

	cout << maxMass << endl;
	cout << growthRate << endl;
	cout << energyThreshold << endl;
	cout << numOffspringRange << endl;
	cout << numOffspringMedian << endl;
	cout << lengthOfPregnancy << endl;
	setCreatureID();
	cout << creatureID[0] << " " << creatureID[1] << " " << creatureID[2] << endl;
	//creatures start with a fixed percentage of their max energy
	energy = maxEnergy * START_ENERGY_PERCENTAGE;
	pregnant = false;
	inHeat = false;
	alive = true;
			
	//set age to -1 as it is just about to be incremented in updateCreatureVariables();
	age = -1;
	//update creature variables
	updateCreatureVariables();

	//curentTreeNode holds the current decision sub node, which at this point is the whole tree, needs to be cpoied
	currentTreeNodeStart = 0;
	currentTreeNodeEnd = decisionTreeLength - 1;
}

Creature::~Creature(){
	//clean up, remember to delete from creature map
	delete decisionTree;
}

bool Creature::update(){
	if (!alive) {
		//remove the creature from the craeture map
		creatureMap->removeCreature(mapX, mapY);
		return false;
	}
	//reduce energy every tick regardless of action, it takes energy to live
	energy -= movementCost;
	pregnancyCheck();//

	//update creature variables
	updateCreatureVariables();
	//if pregnant and brought to term babies are born without action so long as there is a free adjacent space for them to be born into
	//consult decision tree, work through each step of the decision tree until an action is made
	actionTaken = false;
	//record current time, once time passed exceeds allotted time for 1 cycle return with no action and pick up in same place next tick
	clock_t startTime = clock();
	//TODO - to the time limit here properly with microseconds
	while (!actionTaken ){//&& clock() - startTime < MAX_TIME){
		switch(decisionTree[currentTreeNodeStart]){
			//decisions are 100s, actions < 100
		case 1:
			//take no action;
			//function to reset the decision tree
			takeAction();
			break;
		case 2:
			//move up
			if (creatureMap->moveCreature(mapX, mapY, 'u')){
				mapY++;
				energy -= movementCost;
			}
			//if the current creature is a carnivore, the target square has a creature in it and that creature isnt a carnivore
			//else if (carnivore && creatureMap->getCell(mapX, mapY) != NULL && !creatureMap->getCell(mapX, mapY)->isCarnivore()) {

			//}
			takeAction();
			break;
		case 3:
			//move down
			if (creatureMap->moveCreature(mapX, mapY, 'd')){
				mapY--;
				energy -= movementCost;
			}
			takeAction();
			break;
		case 4:
			//move left
			if (creatureMap->moveCreature(mapX, mapY, 'l')){
				mapX--;
				energy -= movementCost;
			}
			takeAction();
			break;
		case 5:
			//move right
			if (creatureMap->moveCreature(mapX, mapY, 'r')){
				mapX++;
				energy -= movementCost;
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
			nextTreeNode(foodCompare('u', 1));
			break;
		case 200:
			//is there most food in the down direction
			nextTreeNode(foodCompare('d', 1));
			break;
		case 300:
			//is there most food in the left direction
			nextTreeNode(foodCompare('l', 1));
			break;
		case 400:
			//is there most food in the right direction
			nextTreeNode(foodCompare('r', 1));
			break;
		case 500:
			nextTreeNode(foodCompare('u', 2));
			break;
		case 600:
			nextTreeNode(foodCompare('d', 2));
			break;
		case 700:
			nextTreeNode(foodCompare('l', 2));
			break;
		case 800:
			nextTreeNode(foodCompare('r', 2));
			break;
		case 900:
			nextTreeNode(animalCompare('u', 1));
			break;
		case 1000:
			nextTreeNode(animalCompare('d', 1));
			break;
		case 1100:
			nextTreeNode(animalCompare('l', 1));
			break;
		case 1200:
			nextTreeNode(animalCompare('r', 1));
			break;
		case 1300:
			nextTreeNode(animalCompare('u', 2));
			break;
		case 1400:
			nextTreeNode(animalCompare('d', 2));
			break;
		case 1500:
			nextTreeNode(animalCompare('l', 2));
			break;
		case 1600:
			nextTreeNode(animalCompare('r', 2));
			break;
		case 1700:
			nextTreeNode(energy > energyThreshold);
			break;
		case 1800:
			nextTreeNode(inHeat);
			break;
		case 1900:
			nextTreeNode(pregnant);
			break;
		case 2000:
			nextTreeNode(mature);
			break;
			/*
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
			//cout << "Error - hit default action - should never happen. " << decisionTree[currentTreeNodeStart] << endl;
			takeAction();
		}
	}
	//TODO - check if creature has < 0 energy and therefore dead, if so clean up and delete leave corpse with energy based on mass and energy at death if killed by predators
	// if dead return false
	if (energy < 0) {
		alive = false;
	}
	//check at the start of the next update if the craeture is dead or not
	return true;
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
		mass += growthRate * (maxMass - mass);
		energy -= growthRate * (maxMass - mass) * ENERGY_TO_MASS_CONST;
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

bool Creature::isCellFree(char dir){
	//check if the adjacent cell is free
	return false;

}

void Creature::lookAround() {
	//very simple algorithm for food checking, does not check for mature vs seedling plants, despite 8x energy difference
	//could square or cube the value to take this into account
	// weights 1 2 3 2 1
	//         2 3 4 3 2
	//         3 4 X 4 3
	//         2 3 4 2 2
	//         1 2 3 2 1
	int weights[25] = { 1, 2, 3, 2, 1, 
					   2, 3, 4, 3, 2,
					   3, 4, 0, 4, 3,
					   2, 3, 4, 3, 2,
					   1, 2, 3, 2, 1 };
	//reset food animal comparators
	for (int i = 0; i < 4; i++) {
		//TODO - change food nearby to a float and have it track energy content not plant maturity, then also add a food in current position comparison
		//and weighting, might need to increase food and food rank to 5 instead of 4?
		foodNearby[i] = 0;
		enemyNearby[i] = 0;
		animalNearby[i] = 0;
	}

	//
	int weightsIndex = 0;
	for (int y = mapY - 2; y < mapY + 3; y++) {
		for (int x = mapX - 2; x < mapX + 3; x++) {
			if (y > mapY){
			//check cell contents
				foodNearby[0] += weights[weightsIndex] * resourceMap->getCell(x, y);
				if (creatureMap->getCell(x, y) != NULL) {
					if (creatureMap->getCell(x, y)->isCarnivore()) {
						enemyNearby[0] += weights[weightsIndex];
					}
					else {
						animalNearby[0] += weights[weightsIndex];
					}
				}
				//friendlyNearby[0]++;
			}
			if (y < mapY){
				foodNearby[1] += weights[weightsIndex] * resourceMap->getCell(x, y);
				if (creatureMap->getCell(x, y) != NULL) {
					if (creatureMap->getCell(x, y)->isCarnivore()) {
						enemyNearby[1] += weights[weightsIndex];
					}
					else {
						animalNearby[1] += weights[weightsIndex];
					}
				}
				//friendlyNearby[1]++;
			}
			if (x < mapX){
				foodNearby[2] += weights[weightsIndex] * resourceMap->getCell(x, y);
				if (creatureMap->getCell(x, y) != NULL) {
					if (creatureMap->getCell(x, y)->isCarnivore()) {
						enemyNearby[2] += weights[weightsIndex];
					}
					else {
						animalNearby[2] += weights[weightsIndex];
					}
				}
				//friendlyNearby[2]++;
			}
			if (x > mapX){
				foodNearby[3] += weights[weightsIndex] * resourceMap->getCell(x, y);
				if (creatureMap->getCell(x, y) != NULL) {
					if (creatureMap->getCell(x, y)->isCarnivore()) {
						enemyNearby[3] += weights[weightsIndex];
					}
					else {
						animalNearby[3] += weights[weightsIndex];
					}
				}
				//friendlyNearby[3]++;
			}
			weightsIndex++;
		}
	}
	//rank food carnivores atc
	//todo - consider changing this to a feed frward type system
	//ie compare to next, if bigger decrease this, if smaller decrease that
	for (int i = 0; i < 4; i++) {
		foodRank[i] = 1;
		animalRank[i] = 1;
		for (int j = 0; j < 4; j++) {
			if (foodNearby[i] < foodNearby[j]) {
				foodRank[i] ++;
			}
			if (animalNearby[j] < animalNearby[i]) {
				animalRank[i] --;
			}
		}
	}
	lookedAround = true;
}

bool Creature::foodCompare(char dir, int rank){
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
		if (foodRank[0] == rank) {
			return true;
		}
		break;
	case 'd':
		if (foodRank[1] == rank) {
			return true;
		}
		break;
	case 'l':
		if (foodRank[2] == rank) {
			return true;
		}
		break;
	case 'r':
		if (foodRank[3] == rank) {
			return true;
		}
		break;
	}
	return false;
}

bool Creature::animalCompare(char dir, int rank) {
	//this function could be called multiple times in a decision tree, but the food distribution will always be the same
	// so check to see if it has been done already
	if (!lookedAround) {
		lookAround();
	}
	switch (dir) {
		//
	case 'u':
		if (animalRank[0] == rank) {
			return true;
		}
		break;
	case 'd':
		if (animalRank[1] == rank) {
			return true;
		}
		break;
	case 'l':
		if (animalRank[2] == rank) {
			return true;
		}
		break;
	case 'r':
		if (animalRank[3] == rank) {
			return true;
		}
		break;
	}
	return false;
}

void Creature::randomTree(int length){
	// there are only 4 rules for a valid tree from a decision tree array
	// 1. the first element must be a decision
	// 2. the last 2 elements must be actions
	// 3. the total number of actions in the tree = the total number of decisions + 1
	// 4. at any point before the end the sum of actions cannot exceed the sum of decisions to that point

	//randomly set the length of the new decision tree
	decisionTreeLength = length;
	decisionTree = new int[decisionTreeLength];

	//set the first element to a random decision to obey rule 1
	decisionTree[0] = (1 + rand() % NUM_OF_DECISIONS) * 100;
	//set the last 2 elements to random actions to obey rule 2
	decisionTree[decisionTreeLength - 1] = 1 + rand() % NUM_OF_ACTIONS;
	decisionTree[decisionTreeLength - 2] = 1 + rand() % NUM_OF_ACTIONS;

	//randomly fill the rest of the decision tree, making sure the final tree obeys rule 3
	int actions = 0;
	int decisions = 1;
	for (int i = 1; i < decisionTreeLength - 2; i++){
		//if the difference in number of decisions and actions already in the tree is less than the number of spaces left to fill and the number 
		// of actions is less than the number of decisions (to obey rule 4) then keep filling at random  
		if (decisions - actions < decisionTreeLength - 2 - i && actions < decisions){
			if (rand() % 2 == 0){
				decisionTree[i] = 1 + rand() % NUM_OF_ACTIONS;
				actions++;
			}
			else {
				decisionTree[i] = (1 + rand() % NUM_OF_DECISIONS) * 100;
				decisions++;
			}
		}
		//once the difference in decisions and actions is equal to the number of positions in the tree to fill they all must be actions
		else if (decisions - actions >= decisionTreeLength - 2 - i){
			decisionTree[i] = 1 + rand() % NUM_OF_ACTIONS;
			actions++;
		}
		else {
			decisionTree[i] = (1 + rand() % NUM_OF_DECISIONS) * 100;
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
	//red deals with carnivore, what else?
	creatureID[0] = 255;
	//green always 0? or only mapped to 127?
	//holds pregnancy details, asexual, num, range of offspring and len of preg
	//TODO - if asexual start at 1
	creatureID[1] = (1 /*for asexual*/ +
		(numOffspringRange / MAX_NUM_OFFSPRING_RANGE) +
		(numOffspringMedian / MAX_NUM_OFFSPRING_MEDIAN) +
		(lengthOfPregnancy / MAX_LENGTH_OF_PREGNANCY)) * 255 / 4;
	//creatureID[2] = 255;
	//blue covers maxMass, energyThreshold, growthrate
	creatureID[2] = (2 * (maxMass / MAX_MAX_MASS) + 
		(growthRate / MAX_GROWTH_RATE)) * 255 / 3;
	creatureID[2] = 0;
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
				creatureList->addCreature(baby[0]);
				baby[0]->beBorn(mapX + x, mapY + y);
				baby[0] = NULL;
			}
			y++;
			if (y > 1) {
				y = -1;
				x++;
			}
		}
		// make sure the whole babies list is empty before declaring not pregnant
		pregnant = false;
		
	}
}

void Creature::beBorn(int x, int y) {
	//TODO - need anything else here?
	mapX = x;
	mapY = y;
}

void Creature::generateOffspringDecisionTree(int* babyDecisionTree, int* babyTreeLength, int* mateDecisionTree, int* mateTreeLength) {

	//copy decision tree array
	for (int i = 0; i < decisionTreeLength; i++) {
		babyDecisionTree[i] = decisionTree[i];
	}
	//then have it mutate with a small chance
	if (RANDOM_NORMALISED_FLOAT < MUTATION_RATE) {
		if (mateDecisionTree == NULL) {
			int randomNode = rand() % *babyTreeLength;
			//check if the node to change is an decision, if so set to random decision, otherwise set to random action
			if (0 == babyDecisionTree[randomNode] % 100) {
				babyDecisionTree[randomNode] = (1 + rand() % NUM_OF_DECISIONS) * 100;
			}
			else {
				babyDecisionTree[randomNode] = 1 + rand() % NUM_OF_ACTIONS;
			}
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
	int babyTreeLength = decisionTreeLength;
	int* babyTree = new int[babyTreeLength];

	generateOffspringDecisionTree(babyTree, &babyTreeLength, NULL, NULL);

	float babyMaxMass = maxMass + ((RANDOM_NORMALISED_FLOAT - 0.5f) * 0.02f) * maxMass;
	float babyMass = mass * 0.35f;
	mass = mass * 0.80f;
	energy = energy * 0.80f;
	float babyEnergyThreshold = energyThreshold + ((RANDOM_NORMALISED_FLOAT - 0.5f) * 0.02f) * energyThreshold;
	float babyGrowthRate = growthRate + ((RANDOM_NORMALISED_FLOAT - 0.5f) * 0.02f) * growthRate;
	int babyNumOffspringRange = numOffspringRange + (rand() % 3) - 1;
	int babyNumOffspringMedian = numOffspringMedian + (rand() % 3) - 1;
	int babyLengthOfPregnancy = lengthOfPregnancy + (rand() % 5) - 2;

	baby[0] = new Creature(babyTree, babyTreeLength, carnivore, babyMaxMass, babyMass, babyEnergyThreshold, babyGrowthRate,
		babyNumOffspringRange, babyNumOffspringMedian, babyLengthOfPregnancy, 0, 0);
}