#include "stdafx.h"
#include "Game.h"

using namespace std;
//as these variables are static they need to be instanced manually
Game::GameState Game::_gameState = Uninitialized;
sf::RenderWindow Game::_mainWindow;
sf::View Game::_view;
tgui::Gui Game::gui{ Game::_mainWindow }; // Create the gui and attach it to the window
sf::Texture Game::texture;
sf::Sprite Game::sprite;
sf::Uint8* Game::pixels;
int Game::width = NULL;
int Game::height = NULL;
int Game::seed = static_cast <unsigned> (time(0));
ResourceMap* Game::resourceMap = NULL;
CreatureMap* Game::creatureMap = NULL;
CreatureList* Game::creatureList = NULL;
int Game::ticksCount = 0;
bool Game::enableScreenInteraction = true;
int Game::speedFactor = 25;
tgui::Label::Ptr Game::displayedStats = NULL;
clock_t Game::runSpeedLimiter;
clock_t Game::statTimer;
int Game::mouseX = -1;
int Game::mouseY = -1;
float* Game::selectedSpecies = NULL;
bool Game::selectSpecies = false;

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
	_gameState = Game::Paused;
	_view.reset(sf::FloatRect(0, 0, width, height));
	_mainWindow.setView(_view);

	GameInit();
	GUISetup();

	runSpeedLimiter = clock();
	statTimer = clock();
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
	clock_t startTime1 = clock();
	clock_t startTime2 = clock();
	
	//update the game state
	if (_gameState == Game::Running && clock() - runSpeedLimiter  > 20*(25 - speedFactor)) {
		ticksCount++;
		runSpeedLimiter = clock();

		startTime1 = clock();
		resourceMap->update();
		startTime2 = clock();
		creatureList->update();
		
		if (clock() - statTimer > 500){
			string statsText = "Update Times "
				"\nResources:     " + to_string(startTime2 - startTime1) +
				"\nCreatures:     " + to_string(clock() - startTime2) +
				"\nPool size:     " + to_string(creatureList->getLengthOfList()) +
				"\nTotal Ticks:   " + to_string(ticksCount) +
				"\n\n\nSpecies: "
				"\nNum of Creatures: " + to_string(creatureList->getNumOfActiveCreatures()) +
				"\nAge of oldest:    " + to_string(creatureList->getAgeOfOldest()) +
				"\nAverage age:      " + to_string(creatureList->getAverageAge()) +
				"\nHighest gen:      " + to_string(creatureList->getHighestGen()) +
				"\nLowest gen:       " + to_string(creatureList->getLowestGen()) +
				"\nAverage mass:     " + to_string((int)creatureList->getAverageMass()) +
				"\n% herbivores:     " + to_string(100) +
				"\nAv tree length:   " + to_string((int)creatureList->getAverageTreeLength()) +
				"\nAv energy:        " + to_string((int)creatureList->getAverageEnergy()) +
				"\nDecs b\\ actions:  " + to_string((int)creatureList->getAverageDecisionsBeforeActions());
			displayedStats->setText(statsText);
			creatureList->collectCreatureStats();
			statTimer = clock();
		}
		cout << "UPDATE:  Resources: " << startTime2 - startTime1 << "  Creatures: " << clock() - startTime2 << endl;
		//Update the screen
		startTime1 = clock();
		float maxEnergy = resourceMap->getMaxEnergy();
		for (int y = 0; y < height; y++)
		{
			for (int x = 0; x < width; x++)
			{
				if (creatureMap->getCell(x, y) == NULL) {
					pixels[(y * width + x) * 4] = 0; // R
					pixels[(y * width + x) * 4 + 1] = resourceMap->getCell(x, y) * 255 / maxEnergy; // G
					pixels[(y * width + x) * 4 + 2] = 0; // B
					if (selectedSpecies == NULL) {
						pixels[(y * width + x) * 4 + 3] = 255; // A
					}
					else {
						pixels[(y * width + x) * 4 + 3] = 125; // A
					}
				}
				else { //if (creatureMap->getCell(x, y) != NULL) {
					if (selectedSpecies != NULL && creatureMap->getCell(x, y)->isSameSpecies(selectedSpecies)) {
						pixels[(y * width + x) * 4] = creatureMap->getCell(x, y)->getCreatureID()[0] + 255; // R
						pixels[(y * width + x) * 4 + 1] = creatureMap->getCell(x, y)->getCreatureID()[1] + 50; // G
						pixels[(y * width + x) * 4 + 2] = creatureMap->getCell(x, y)->getCreatureID()[2]; // B
						pixels[(y * width + x) * 4 + 3] = 255; // A
					}
					else {
						pixels[(y * width + x) * 4] = creatureMap->getCell(x, y)->getCreatureID()[0]; // R
						pixels[(y * width + x) * 4 + 1] = creatureMap->getCell(x, y)->getCreatureID()[1]; // G
						pixels[(y * width + x) * 4 + 2] = creatureMap->getCell(x, y)->getCreatureID()[2]; // B
						pixels[(y * width + x) * 4 + 3] = 255; // A
					}
				}
			}
		}
		//startTime1 = clock();
		texture.update(pixels);
	}

	_mainWindow.clear();
	_mainWindow.draw(sprite);
	if (_gameState == Game::Running)
		cout << "   screen: " << clock() - startTime1  << endl;
	gui.draw(); // Draw all widgets
	_mainWindow.display();

	while (_mainWindow.pollEvent(currentEvent)){
		switch (_gameState){
			case Game::Running:
				if (currentEvent.type == sf::Event::KeyReleased && enableScreenInteraction){
					if (currentEvent.key.code == 'r' - 'a') {
						_gameState = Game::Paused;
					}
				}
				break;
			case Game::Paused:
				if (currentEvent.type == sf::Event::KeyReleased && enableScreenInteraction) {
					if (currentEvent.key.code == 'r' - 'a') {
						_gameState = Game::Running;
					}
				}
		}
		if (currentEvent.type == sf::Event::Closed)
		{
			_gameState = Game::Exiting;
		}
		else if (currentEvent.type == sf::Event::KeyPressed && enableScreenInteraction) {
			//key.code 56 is '-'
			if (currentEvent.key.code == 56) {
				zoomView(-1);
			}
			//key.code 55 is '+'
			else if (currentEvent.key.code == 55) {
				zoomView(1);
			}
		}
		// catch the resize events
		else if (currentEvent.type == sf::Event::Resized)
		{
			// update the view to the new size of the window
			_view.setSize(sf::Vector2f(currentEvent.size.width, currentEvent.size.height)); //currentEvent.size.width * height / width));// 
			_mainWindow.setView(_view);
		}
		else if (currentEvent.type == sf::Event::MouseWheelScrolled) {
			zoomView(currentEvent.mouseWheelScroll.delta);
		}
		else if (currentEvent.type == sf::Event::MouseButtonPressed) {
			if (selectSpecies) {
				//mouse click will give from top left of screen
				// center of view is coordinates of map at screen center
				//need to switch to view coordinates and adjust for center
				int mapX = (float)(currentEvent.mouseButton.x - ((float)_mainWindow.getSize().x / 2.0f)) * (float)_view.getSize().x / (float)_mainWindow.getSize().x + (float)_view.getCenter().x;
				int mapY = (float)(currentEvent.mouseButton.y - ((float)_mainWindow.getSize().y / 2.0f))  * (float)_view.getSize().y / (float)_mainWindow.getSize().y + (float)_view.getCenter().y;
				if (creatureMap->getCell(mapX, mapY)) {
					if (selectedSpecies != NULL) delete[] selectedSpecies;
					selectedSpecies = new float[4];
					selectedSpecies[0] = creatureMap->getCell(mapX, mapY)->isCarnivore();
					selectedSpecies[1] = creatureMap->getCell(mapX, mapY)->getMaxMass();
					selectedSpecies[2] = creatureMap->getCell(mapX, mapY)->getEnergyThreshold();
					selectedSpecies[3] = creatureMap->getCell(mapX, mapY)->getGrowthRate();
				}
				else {
					if (selectedSpecies != NULL) {
						delete[] selectedSpecies;
						selectedSpecies = NULL;
					}
				}
				selectSpecies = false;
			}
			else {
				mouseX = currentEvent.mouseButton.x;
				mouseY = currentEvent.mouseButton.y;
			}
		}
		else if (currentEvent.type == sf::Event::MouseButtonReleased) {
			mouseX = -1;
			mouseY = -1;
		}
		if (currentEvent.type == sf::Event::MouseMoved && mouseX >= 0 && enableScreenInteraction) {
			double newX = _view.getCenter().x + (mouseX - currentEvent.mouseMove.x) * _view.getSize().x / _mainWindow.getSize().x;
			double newY = _view.getCenter().y + (mouseY - currentEvent.mouseMove.y) * _view.getSize().y / _mainWindow.getSize().y;
			mouseX = currentEvent.mouseMove.x;
			mouseY = currentEvent.mouseMove.y;
			_view.setCenter(newX, newY);
			_mainWindow.setView(_view);
		}

		gui.handleEvent(currentEvent); // Pass the event to the widgets
	}
}

void Game::saveCreatures(tgui::TextBox::Ptr pathTextBox, tgui::TextBox::Ptr filenameTextBox) {
	string path = pathTextBox->getText();
	string filename = filenameTextBox->getText();
	//output to file
	ofstream outputFile;
	outputFile.open(path + filename);
	if (outputFile.is_open()) {
		Creature** aliveCreatures = NULL;
		int numAliveCreatures = creatureList->getAliveCreatures(aliveCreatures);
		for (int i = 0; i < numAliveCreatures; i++) {
			if (aliveCreatures[i]->isActive()) {
				outputFile << aliveCreatures[i]->getDecisionTreeLength() << " ";
				int* decisionTree = aliveCreatures[i]->getDecisionTree();
				int treeLength = aliveCreatures[i]->getDecisionTreeLength();
				for (int j = 0; j < treeLength; j++) {
					outputFile << decisionTree[j] << " ";
				}
				outputFile << aliveCreatures[i]->isCarnivore() << " "
					<< aliveCreatures[i]->getMaxMass() << " "
					<< aliveCreatures[i]->getEnergyThreshold() << " "
					<< aliveCreatures[i]->getGrowthRate() << " "
					<< aliveCreatures[i]->getNumOffspringRange() << " "
					<< aliveCreatures[i]->getNumOffspringMedian() << " "
					<< aliveCreatures[i]->getLengthOfPregnancy() << endl;
			}
		}
		outputFile.close();
		cout << "Creatures successfully saved to: " << filename << endl;
	}
	else {
		cout << "Error - output file could not be opened. Creatures not saved." << endl;
	}
}

void Game::loadCreatures(tgui::TextBox::Ptr pathTextBox, tgui::TextBox::Ptr filenameTextBox) {
	string path = pathTextBox->getText();
	string filename = filenameTextBox->getText();
	cout << path << " " << filename << endl;
	//read from file
	ifstream inputFile;
	inputFile.open(path + filename);
	if (inputFile.is_open()) {
		string line;
		while (getline(inputFile, line))
		{
			istringstream iss(line);
			int* decisionTree;
			int decisionTreeLength, numOffspringRange, numOffspringMedian, lengthOfPregnancy;
			bool carnivore;
			float maxMass, mass, energyThreshold, growthRate;

			iss >> decisionTreeLength;
			decisionTree = new int[decisionTreeLength];
			for (int i = 0; i < decisionTreeLength; i++) {
				iss >> decisionTree[i];
			}
			if (!(iss >> carnivore >> maxMass >> energyThreshold >> growthRate >> numOffspringRange
				>> numOffspringMedian >> lengthOfPregnancy)) {
				//indicates an error here, not sure how to deal with it
				cout << "sstream input error" << endl;
			}
			mass = 0.5f * maxMass;

			Creature* creatureFromFile = creatureList->getPoolCreature();

			creatureFromFile->setCreatureAttributes(decisionTree, decisionTreeLength, carnivore, maxMass, mass, 0.0f,
				energyThreshold, growthRate, numOffspringRange, numOffspringMedian, lengthOfPregnancy, 0);
			int x, y;
			do {
				x = 1 + xor128() % (width - 1);
				y = 1 + xor128() % (height - 1);
			} while (!creatureMap->addCreature(creatureFromFile, x, y));
			creatureFromFile->born(x, y);
		}
		inputFile.close();
		cout << "Creatures successfully loaded from: " << filename << endl;
	}
	else {
		cout << "Error - input file could not be opened. Creatures not loaded." << endl;
	}
}

void Game::restartSimulation(tgui::TextBox::Ptr seedBox){
	string seedString = string(seedBox->getText());
	//remove any non digits from the string
	size_t charToRemove = seedString.find_first_not_of("0123456789");
	while (charToRemove != string::npos) {
		seedString.erase(charToRemove, 1);
		charToRemove = seedString.find_first_not_of("0123456789");
	}
	//make sure the value contained in the string isn't larger than signed int max
	if (seedString.size() > 10) {
		seedString = seedString.substr(0, 10);
	}
	if (seedString[0] > '1') {
		seedString[0] = '1';
	}
	//Update the GUI with the processed seed
	seedBox->setText(seedString);
	seed = stoi(seedString);
	//clean up last run
	cout << "deleting resource map" << endl;
	delete resourceMap;
	cout << "deleting creature map" << endl;
	delete creatureMap;
	cout << "deleting creature list" << endl;
	//TODO don't delete the creaturelist pool all the craetures but also change game init so it doesnt create a new list and cause mem leak
	//delete creatureList;
	cout << "deleted all" << endl;
	GameInit();
}

void Game::zoomView(int numOfTicks) {
	double aspectRatio = _view.getSize().y / _view.getSize().x;
	double newX = _view.getSize().x - 0.1 * numOfTicks * width;
	double newY = newX * aspectRatio;
	if (newX > 5 && newY > 5 && newX <= 2 * _mainWindow.getSize().x && newY <= 2 * _mainWindow.getSize().y) {
		_view.setSize(newX, newY);
		_mainWindow.setView(_view);
	}
}

void Game::GameInit(){
	/*
	Need to update the game init. when game starts need to seed the random number generatorto provide the random seed which is then used to 
	reseed the RNG to give a random spread of random seeds. the rest of this function shouldn't be run until the start button is pressed.
	extract resourcemap, creature map and creature list object creation and change constructors to create empty objects that can be then reset
	with a method so as to not fragment the memory on restart.
	*/


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
	srand(seed);
	initialiseXorState();
	//srand(1234567890);

	resourceMap = new ResourceMap(width, height, 1.0f);
	creatureMap = new CreatureMap(width, height);
	creatureList = new CreatureList(2000);
	//pass the maps to the creature class as static pointers so creatures can interact with them
	Creature::resourceMap = resourceMap;
	Creature::creatureMap = creatureMap;
	Creature::creatureList = creatureList;
	
	for (int i = 0; i < numHerbivoreSpecies; i++){
		//creates full random creature to act as the template for a species
		//it is passed as a variable to the newly created creatures in the  
		Creature* speciesBase = new Creature(false);

		cout << speciesBase->getMaxMass() << endl
			<< speciesBase->getGrowthRate() << endl
			<< speciesBase->getEnergyThreshold() << endl
			<< speciesBase->getNumOffspringRange() << endl
			<< speciesBase->getNumOffspringMedian() << endl
			<< speciesBase->getLengthOfPregnancy() << endl
			<< speciesBase->getCreatureID()[0] << " " << speciesBase->getCreatureID()[1] << " " << speciesBase->getCreatureID()[2] << endl;
		
		//want to create x num creatures, want to have max 1 creature to 9 cells
		//find a random spot on the map with a border of 50 pixels around the edge
		int speciesSpawnX = width / 2;//50 + rand() % (width - 100);
		int speciesSpawnY = height / 2;//50 + rand() % (height - 100);
		for (int x = speciesSpawnX - 150; x < speciesSpawnX + 150; x++){
			for (int y = speciesSpawnY - 100; y < speciesSpawnY + 100; y++){
				//with a 1/14 chance put a creature in the cell if the cell is free
				if (xor128() % 30 < 1 && creatureMap->getCell(x, y) == NULL){
					//create a creature in the same species as the randomised one
					Creature* creature = creatureList->getPoolCreature();
					creature->setCreatureAttributes(speciesBase);
					//add the creature to the map
					creatureMap->addCreature(creature, x, y);
					creature->born(x, y);
					// add creature to the list
				}
			}
		}
		delete speciesBase;
	}
	for (int i = 0; i < numCarnivoreSpecies; i++) {
		//creates full random creature to act as the template for a species
		//it is passed as a variable to the newly created creatures in the  
		Creature* speciesBase = new Creature(true);

		cout << speciesBase->getMaxMass() << endl
			<< speciesBase->getGrowthRate() << endl
			<< speciesBase->getEnergyThreshold() << endl
			<< speciesBase->getNumOffspringRange() << endl
			<< speciesBase->getNumOffspringMedian() << endl
			<< speciesBase->getLengthOfPregnancy() << endl
			<< speciesBase->getCreatureID()[0] << " " << speciesBase->getCreatureID()[1] << " " << speciesBase->getCreatureID()[2] << endl;

		//want to create x num creatures, want to have max 1 creature to 9 cells
		//find a random spot on the map with a border of 50 pixels around the edge
		int speciesSpawnX = width / 2;//50 + rand() % (width - 100);
		int speciesSpawnY = height / 2;//50 + rand() % (height - 100);
		for (int x = speciesSpawnX - 250; x < speciesSpawnX + 250; x++) {
			for (int y = speciesSpawnY - 140; y < speciesSpawnY + 140; y++) {
				//with a 1/14 chance put a creature in the cell if the cell is free
				if (xor128() % 60 < 1 && creatureMap->getCell(x, y) == NULL) {
					//create a creature in the same species as the randomised one
					Creature* creature = creatureList->getPoolCreature();
					creature->setCreatureAttributes(speciesBase);
					//add the creature to the map
					creatureMap->addCreature(creature, x, y);
					creature->born(x, y);
					// add creature to the list
				}
			}
		}
		delete speciesBase;
	}
}

void Game::toggleShowMenu(tgui::Gui& gui, tgui::Tab::Ptr tabs, int buttonPos) {
	sf::View currentView = _mainWindow.getView();
	if (gui.get("tabs")->isVisible()) {
		gui.get("tabs")->hide();
		gui.get("controlPanel")->hide();
		gui.get("infoPanel")->hide();
		gui.get("menuButton")->setPosition(0, 0);
		_view.setCenter(sf::Vector2f(_view.getCenter().x + _view.getSize().x * 0.137, _view.getCenter().y));
		_mainWindow.setView(_view);
	}
	else {
		gui.get("tabs")->show();
		if (tabs->getSelected() == "Controls") {
			gui.get("controlPanel")->show();
			gui.get("infoPanel")->hide();
		}
		else {
			gui.get("controlPanel")->hide();
			gui.get("infoPanel")->show();
		}
		gui.get("menuButton")->setPosition(buttonPos, 0);
		_view.setCenter(sf::Vector2f(_view.getCenter().x - _view.getSize().x * 0.137, _view.getCenter().y));
		_mainWindow.setView(_view);
	}
}

void Game::startStopSimulation(tgui::Button::Ptr startStopButton) {
	if (_gameState == Game::Paused) {
		_gameState = Game::Running;
		startStopButton->setText("Stop");
	}
	else {
		_gameState = Game::Paused;
		startStopButton->setText("Start");
	}
}

void onTabSelected(tgui::Gui& gui, std::string selectedTab){
	// Show the correct panel
	if (selectedTab == "Controls")
	{
		gui.get("controlPanel")->show();
		gui.get("infoPanel")->hide();
	}
	else if (selectedTab == "Creature Info")
	{
		gui.get("controlPanel")->hide();
		gui.get("infoPanel")->show();
	}
}

void Game::GUISetup() {
	int panelWidth = 274;
	int panelHeight = 1000;

	gui.setFont("fonts/consolas.ttf");

	tgui::Tab::Ptr tabs = tgui::Tab::create();
	tabs->add("Controls");
	tabs->add("Creature Info");
	tabs->setPosition(0, 0);
	gui.add(tabs, "tabs");
	// Enable callback when another tab is selected (pass reference to the gui as first parameter)
	tabs->connect("TabSelected", onTabSelected, std::ref(gui));

	auto menuButton = tgui::Button::create();
	menuButton->setPosition(0, 0);
	menuButton->setSize(25, 25);
	gui.add(menuButton, "menuButton");
	menuButton->connect("pressed", toggleShowMenu, std::ref(gui), tabs, panelWidth + 2);

	// Create the first panel
	tgui::Panel::Ptr panel1 = tgui::Panel::create();
	panel1->setSize(panelWidth, panelHeight);
	panel1->setPosition(tabs->getPosition().x, tabs->getPosition().y + tabs->getTabHeight());
	gui.add(panel1, "controlPanel");

	// Create the second panel (by copying of first one)
	tgui::Panel::Ptr panel2 = tgui::Panel::copy(panel1);
	gui.add(panel2, "infoPanel");

	// Add some widgets to the panels
	/*
	start/stop
	save creatures
	load creatures
	Console?
	speed up slow down slider
	zoom slider??
	button to track species
	button to show decision tree in new window
	track creature?? have clicked creature leave a trail or center screen on creature and follow

	stats
	update times creatures and resources
	size of pool

	stats for all or for clicked species
	Species: 
	age of oldest: 
	highest and lowest generations: 
	average mass: 
	percentage herbivores?: 
	average tree length: 
	average energy: 
	decisions before actions: 
	*/

	//Controls panel widgets
	//Start stop button
	auto startStopButton = tgui::Button::create();
	startStopButton->setPosition(100, 100);
	startStopButton->setSize(80, 80);
	startStopButton->setText("Start");
	panel1->add(startStopButton, "startStopButton");
	startStopButton->connect("pressed", startStopSimulation, startStopButton);
	
	//Save button, file path and file name
	auto saveButton = tgui::Button::create();
	saveButton->setPosition(10, 200);
	saveButton->setSize(80, 80);
	saveButton->setText("   Save\nCreatures");
	panel1->add(saveButton, "saveButton");
	tgui::TextBox::Ptr savePath = tgui::TextBox::create();
	savePath->setPosition(100, 200);
	savePath->setSize(140, 40);
	savePath->setText("/path/to/file");
	panel1->add(savePath, "savePath");
	savePath->connect("Focused Unfocused", [&]() { enableScreenInteraction = !enableScreenInteraction; });
	tgui::TextBox::Ptr saveName = tgui::TextBox::create();
	saveName->setPosition(100, 240);
	saveName->setSize(140, 40);
	saveName->setText("fileName.txt");
	panel1->add(saveName, "saveName");
	saveName->connect("Focused Unfocused", [&]() { enableScreenInteraction = !enableScreenInteraction; });

	//load button, file path and file name
	auto loadButton = tgui::Button::create();
	loadButton->setPosition(10, 300);
	loadButton->setSize(80, 80);
	loadButton->setText("   Load\nCreatures");
	panel1->add(loadButton, "loadButton");
	tgui::TextBox::Ptr loadPath = tgui::TextBox::create();
	loadPath->setPosition(100, 300);
	loadPath->setSize(140, 40);
	loadPath->setText("/path/to/file");
	panel1->add(loadPath, "loadPath");
	loadPath->connect("Focused Unfocused", [&]() { enableScreenInteraction = !enableScreenInteraction; });
	tgui::TextBox::Ptr loadName = tgui::TextBox::create();
	loadName->setPosition(100, 340);
	loadName->setSize(140, 40);
	loadName->setText("fileName.txt");
	panel1->add(loadName, "loadName");
	loadName->connect("Focused Unfocused", [&]() { enableScreenInteraction = !enableScreenInteraction; });
	loadButton->connect("pressed", loadCreatures, loadPath, loadName);

	//speed up slow down slider
	tgui::Label::Ptr speedSliderLabel = tgui::Label::create();
	speedSliderLabel->setPosition(70, 400);
	speedSliderLabel->setText("Speed Slider");
	panel1->add(speedSliderLabel, "speedSliderLabel");
	tgui::Slider::Ptr speedSlider = tgui::Slider::create();
	speedSlider->setPosition(20, 450);
	speedSlider->setSize(210, 15);
	speedSlider->setMinimum(1);
	speedSlider->setMaximum(25);
	speedSlider->setValue(25);
	panel1->add(speedSlider, "speedSlider");
	speedSlider->connect("ValueChanged", [&](int value) { speedFactor = value; });
	speedSlider->connect("MouseEntered  MouseLeft", [&]() { enableScreenInteraction = !enableScreenInteraction; });

	//zoom slider
	/*tgui::Label::Ptr zoomSliderLabel = tgui::Label::create();
	zoomSliderLabel->setPosition(100, 500);
	zoomSliderLabel->setText("Zoom");
	panel1->add(zoomSliderLabel, "zoomSliderLabel");
	tgui::Slider::Ptr zoomSlider = tgui::Slider::create();
	zoomSlider->setPosition(20, 550);
	zoomSlider->setSize(210, 15);
	zoomSlider->setMinimum(0);
	zoomSlider->setMaximum(10);
	panel1->add(zoomSlider, "zoomSlider");
	zoomSlider->connect("Focused Unfocused", [&]() { enableScreenInteraction = !enableScreenInteraction; });*/

	//Select Species button
	auto selectSpeciesButton = tgui::Button::create();
	selectSpeciesButton->setPosition(80, 500);
	selectSpeciesButton->setSize(120, 80);
	selectSpeciesButton->setText("Select Species");
	panel1->add(selectSpeciesButton, "selectSpeciesButton");
	selectSpeciesButton->connect("pressed", [&]() { selectSpecies = true; });
	
	//Restart button and start options
	auto restartButton = tgui::Button::create();
	restartButton->setPosition(100, 600);
	restartButton->setSize(80, 40);
	restartButton->setText("Restart");
	panel1->add(restartButton, "restartButton");

	tgui::TextBox::Ptr seedBox = tgui::TextBox::create();
	seedBox->setPosition(100, 650);
	seedBox->setSize(140, 40);
	seedBox->setText(to_string(seed));
	panel1->add(seedBox, "seedBox");
	//TODO on init get seed from time, put in box, when start take seed from box incase changed
	//add options for num herb species and starting grass cover etc

	//todo add check box for adding herbivores percentage grass cover
	seedBox->connect("Focused Unfocused", [&]() { enableScreenInteraction = !enableScreenInteraction; });
	restartButton->connect("pressed", restartSimulation, seedBox);


	//Creature info Panel widgets
	string statsText = "Update Times "
		"\nResources:     0"
		"\nCreatures:     0"
		"\nPool size:     0" 
		"\nTotal Ticks:   0"
		"\n\n\nSpecies: "
		"\nNum of Creatures: 0"
		"\nAge of oldest:    0"
		"\nAverage age:      0"
		"\nHighest gen:      0" 
		"\nLowest gen:       0" 
		"\nAverage mass:     0" 
		"\n% herbivores:     0" 
		"\nAv tree length:   0" 
		"\nAv energy:        0" 
		"\nDecs b\\ actions:  0" ;
	tgui::Label::Ptr mainStats = tgui::Label::create();
	mainStats->setPosition(10, 10);
	mainStats->setText(statsText);
	displayedStats = mainStats;
	panel2->add(mainStats, "mainStats");

	// Select the first tab and only show the first panel
	tabs->select("Controls");
	tabs->hide();
	panel1->hide();
	panel2->hide();
}