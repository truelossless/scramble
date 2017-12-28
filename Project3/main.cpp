#include <SFML/Audio.hpp>
#include <SFML/Graphics.hpp>
#include <iostream>
#include <vector>
#include <ctime>
#include <cstdlib>

#include "fileManager.h"
#include "shakeView.h"
#include "ParticleSystem.h"
#include "Mountain.h"
#include "Ship.h"

int main() {
	// fenêtre principale
	sf::RenderWindow window(sf::VideoMode(), "Test", sf::Style::Fullscreen);
	window.setMouseCursorVisible(false);
	window.setFramerateLimit(60);

	// constantes et variables de jeu
	const int screenWidth = window.getSize().x;
	const int screenHeight = window.getSize().y;
	float gameSpeed = 1;
	bool gameOver = false;

	// canvas de rendu sur lequel sera appliqué les shaders
	sf::RenderTexture render;
	if (!render.create(screenWidth, screenHeight)) return -1;

	sf::View view = render.getView();
	sf::View defaultView = view;

	// horloges
	sf::Clock renderClock; // rendu à une framerate donnée
	sf::Clock speedClock; // changement de vitesse du vaisseau
	sf::Clock laserClock; // vitesse de tir du vaisseau

	// initialisation des sprites

	Ship ship("resources/img/sprite/ship.png");
	ship.setPosition(sf::Vector2f((screenWidth - ship.getSize().x) / 2, (screenHeight - ship.getSize().y) / 3));

	// variables pour les évenements clavier
	bool moveLeft = false, moveRight = false, moveUp = false, moveDown = false;

	// tableau contenant les lasers lancés par le vaisseau
	std::vector<sf::Sprite> lasers;

	//tableau contenant les shapes pour faire les montagnes
	std::vector<Mountain> mountains;
	mountains.push_back(Mountain(-screenWidth, screenHeight - 400));

	// sol
	sf::RectangleShape ground(sf::Vector2f(screenWidth, 100));
	ground.setPosition(0, screenHeight - 100);
	ground.setFillColor(sf::Color(100, 250, 50));

	// tableau contenant les explosions en cours
	std::vector<ParticleSystem> explosions;

	// shader de pixelisation
	sf::Shader pixelate;
	if (!pixelate.loadFromFile("resources/shader/pixelate.frag", sf::Shader::Fragment)) return -1;

	// chargement des sons principaux en cache
	FileManager::getSound("resources/sound/crash.wav");
	FileManager::getSound("resources/sound/laser.wav");

	// boucle principale
	while (window.isOpen()) {
		sf::Event event;
		while (window.pollEvent(event)) {
			if (event.type == sf::Event::Closed) {
				window.close();
			}
			else if (event.type == sf::Event::KeyPressed) {
				if (event.key.code == sf::Keyboard::Left) moveLeft = true;
				else if (event.key.code == sf::Keyboard::Right)	moveRight = true;
				else if (event.key.code == sf::Keyboard::Up) moveUp = true;
				else if (event.key.code == sf::Keyboard::Down) moveDown = true;
			}
			else if (event.type == sf::Event::KeyReleased) {
				if (event.key.code == sf::Keyboard::Left) moveLeft = false;
				else if (event.key.code == sf::Keyboard::Right) moveRight = false;
				else if (event.key.code == sf::Keyboard::Up) moveUp = false;
				else if (event.key.code == sf::Keyboard::Down) moveDown = false;
			}
		}

		// incrémentation de la vitesse avec un maximum de 6 fois la vitesse initiale toutes les dixièmes de seconde
		if (speedClock.getElapsedTime() > sf::milliseconds(100)) {

			if (gameSpeed < 6) {
				gameSpeed += 0.005;

				speedClock.restart();
			}
		}

		// gestion du tir du vaisseau
		if (laserClock.getElapsedTime() > sf::milliseconds(333) && sf::Keyboard::isKeyPressed(sf::Keyboard::Space) && !gameOver) { // 3 tirs par seconde
			sf::Sprite laserSprite(FileManager::getTexture("resources/img/sprite/laser.png"));
			laserSprite.setPosition(ship.getPosition().x + ship.getSize().x, ship.getPosition().y + ship.getSize().y / 3);
			lasers.push_back(laserSprite);

			FileManager::getSound("resources/sound/laser.wav").play();

			laserClock.restart();
		}

		// rafraichissmenet de l'affichage
		sf::Time elapsed = renderClock.getElapsedTime();
		renderClock.restart();

		// scrolling horizontal
		if (!gameOver) {
			view.move(sf::Vector2f(100 * gameSpeed * elapsed.asSeconds(), 0));
		}

		// mouvement du vaisseau
		float displacementX = 0;
		float displacementY = 0;
		if (moveLeft) displacementX -= 1000;
		if (moveRight) displacementX += 1000;
		if (moveUp) displacementY -= 1000;
		if (moveDown) displacementY += 1000;

		if (!gameOver) {
			ship.move(sf::Vector2f(displacementX * elapsed.asSeconds(), displacementY * elapsed.asSeconds()));
			ship.move(sf::Vector2f(100 * gameSpeed * elapsed.asSeconds(), 0));

		}

		ground.setPosition(render.mapPixelToCoords(sf::Vector2i(0, screenHeight - 100)));
		
		// collisions avec l'écran
		sf::Vector2i shipCoordinates = render.mapCoordsToPixel(sf::Vector2f(ship.getPosition().x, ship.getPosition().y));

		// collision haut
		if (shipCoordinates.y < 0) {
			sf::Vector2f newShipWorldCoordinates = render.mapPixelToCoords(sf::Vector2i(0, 0));
			newShipWorldCoordinates.x = ship.getPosition().x;
			ship.setPosition(newShipWorldCoordinates);
		}

		// la collision avec le bas n'est pas possible grâce aux montagnes

		// collision gauche
		if (shipCoordinates.x < 0) {
			sf::Vector2f newShipWorldCoordinates = render.mapPixelToCoords(sf::Vector2i(0, 0));
			newShipWorldCoordinates.y = ship.getPosition().y;
			ship.setPosition(newShipWorldCoordinates);
		}

		// collision droite
		else if (shipCoordinates.x + ship.getSize().x > screenWidth) {
			sf::Vector2f newShipWorldCoordinates = render.mapPixelToCoords(sf::Vector2i(screenWidth - ship.getSize().x, 0));
			newShipWorldCoordinates.y = ship.getPosition().y;
			ship.setPosition(newShipWorldCoordinates);
		}

		// collision avec les montagnes
		for (short i = 0; i < mountains.size(); i++) {
			if (mountains[i].checkForCollision(ship.getSprite())) {
				sf::Vector2f emitPos = sf::Vector2f(ship.getPosition().x + ship.getSize().x / 2, ship.getPosition().y + ship.getSize().y / 2);
				if (!gameOver) {
					ParticleSystem explosion(300, emitPos, sf::seconds(0.4), sf::seconds(1));
					explosion.setSpeed(400);
					explosion.setRandomFade(sf::seconds(0.3));
					explosion.setParticleSize(40);
					explosion.setStartColor(sf::Color(255, 0, 0));
					explosion.setEndColor(sf::Color(255, 255, 0));
					explosions.push_back(explosion);

					FileManager::getSound("resources/sound/crash.wav").play();
					gameOver = true;
				}
			}
		}

		// dessin des éléments
		render.clear(sf::Color::Black);
		window.clear(sf::Color::Black);

		// dessin des montagnes
		sf::Vector2i mountainCoordinates = render.mapCoordsToPixel(sf::Vector2f(mountains.back().getLX(), 0));

		while (mountainCoordinates.x < 2 * screenWidth) {
			mountains.push_back(Mountain(mountains.back().getLX(), mountains.back().getLY()));

			mountainCoordinates = render.mapCoordsToPixel(sf::Vector2f(mountains.back().getLX(), 0));
		}

		for (short i = mountains.size() - 1; i >= 0; i--) {
			sf::Vector2i mountainCoordinates = render.mapCoordsToPixel(sf::Vector2f(mountains[i].getLX(), 0));
			if (mountainCoordinates.x < -screenWidth) {
				mountains.erase(mountains.begin() + i);
			}

			if (mountains[i].isFlat()) mountains[i].drawProps(render);
			render.draw(mountains[i].getShape());
		}

		// dessin des missiles
		for (short i = lasers.size() - 1; i >= 0; i--) {
			lasers[i].move(2000 * elapsed.asSeconds(), 0);

			bool laserCollided = false;

			// collision avec les réservoirs de fuel
			for (short j = 0; j < mountains.size(); j++) {
				if (!mountains[j].isFuelTankDestroyed() && lasers[i].getGlobalBounds().intersects(mountains[j].getFuelTankSprite().getGlobalBounds())) {
					laserCollided = true;
					int index = explosions.size();
					explosions.push_back(ParticleSystem(200, lasers[i].getPosition(), sf::seconds(0.5), sf::seconds(1)));
					mountains[j].destroyFuelTank();

					break;
				}

			}
			
			// on supprime le laser s'il y a eu une collision
			if (laserCollided) {
				lasers.erase(lasers.begin() + i);
				continue;
			}

			// on supprime le laser s'il sort de l'écran
			sf::Vector2i laserCoordinates = render.mapCoordsToPixel(sf::Vector2f(lasers[i].getGlobalBounds().left, 0));
			if (laserCoordinates.x > screenWidth) {
				lasers.erase(lasers.begin() + i);
				continue;
			}
			
			
			render.draw(lasers[i]);

		}

		// dessin du sol
		render.draw(ground);

		// dessin du vaisseau
		if (!gameOver) {
			render.draw(ship.getSprite());
		}

		// dessin des explosions

		int biggestExplosionShake = 0;

		for (short i = explosions.size() - 1; i >= 0; i--) {
			explosions[i].update(elapsed);
			if (explosions[i].isDone()) {
				explosions.erase(explosions.begin() + i);
			}
			else {
				int explosionShake = explosions[i].getState() * 50;
				if (explosionShake > biggestExplosionShake) biggestExplosionShake = explosionShake;
				render.draw(explosions[i]);
			}
		}

		// effet de tremblement
		std::cout << biggestExplosionShake << std::endl;
		shakeView(view, biggestExplosionShake);

		// mise à jour de la vue
		render.setView(view);

		render.display();
		sf::Sprite renderSprite(render.getTexture());

		window.draw(renderSprite, &pixelate);
		window.display();
	}

	return 0;

}