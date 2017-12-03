#include "stdafx.h"
#include "CreatureList.h"

using namespace std;


//when list created it is empty, need to add creatures to it
CreatureList::CreatureList(int size) {
	root = NULL;
	aliveNode = NULL;
	lengthOfList = 0;
	numOfAliveCreatures = 0;
	//add first creature and set alive node as it
	addCreature(new Creature(false));
	aliveNode = root;
	//add the rest of the creatures
	for (int i = 1; i < size; i++) {
		addCreature(new Creature(false));
	}
	collectStats = false;
}

//the list is the access point for the creatures it contains, must go to each node in the list and delete the creature it contains and the node itself
//in order to free up the momory the list takes up
CreatureList::~CreatureList() {
	//TODO deleting this and creating a new one crashes everything if the seed is different.
	while (root != NULL) {
		node* temp = root->next;
		delete root->creature;
		delete root;
		root = temp;
	}
}

//root points to the first element in the list, to add a new element it becomes the new root and points to the old root
void CreatureList::addCreature(Creature* creature) {
	node* temp = root;
	root = new node;
	root->next = temp;
	root->creature = creature;
	if (temp != NULL) {
		temp->previous = root;
	}
	lengthOfList++;
}

Creature* CreatureList::getPoolCreature() {
	//activeNode points to the next pooled creature available
	Creature* creatureToReturn;
	
	//if active node is NULL it has reached the end of the creature list and no more creature are in the pool to use.
	//create a new craeture, add it to the list and return that
	if (aliveNode == NULL) {
		creatureToReturn = new Creature(false);
		addCreature(creatureToReturn);
	}
	else {
		// should this ever happen?
		if (aliveNode->creature == NULL) {
			aliveNode->creature = new Creature(false);
		}
		creatureToReturn = aliveNode->creature;
		aliveNode = aliveNode->previous;
	}
	numOfAliveCreatures++;
	return creatureToReturn;
}

void CreatureList::returnCreatureToPool(node* creatureNodeToReturn) {

	//get the node either side connected to each other and place node at root
	// if creatureToReturn->previous == null then creatureToReturn == root
	if (creatureNodeToReturn->previous != NULL) {
		creatureNodeToReturn->previous->next = creatureNodeToReturn->next;
		if (creatureNodeToReturn->next != NULL) {
			creatureNodeToReturn->next->previous = creatureNodeToReturn->previous;
		}
		
		//creature is not root so when the list is routed around it move the craeture to the root of the list
		creatureNodeToReturn->next = root;
		root->previous = creatureNodeToReturn;
		creatureNodeToReturn->previous = NULL;
		root = creatureNodeToReturn;
	}

	// if aliveNode == null all pool creatures were in use, now therre is a free one so make aliveNode point to root, the node just pooled
	if (aliveNode == NULL){
		aliveNode = root;
	}
	numOfAliveCreatures--;
}

//cycle through the entire list and update all the creatures
void CreatureList::update(){
	//used to collect stats of creatures in the list
	int tempNumOfActiveCreatures = 0;
	float totalMass = 0;
	float totalTreeLength = 0;
	float totalDecisionsMade = 0;
	float totalEnergy = 0;
	int highestAge = 0;
	int totalAge = 0;
	int highestGeneration = 0;
	int lowestGeneration = INT_MAX;
	//activeNode points to the first available pool creature, the next is the first active creature
	node* current = NULL;
	if (aliveNode != NULL) {
		current = aliveNode->next;
	}
	else {
		current = root;
	}
	while (current != NULL){
		//pooled creatures are dead, non pooled creatures are alive, but if they are not born yet they shouldn't be updated. They are not active
		node* temp = current->next;
		if (current->creature->isActive()) {
			//creature.update() returns true unless the creature died, in which case it must be moved to the pool
			if (current->creature->update()) {
				if (collectStats) {
					tempNumOfActiveCreatures++;
					totalMass += current->creature->getMaxMass();
					totalTreeLength += current->creature->getDecisionTreeLength();
					totalDecisionsMade += current->creature->getDecisionsBeforeAction();
					totalEnergy += current->creature->getEnergy();
					totalAge += current->creature->getAge();
					if (current->creature->getAge() > highestAge) {
						highestAge = current->creature->getAge();
					}
					int currentCreatureGeneration = current->creature->getGeneration();
					if (currentCreatureGeneration > highestGeneration) {
						highestGeneration = currentCreatureGeneration;
					}
					else if (currentCreatureGeneration < lowestGeneration) {
						lowestGeneration = currentCreatureGeneration;
					}
				}
			}
			else {
				returnCreatureToPool(current);
			}
		}
		else if (!current->creature->isAlive()) {
			returnCreatureToPool(current);
		}
		current = temp;
	}
	if (collectStats && tempNumOfActiveCreatures > 0) {
		numOfActiveCreatures = tempNumOfActiveCreatures;
		averageMass = totalMass / numOfActiveCreatures;
		averageTreeLength = totalTreeLength / numOfActiveCreatures;
		averageEnergy = totalEnergy / numOfActiveCreatures;
		averageDecisionsBeforeActions = totalDecisionsMade / numOfActiveCreatures;
		numOfActiveCreatures = tempNumOfActiveCreatures;
		ageOfOldest = highestAge;
		averageAge = totalAge / numOfActiveCreatures;
		highestGen = highestGeneration;
		lowestGen = lowestGeneration;
		collectStats = false;
	}
}

void CreatureList::poolAllCreatures() {
	node* current = NULL;
	if (aliveNode != NULL) {
		current = aliveNode->next;
	}
	else {
		current = root;
	}
	while (current != NULL) {
		//pooled creatures are dead, non pooled creatures are alive, but if they are not born yet they shouldn't be updated. They are not active
		node* temp = current->next;
		if (current->creature->isActive()) {
			current->creature->kill();
			returnCreatureToPool(current);
		}
		else if (!current->creature->isAlive()) {
			returnCreatureToPool(current);
		}
		current = temp;
	}
}

int CreatureList::getAliveCreatures(Creature** &aliveCreatures) {
	aliveCreatures = new Creature*[numOfAliveCreatures];
	node* current = NULL;
	if (aliveNode != NULL) {
		current = aliveNode->next;
	}
	else {
		current = root;
	}
	int i = 0;
	while (current != NULL && i < numOfAliveCreatures) {
		aliveCreatures[i] = current->creature;
		current = current->next;
		i++;
	}
	if (!(current == NULL && i == numOfAliveCreatures)) {
		cout << "numOfAliveCreatures did not equal actual number of alive creatures." << endl;
	}
	return numOfAliveCreatures;
}

/*
act as pool and list of active creatures.
have root pointer that points to start of chain of pool and active pointer that points to the start of the active section of the list
ie 
root -> P - P - P - P - P - P - A - A - A - A - A - A
							^
							activeNode
when a new creature is born the active pointer just moves to the left and initialises that object
when a creature dies its pointers in the middle of the active section are changed to put it at the start of the list.
if active ever meets up with root just creature new creatures and add them in
*/