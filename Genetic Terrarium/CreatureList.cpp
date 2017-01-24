#include "stdafx.h"
#include "CreatureList.h"
//#include "Creature.h"


//when list created it is empty, need to add creatures to it
CreatureList::CreatureList(){
	root = NULL;
	numOfCreatures = 0;
}

//the list is the access point for the creatures it contains, must go to each node in the list and delete the creature it contains and the node itself
//in order to free up the momory the list takes up
CreatureList::~CreatureList(){
	while(root != NULL){
		node* temp = root->next;
		delete root->creature;
		delete root;
		root = temp;
	}
}

//root points to the first element in the list, to add a new element it becomes the new root and points to the old root
void CreatureList::addCreature(Creature* creature){
	node* temp = root;
	root = new node;
	root->next = temp;
	root->creature = creature;
	if(temp != NULL){
		temp->previous = root;
	}
	numOfCreatures++;
}

//cycle through the entire list and update all the creatures
void CreatureList::update(){
	int count = 0;
	float totalMass = 0;
	node* current = root;
	while (current != NULL){
		count++;
		//creature.update() returns true unless the creature died, in which case it must be removed from the list and its next and previous now
		//need to point to each other
		if (current->creature->update()){
			totalMass += current->creature->getMaxMass();
			current = current->next;
		}
		else {
			//before removing the current node from the list connect the previous node to the next
			if (current->next != NULL){
				current->next->previous = current->previous;
			}
			if (current->previous != NULL){
				current->previous->next = current->next;
			}
			//if the current node being deleted is the root node, update the root to be the next node 
			if (current == root) {
				root = current->next;
			}
			node* temp = current;
			current = current->next;
			delete temp->creature;
			delete temp;
			numOfCreatures--;
		}
	}
	std::cout << "total creatures: " << numOfCreatures << "   average mass: " << totalMass / numOfCreatures << std::endl;
}