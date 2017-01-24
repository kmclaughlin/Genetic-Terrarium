#ifndef CREATUREMAP_H
#define CREATUREMAP_H
//cannot have creature and creatureMap include each other, but can forward declare creature here and only use pointers to creaturemap
class Creature;

class CreatureMap {
public:
	//constructor destructor
	CreatureMap(int screenX, int screenY);
	~CreatureMap();

	Creature* getCell(int x, int y);
	bool addCreature(Creature* creature, int x, int y);
	bool moveCreature(int currentX, int currentY, char dir);
	void removeCreature(int x, int y);
private:
	Creature** map;
	int width, height;
	//don't need a buffer, allow for constant change, not single state change.
	//may need a buffer for parrallelisation
	//Creature** bufferMap;
};
#endif
