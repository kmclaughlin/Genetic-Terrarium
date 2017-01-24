#include "stdafx.h"
#include "Game.h"

using namespace std;
//as these variables are static they need to be instanced manually
Game::GameState Game::_gameState = Uninitialized;
sf::RenderWindow Game::_mainWindow;
sf::Texture Game::texture;
sf::Sprite Game::sprite;
sf::Uint8* Game::pixels;
int Game::width = NULL;
int Game::height = NULL;
ResourceMap* Game::resourceMap = NULL;
CreatureMap* Game::creatureMap = NULL;
CreatureList* Game::creatureList = NULL;

void Game::Start(int screenX, int screenY){
	/*start() should only be run once when the program is launched, therefore we
	initialise _gameState to uninitialized then use that as a check to make sure
	this only runs once. Could add an error output here to catch and report if this
	is ever run more than once rather than just returning
	*/
	if (_gameState != Uninitialized){
		return;
	}
	
	width = screenX;
	height = screenY;
	
	_mainWindow.create(sf::VideoMode(width, height, 32), "Genetic Terrarium");
	pixels = new sf::Uint8[width * height * 4];
	texture.create(width, height);
	sprite.setTexture(texture);
	_gameState = Game::Running;
	
	GameInit();

	while (!isExiting())
	{
		GameLoop();
	}

	_mainWindow.close();
}

bool Game::isExiting(){
	if (_gameState == Game::Exiting)
		return true;
	else
		return false;
}

void Game::GameLoop(){
	sf::Event currentEvent;
	clock_t startTime = clock();
	while (_mainWindow.pollEvent(currentEvent)){
		//cout << rand() << endl;
		switch (_gameState){
			case Game::Running:
				for (int y = 0; y < height; y++)
				{
					for (int x = 0; x < width; x++)
				{
						if (creatureMap->getCell(x, y) == NULL) {
							pixels[(y * width + x) * 4] = 0; // R
							pixels[(y * width + x) * 4 + 1] = resourceMap->getCell(x, y) * 255 / resourceMap->getMaxEnergy(); // G
							pixels[(y * width + x) * 4 + 2] = 0; // B
							pixels[(y * width + x) * 4 + 3] = 255; // A
						}
						else { //if (creatureMap->getCell(x, y) != NULL) {
							pixels[(y * width + x) * 4] = creatureMap->getCell(x, y)->getCreatureID()[0]; // R
							pixels[(y * width + x) * 4 + 1] = creatureMap->getCell(x, y)->getCreatureID()[1]; // G
							pixels[(y * width + x) * 4 + 2] = creatureMap->getCell(x, y)->getCreatureID()[2]; // B
							pixels[(y * width + x) * 4 + 3] = 255; // A
						}
					}
				}
				

				texture.update(pixels);
				_mainWindow.draw(sprite);
				_mainWindow.display();
				//cout << "screen: " << clock() - startTime << endl;
				startTime = clock();
				resourceMap->update();
				creatureList->update();
				cout << "update: " << clock() - startTime << endl;
				if (currentEvent.type == sf::Event::Closed)
				{
					_gameState = Game::Exiting;
				}
				else if (currentEvent.key.code == 'a') {
					cout << "AAA" << endl;
				}
				break;
		}
	}
}

void Game::GameInit(){
	/*
	gnenerate creatures and populate creature list and creature map
		where does this happen?? create creature, set tree to null and call random tree on it?
		- creatures need decision trees and base values and to be grouped in species 
		- initialiser given number of species to create at launch, selects a random point on the creature map and randomly creates 
		creatures of a single species with a specied density in an area around that point.
		- how to create species of creatures? random decision trees for all? and random initial variables for first but each after gets values with variations on that
		- species will have to be primed in a safe zone then saved and loaded into main
	initialise resource map (change to have start in boon times, ie 1 in 3 cells mature
	resource map and creature map need to be created and be vairables of main game file then passed by reference down to what ever needs them below
	*/
	int numHerbivoreSpecies = 1;
	int numCarnivoreSpecies = 0;
	
	//seed the random number generator
	srand(static_cast <unsigned> (time(0)));
	//srand(1234567890);
	
	resourceMap = new ResourceMap(width, height, 1.0f);
	creatureMap = new CreatureMap(width, height);
	creatureList = new CreatureList();
	//pass the maps to the creature class as static pointers so creatures can interact with them
	Creature::resourceMap = resourceMap;
	Creature::creatureMap = creatureMap;
	Creature::creatureList = creatureList;
	
	for (int i = 0; i < numHerbivoreSpecies; i++){
		//creates full random creature to act as the template for a species
		//it is passed as a variable to the newly created creatures in the  
		Creature* speciesBase = new Creature(false);
		
		//want to create x num creatures, want to have max 1 creature to 9 cells
		//find a random spot on the map with a border of 50 pixels around the edge
		int speciesSpawnX = width / 2;//50 + rand() % (width - 100);
		int speciesSpawnY = height / 2;//50 + rand() % (height - 100);
		for (int x = speciesSpawnX - 50; x < speciesSpawnX + 50; x++){
			for (int y = speciesSpawnY - 50; y < speciesSpawnY + 50; y++){
				//with a 1/14 chance put a creature in the cell if the cell is free
				if (rand() % 30 < 1 && creatureMap->getCell(x, y) == NULL){
					//create a creature in the same species as the randomised one
					Creature* creature = new Creature(speciesBase, x, y);
					//add the creature to the map
					creatureMap->addCreature(creature, x, y);
					// add creature to the list
					creatureList->addCreature(creature);
				}
			}
		}
	}
}