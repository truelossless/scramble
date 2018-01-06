#include <SFML/Audio.hpp>
#include <SFML/Graphics.hpp>
#include <iostream>
#include <vector>
#include <ctime>
#include <cstdlib>
#include <memory>

#include "constants.h"
#include "fileManager.h"

#include "Game.h"
#include "RetryScreen.h"

#include "Mountain.h"
#include "Ship.h"
#include "ParticleSystem.h"
#include "shakeView.h"

int main() {
	// create the game window (fullscreen mode)
	sf::RenderWindow window(sf::VideoMode(), "Test", sf::Style::Fullscreen);
	window.setMouseCursorVisible(false);
	window.setFramerateLimit(60);

	bool run = true;

	while (run) {

		// create the game
		std::unique_ptr<Game> game(std::make_unique<Game>(window));

		int gameState = 0;

		// retry screen (created later)
		std::unique_ptr<RetryScreen> retryScreen;

		// run the game until the player restart a new game or the window is closed
		while (window.isOpen() && gameState != 1) {

			// clear the display
			window.clear(sf::Color::Black);

			// 0: update the game
			if (gameState == 0) {
				gameState = game->update(true);
			}
			// -1: gameover is triggered
			else if (gameState == -1) {
				retryScreen = std::make_unique<RetryScreen>(window);
				game->update(false);
				gameState = -2;
			}
			// -2: retry screen
			else if (gameState == -2) {
				game->update(false);
				gameState = retryScreen->update(true);
			}
			// -3: exit
			else if (gameState == -3) {
				return 0;
			}

			// draw calls are achieved by game classes, just refresh the window
			window.display();

		}

	}


	return 0;

}