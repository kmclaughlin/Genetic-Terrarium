#ifndef GAME_H
#define GAME_h

#include <string>
#include "SFML/Window.hpp"
#include "SFML/Graphics.hpp"
#include "TGUI/TGUI.hpp"
#include "Xorshift.h"
#include "Creature.h"
#include "ResourceMap.h"
#include "CreatureMap.h"
#include "CreatureList.h"

class Game{
public:
	static void Start(int screenX, int screenY);
private:
	static bool isExiting();
	static void GameLoop();
	static void GameInit();
	static void GUISetup();
	static void updateGUIStats(); 
	static void toggleShowMenu(tgui::Gui& gui, tgui::Tab::Ptr tabs, int buttonPos);
	static void saveCreatures(tgui::TextBox::Ptr path, tgui::TextBox::Ptr filename);
	static void loadCreatures(tgui::TextBox::Ptr path, tgui::TextBox::Ptr filename);
	static void zoomView(int numOfTicks);

	static void startStopSimulation(tgui::Button::Ptr startStopButton);

	enum GameState { Uninitialized, ShowingSplash, Paused,
		ShowingMenu, Running, Exiting };

	static GameState _gameState;
	static sf::RenderWindow _mainWindow;
	static sf::View _view;
	static tgui::Gui gui;
	static sf::Texture texture;
	static sf::Sprite sprite;
	static sf::Uint8* pixels;
	//TODO separate map height and width from window height and width?
	static int width;
	static int height;
	static ResourceMap* resourceMap;
	static CreatureMap* creatureMap;
	static CreatureList* creatureList;
	static int ticksCount;
	static bool enableScreenInteraction;
	static int speedFactor;
	static tgui::Label::Ptr displayedStats;
	static clock_t runSpeedLimiter;
	static clock_t statTimer;
	static int mouseX;
	static int mouseY;
	static Creature* selectedSpecies;
	static bool selectSpecies;
};

#endif
