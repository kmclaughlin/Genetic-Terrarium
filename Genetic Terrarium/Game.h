#ifndef GAME_H
#define GAME_h
#include "SFML/Window.hpp"
#include "SFML/Graphics.hpp"
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

	enum GameState { Uninitialized, ShowingSplash, Paused,
		ShowingMenu, Running, Exiting };

	static GameState _gameState;
	static sf::RenderWindow _mainWindow;
	static sf::Texture texture;
	static sf::Sprite sprite;
	static sf::Uint8* pixels;
	static int width;
	static int height;
	static ResourceMap* resourceMap;
	static CreatureMap* creatureMap;
	static CreatureList* creatureList;
	static int loopCount;
};

#endif
