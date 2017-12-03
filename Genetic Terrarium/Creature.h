#ifndef CREATURE_H
#define CREATURE_H
#include "WorldMap.h"
#include "Xorshift.h"

class CreatureList;

//can creature remember the results of its last food/predator/friendly surroundings check so it can increase efficiency by calling them every other tick, or longer?

class Creature{
public:
	//constructor destructor
	Creature(bool carnivore);
	~Creature();

	void setCreatureAttributes(int* tree, int treeLength, bool carnivore, float maxMass, float mass, float energy, float energyThreshold, float growthRate,
		int numOffspringRange, int numOffspringMedian, int lengthOfPregnancy, int _generation);
	void setCreatureAttributes(Creature* creature);
	void born(int x, int y);

	bool update();
	bool isActive() { return active; };
	float kill(); 
	bool isAlive() { return alive; };
	/*add iscarnivore check and carnivore bool to craeture as a var, or decide how herbs and carns differ and built the separate classes
		finish off the creature lookaround and foodcompare functions may also consider moving without eating, need move actions and eat actions,
		could help with large herbs eating more of the plant than smaller herbs*/

	int* getDecisionTree() { return decisionTree; };
	int getDecisionTreeLength() { return decisionTreeLength; };
	bool isCarnivore() { return carnivore; };
	float getMaxMass() { return maxMass; };
	float getEnergy() { return energy; };
	float getEnergyThreshold() { return energyThreshold; };
	float getGrowthRate() { return growthRate; };
	int getNumOffspringRange() { return numOffspringRange; };
	int getNumOffspringMedian() { return numOffspringMedian; };
	int getLengthOfPregnancy() { return lengthOfPregnancy; };
	int* getCreatureID() { return creatureID; };
	int getDecisionsBeforeAction() { return decisionsBeforeAction; };
	int getAge() { return age; };
	int getGeneration() { return generation; };
	bool isSameSpecies(float* statsToCheck);
	float getTotalEnergy();

	static WorldMap* worldMap;
	static CreatureList* creatureList;

protected:
	int decisionsBeforeAction;
	bool active;
	//the maximum time that the creature can spend deciding what to do this tick, when max time is reached the creature does nothing but next tick continues where
	//it left off in the decision process
	const int MAX_TIME = 1; //time in milliseconds
	const float MAX_DEFENSE = 1.0;
	const float START_ENERGY_PERCENTAGE = 0.75f / 10.0f; //maybe 
	const float MOVEMENT_COST_CONST = 0.005f;
	const float MAX_ENERGY_CONST = 0.75f *10.0f;
	const float ENERGY_TO_MASS_CONST = 0.5;
	//These may be needed to set random values
	const float MAX_MAX_MASS = 60.0f;
	const float MAX_GROWTH_RATE = 0.15f;
	const int MAX_NUM_OFFSPRING_RANGE = 2;
	const int MAX_NUM_OFFSPRING_MEDIAN = 1;
	const int MAX_LENGTH_OF_PREGNANCY = 10;
	//TODO - make this a creature specific variable?
	const float MUTATION_RATE = 0.8f;// 0.2f;
	const float CHANCE_OF_DEATH = 0;// .000005f;
	//variables used to help define the creatures behaviour
	//need to lose energy every turn based on mass regardless of action, then lose mass based on action ie movement.
	//need a no action action.
	bool alive;
	float energy;
	float expendedEnergy;
	float maxEnergy; //dependant on mass
	float energyThreshold; //value that tree can make decisions on compared to current energy
	//maxMass dictates movement energy cost and defense score, higher mass takes longer to get to maturity, takes energy to grow
	//can have longer pregnancy to have children closer to maturity at birth but takes more energy from mother to grow them all
	bool carnivore;
	float maxMass;
	//could have body mass and energy mass, energy mass fluctuates with energy level, body mass is not a function of energy, though it determines
	// max energy
	int ticksSinceShit;
	float mass;
	float movementCost; //increse exponentially with mass
	float defense; //decreases exponentially with mass
	int speed;//?? how can this be implemented? no speed attribute, larger tree takes more cycles to run so acts as a speed meter
	int age;//?? update ticks survived?
	int generation; // number of ancestors
	bool mature; //maturity reached at 80% max mass, when mature can breed
	int numOffspringRange;
	int numOffspringMedian;
	int lengthOfPregnancy;
	bool pregnant;
	bool inHeat;
	int mapX, mapY;
	float growthRate; //grow every round if not max mass, growth rate is the percentage of (mass - maxMass) gained ie will grow fast when small but slow off as older
	Creature* baby[1];
	int creatureID[3];

	// functional variables
	float plantsNearby[4];
	float carcassNearby[4];
	int carnivoreNearby[4];
	int friendlyNearby[4];
	int herbivoreNearby[4];

	int plantsRank[4];
	int carnivoreRank[4];
	int friendlyRank[4];
	int herbivoreRank[4];
	int carcassRank[4];
	// fights nearby
	// carcasses nearby
	// injured nearby
	// young nearby
	// defence of creatures?
	//array of all the possible decision ids that could be made
	const static int NUM_OF_DECISIONS;
	const static int NUM_OF_ACTIONS;
	int* decisionTree = NULL;
	int decisionTreeLength;
	int currentTreeNodeStart;
	int currentTreeNodeEnd;
	//multiple functions will use the results of these checks, but only have to calc once per tick
	bool lookedAround;

	bool actionTaken;

	void takeAction();
	void updateCreatureVariables();

	void lookAround();
	void addToNearby(int index, int* weights, int weightsIndex, MapCell mapCell);
	bool compare(char dir, int rank, char toCompare);
	
	void nextTreeNode(bool lastDecision);
	bool isOutcomeAction(bool decision);

	void randomTree(int length);
	void checkVariablesWithinBounds();
	void setCreatureID();

	void generateOffspringDecisionTree(int* &babyDecisionTree, int &babyTreeLength, int* mateDecisionTree, int* mateTreeLength);
	void pregnancyCheck();
	void replicate();
	/*
	replicate
	create offspring

	in heat

	*/

};

#endif
