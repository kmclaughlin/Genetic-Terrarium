#include "stdafx.h"
#include "CreatureList.h"

using namespace std;


//when list created it is empty, need to add creatures to it
CreatureList::CreatureList(int size) {
	root = NULL;
	activeNode = NULL;
	lengthOfList = size;
	numOfActiveCreatures = 0;
	for (int i = 0; i < size; i++) {
		Creature* poolCreature = new Creature(false);

		node* temp = root;
		root = new node;
		root->next = temp;
		root->creature = poolCreature;
		//the first time through this loop root will be null so there will be no node to get the previous off
		if (temp != NULL) {
			temp->previous = root;
		}
		//set active to be the very first creature created
		else {
			activeNode = root;
		}

	}
}

//the list is the access point for the creatures it contains, must go to each node in the list and delete the creature it contains and the node itself
//in order to free up the momory the list takes up
CreatureList::~CreatureList() {
	while (root != NULL) {
		node* temp = root->next;
		delete root->creature;
		delete root;
		root = temp;
	}
}

//root points to the first element in the list, to add a new element it becomes the new root and points to the old root
void CreatureList::addCreature(Creature* creature) {
	//cout << " added new node" << endl;
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
	if (activeNode == NULL) {
		creatureToReturn = new Creature(false);
		addCreature(creatureToReturn);
	}
	else {
		if (activeNode->creature == NULL) {
			cout << "uyagsfoiaugoaiug!!!!!!!!!!!!" << endl;
			activeNode->creature = new Creature(false);
		}
		creatureToReturn = activeNode->creature;
		activeNode = activeNode->previous;
	}
	numOfActiveCreatures++;
	return creatureToReturn;
}

void CreatureList::returnCreatureToPool(node* creatureNodeToReturn) {

	//get the node either side connected to each other and place node at root
	if (creatureNodeToReturn->previous != NULL) {
		creatureNodeToReturn->previous->next = creatureNodeToReturn->next;
		if (creatureNodeToReturn->next != NULL) {
			creatureNodeToReturn->next->previous = creatureNodeToReturn->previous;
		}
	}

	if (creatureNodeToReturn != root) {
		creatureNodeToReturn->next = root;
		root->previous = creatureNodeToReturn;
		creatureNodeToReturn->previous = NULL;
		root = creatureNodeToReturn;
	}
	if (activeNode == NULL){
		activeNode = root;
	}
	/*
	//before removing the current node from the list connect the previous node to the next
	if (creatureNodeToReturn->next != NULL) {
		creatureNodeToReturn->next->previous = creatureNodeToReturn->previous;
	}
	if (creatureNodeToReturn->previous != NULL) {
		creatureNodeToReturn->previous->next = creatureNodeToReturn->next;
	}
	//if the current node being deleted is the root node, update the root to be the next node 
	if (creatureNodeToReturn == root) {
		root = creatureNodeToReturn->next;
	}*/
	//delete creatureNodeToReturn->creature;
	//creatureNodeToReturn->creature = NULL;
	//delete creatureNodeToReturn;
	numOfActiveCreatures--;
}

//cycle through the entire list and update all the creatures
void CreatureList::update(){
	int count = 0;
	float totalMass = 0;
	float totalTreeLength = 0;
	float totalDecisionsMade = 0;
	//activeNode points to the first available pool creature, the next is the first active creature
	node* current = NULL;
	if (activeNode != NULL) {
		current = activeNode->next;
	}
	else {
		current = root;
	}
	while (current != NULL){
		//cout << activeNode << " " << current->next << " " << root << endl;
		//pooled creatures are dead, non pooled creatures are alive, but if they are not born yet they shouldn't be updated. They are not active
		node* temp = current->next;
		if (current->creature->isActive()) {
			count++;
			//creature.update() returns true unless the creature died, in which case it must be moved to the pool
			if (current->creature->update()) {
				totalMass += current->creature->getMaxMass();
				totalTreeLength += current->creature->getDecisionTreeLength();
				totalDecisionsMade += current->creature->getDecisionsBeforeAction();
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
	cout << "total creatures: " << count << "   average mass: " << totalMass / count << "  average tree length: " << totalTreeLength / count << endl;
	cout << "total pool len : " << lengthOfList << "   num active: " << numOfActiveCreatures << "   average decisions b/ action: " << totalDecisionsMade / count << endl;
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