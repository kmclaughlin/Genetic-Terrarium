#ifndef CREATURELIST_H
#define CREATURELIST_H
#include "Creature.h"
#include "ResourceMap.h"
#include "CreatureMap.h"

class CreatureList {
public:
	CreatureList();

	~CreatureList();
	
	void update();
	void addCreature(Creature* creature);

private:
	//node structure to build the creature list out of
	struct node{
		Creature* creature = NULL;
		node* next = NULL;
		node* previous = NULL;
	};
	//the top of the list
	node* root;
	int numOfCreatures;
};

#endif
