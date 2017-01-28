#ifndef CREATURELIST_H
#define CREATURELIST_H
#include "Creature.h"

class CreatureList {
public:
	CreatureList(int size);

	~CreatureList();
	
	void update();
	Creature* getPoolCreature();
	int getAliveCreatures(Creature** &numOfAliveCreatures);

private:
	//node structure to build the creature list out of
	struct node{
		Creature* creature = NULL;
		node* next = NULL;
		node* previous = NULL;
	};
	void returnCreatureToPool(node* creatureNodeToReturn);
	void addCreature(Creature* creature);
	//the top of the list
	node* root;
	node* activeNode;
	int lengthOfList;
	int numOfAliveCreatures;
};

#endif
