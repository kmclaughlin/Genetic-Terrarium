#ifndef CREATURELIST_H
#define CREATURELIST_H
#include "Creature.h"

class CreatureList {
public:
	CreatureList(int size);

	~CreatureList();
	
	void update();
	void poolAllCreatures();
	Creature* getPoolCreature();
	int getAliveCreatures(Creature** &numOfAliveCreatures);
	int getLengthOfList() { return lengthOfList; };
	int getNumOfActiveCreatures() { return numOfActiveCreatures; };
	float getAverageMass() { return averageMass; };
	float getAverageTreeLength() { return averageTreeLength; };
	float getAverageEnergy() { return averageEnergy; };
	float getAverageCarnivorism() { return averageCarnivorism; };
	float getAverageMutationRate() { return averageMutationRate; };
	float getAverageDecisionsBeforeActions() { return averageDecisionsBeforeActions; };
	int getAgeOfOldest() { return ageOfOldest; };
	int getAverageAge() { return averageAge; };
	int getHighestGen() { return highestGen; };
	int getLowestGen() { return lowestGen; };
	void collectCreatureStats(void) { collectStats = true; };

private:
	//node structure to build the creature list out of
	struct node{
		Creature* creature = NULL;
		node* next = NULL;
		node* previous = NULL;
	};
	void returnCreatureToPool(node* creatureNodeToReturn);
	void addCreature(Creature* creature);
	node* root;	//the top of the list
	node* aliveNode; //next free slot in the list, everything after this is occupied by an alive creature, this and everything before it is pooled
	bool collectStats;
	//list attributes
	int lengthOfList;
	int numOfAliveCreatures; //alive creatures are removed from the pool, contains all active and inactive creatures
	int numOfActiveCreatures; //active creatures are creatures that need updating, alive inactive creatures are held by a pregnant creature
	//stats of active creatures in list
	float averageMass;
	float averageTreeLength;
	float averageEnergy;
	float averageCarnivorism;
	float averageMutationRate;
	float averageDecisionsBeforeActions;
	int ageOfOldest;
	int averageAge;
	int highestGen;
	int lowestGen;
};

#endif
