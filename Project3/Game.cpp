#include <SFML/Audio.hpp>
#include <SFML/Graphics.hpp>
#include <iostream>
#include <vector>
#include <ctime>
#include <cstdlib>

#include "constants.h"
#include "fileManager.h"
#include "Game.h"
#include "ParticleSystem.h"
#include "Mountain.h"
#include "Ship.h"
#include "shakeView.h"



Game::Game(sf::RenderWindow& window) :
	m_window(window),
	m_ship("resources/img/sprite/ship.png"),
	m_shipSmoke(30, sf::Vector2f(0, 0), sf::seconds(1), sf::seconds(4))
{
	m_gameSpeed = 1;
	m_gameOver = false;

	// canvas de rendu sur lequel sera appliqué les shaders
	m_render.create(screenWidth, screenHeight);

	m_view = m_render.getView();

	// initialisation des sprites
	m_ship.setPosition(sf::Vector2f((screenWidth - m_ship.getSize().x) / 2, (screenHeight - m_ship.getSize().y) / 3));

	// variables pour les évenements clavier
	bool m_mLeft = false, m_mRight = false, m_mUp = false, m_mDown = false;

	// ajout de la première montagne
	m_mountains.push_back(Mountain(-screenWidth, screenHeight - 400));

	// sol
	m_ground.setSize(sf::Vector2f(screenWidth, 100));
	m_ground.setPosition(0, screenHeight - 100);
	m_ground.setFillColor(sf::Color(100, 250, 50));

	// fumée si le vaisseau se met à manquer de fuel
	m_shipSmoke.setStartColor(sf::Color(117, 117, 117));
	m_shipSmoke.setEndColor(sf::Color(255, 255, 255));
	m_shipSmoke.setSpeed(50);
	m_shipSmoke.setParticleSize(30);

	// shader de pixelisation
	m_pixelate.loadFromFile("resources/shader/pixelate.frag", sf::Shader::Fragment);

	// chargement de la police
	m_kindlyRewind.loadFromFile("resources/font/kindlyrewind.ttf");

	// création du texte
	m_fuelString.setFont(m_kindlyRewind);
	m_fuelString.setStyle(sf::Text::Bold);
	m_fuelString.setString("FUEL");
	m_fuelString.setFillColor(sf::Color::Red);
	m_fuelString.setCharacterSize(48);
	m_fuelString.setPosition(12, screenHeight - 60);

	// jauge de fuel
	m_fuelMeter.setSize(sf::Vector2f(300, 40));
	m_emptyFuelMeter.setSize(sf::Vector2f(300, 40));
	m_fuelMeter.setPosition(180, screenHeight - 52);
	m_emptyFuelMeter.setPosition(180, screenHeight - 52);
	m_fuelMeter.setFillColor(sf::Color(43, 71, 196));
	m_emptyFuelMeter.setFillColor(sf::Color::Red);

	m_fuelDrain = 0;
	m_noFuel = false;
}

int Game::update(bool focus) {

	// logique du jeu

	// gestion des inputs

	if (focus) {

		sf::Event event;

		while (m_window.pollEvent(event)) {
			// joystick
			if (sf::Joystick::isConnected(0)) {
				if (sf::Joystick::getAxisPosition(0, sf::Joystick::X) < -30) m_mLeft = true;
				else if (sf::Joystick::getAxisPosition(0, sf::Joystick::X) > 30) m_mRight = true;
				else {
					m_mLeft = false;
					m_mRight = false;
				}

				if (sf::Joystick::getAxisPosition(0, sf::Joystick::Y) < -30) m_mUp = true;
				else if (sf::Joystick::getAxisPosition(0, sf::Joystick::Y) > 30) m_mDown = true;
				else {
					m_mDown = false;
					m_mUp = false;
				}
			}

			if (event.type == sf::Event::Closed) {
				m_window.close();
			}
			else if (event.type == sf::Event::KeyPressed) {
				if (event.key.code == sf::Keyboard::Left) m_mLeft = true;
				else if (event.key.code == sf::Keyboard::Right)	m_mRight = true;
				else if (event.key.code == sf::Keyboard::Up) m_mUp = true;
				else if (event.key.code == sf::Keyboard::Down) m_mDown = true;
			}
			else if (event.type == sf::Event::KeyReleased) {
				if (event.key.code == sf::Keyboard::Left) m_mLeft = false;
				else if (event.key.code == sf::Keyboard::Right) m_mRight = false;
				else if (event.key.code == sf::Keyboard::Up) m_mUp = false;
				else if (event.key.code == sf::Keyboard::Down) m_mDown = false;
			}

		}

	}

	// incrémentation de la vitesse avec un maximum de 6 fois la vitesse initiale toutes les dixièmes de seconde
	if (m_speedClock.getElapsedTime() > sf::milliseconds(100)) {

		if (m_gameSpeed < 6) {
			m_gameSpeed += 0.005;

			m_speedClock.restart();
		}
	}

	// gestion du tir du vaisseau
	if (m_laserClock.getElapsedTime() > sf::milliseconds(333) && !m_gameOver) { // 3 tirs par seconde
		if ((sf::Joystick::isConnected(0) && sf::Joystick::isButtonPressed(0, 0))
			|| sf::Keyboard::isKeyPressed(sf::Keyboard::Space)) {

			sf::Sprite laserSprite(FileManager::getTexture("resources/img/sprite/laser.png"));
			laserSprite.setPosition(m_ship.getPosition().x + m_ship.getSize().x, m_ship.getPosition().y + m_ship.getSize().y / 3);
			m_lasers.push_back(laserSprite);

			FileManager::getSound("resources/sound/laser.wav").play();

			m_laserClock.restart();
		}

	}

	// rafraichissmenet de l'affichage
	sf::Time elapsed = m_renderClock.getElapsedTime();
	m_renderClock.restart();

	// draînage de la jauge de fuel
	if (!m_gameOver && !m_noFuel) {
		m_fuelDrain += elapsed.asSeconds() * 10;

		if (m_fuelDrain >= 300) {
			m_noFuel = true;
			m_fuelDrain = 300;
		}

		m_fuelMeter.setSize(sf::Vector2f(300 - m_fuelDrain, 40));
	}

	// scrolling horizontal
	if (!m_gameOver) {
		m_view.move(sf::Vector2f(100 * m_gameSpeed * elapsed.asSeconds(), 0));
	}

	// mouvement du vaisseau
	float displacementX = 0;
	float displacementY = 0;

	if (!m_noFuel) {
		if (m_mLeft) displacementX -= 1000;
		if (m_mRight) displacementX += 1000;
		if (m_mUp) displacementY -= 1000;
		if (m_mDown) displacementY += 1000;
	}
	else if (!m_gameOver) {
		displacementY += 500;
	}

	if (!m_gameOver) {
		m_ship.move(sf::Vector2f(displacementX * elapsed.asSeconds(), displacementY * elapsed.asSeconds()));
		m_ship.move(sf::Vector2f(100 * m_gameSpeed * elapsed.asSeconds(), 0));

	}

	m_ground.setPosition(m_render.mapPixelToCoords(sf::Vector2i(0, screenHeight - 100)));

	// collisions avec l'écran
	sf::Vector2i shipCoordinates = m_render.mapCoordsToPixel(sf::Vector2f(m_ship.getPosition().x, m_ship.getPosition().y));

	// collision haut
	if (shipCoordinates.y < 0) {
		sf::Vector2f newShipWorldCoordinates = m_render.mapPixelToCoords(sf::Vector2i(0, 0));
		newShipWorldCoordinates.x = m_ship.getPosition().x;
		m_ship.setPosition(newShipWorldCoordinates);
	}

	// la collision avec le bas n'est pas possible grâce aux montagnes

	// collision gauche
	if (shipCoordinates.x < 0) {
		sf::Vector2f newShipWorldCoordinates = m_render.mapPixelToCoords(sf::Vector2i(0, 0));
		newShipWorldCoordinates.y = m_ship.getPosition().y;
		m_ship.setPosition(newShipWorldCoordinates);
	}

	// collision droite
	else if (shipCoordinates.x + m_ship.getSize().x > screenWidth) {
		sf::Vector2f newShipWorldCoordinates = m_render.mapPixelToCoords(sf::Vector2i(screenWidth - m_ship.getSize().x, 0));
		newShipWorldCoordinates.y = m_ship.getPosition().y;
		m_ship.setPosition(newShipWorldCoordinates);
	}

	// collision avec les montagnes
	for (short i = 0; i < m_mountains.size(); i++) {
		if (m_mountains[i].checkForCollision(m_ship.getSprite())) {
			sf::Vector2f emitPos = sf::Vector2f(m_ship.getPosition().x + m_ship.getSize().x / 2, m_ship.getPosition().y + m_ship.getSize().y / 2);
			if (!m_gameOver) {
				// on crée l'explosion
				ParticleSystem explosion(300, emitPos, sf::seconds(0.4), sf::seconds(1));
				explosion.setSpeed(400);
				explosion.setRandomFade(sf::seconds(0.3));
				explosion.setParticleSize(40);
				explosion.setStartColor(sf::Color(255, 0, 0));
				explosion.setEndColor(sf::Color(255, 255, 0));
				m_explosions.push_back(explosion);

				// on stoppe la fumée du vaisseau qui pourrait venir du manque de fuel
				m_shipSmoke.stop();

				FileManager::getSound("resources/sound/crash.wav").play();
				m_gameOver = true;
				m_gameOverClock.restart();
			}
		}
	}

	// dessin des éléments
	m_render.clear(sf::Color::Black);

	// dessin des montagnes
	sf::Vector2i mountainCoordinates = m_render.mapCoordsToPixel(sf::Vector2f(m_mountains.back().getLX(), 0));

	while (mountainCoordinates.x < 2 * screenWidth) {
		m_mountains.push_back(Mountain(m_mountains.back().getLX(), m_mountains.back().getLY()));

		mountainCoordinates = m_render.mapCoordsToPixel(sf::Vector2f(m_mountains.back().getLX(), 0));
	}

	for (short i = m_mountains.size() - 1; i >= 0; i--) {
		sf::Vector2i mountainCoordinates = m_render.mapCoordsToPixel(sf::Vector2f(m_mountains[i].getLX(), 0));
		if (mountainCoordinates.x < -screenWidth) {
			m_mountains.erase(m_mountains.begin() + i);
		}

		if (m_mountains[i].isFlat()) m_mountains[i].drawProps(m_render);
		m_render.draw(m_mountains[i].getShape());
	}

	// dessin des missiles
	for (short i = m_lasers.size() - 1; i >= 0; i--) {
		m_lasers[i].move(2000 * elapsed.asSeconds(), 0);

		bool laserCollided = false;

		// collision avec les réservoirs de fuel
		for (short j = 0; j < m_mountains.size(); j++) {
			if (!m_mountains[j].isFuelTankDestroyed() && m_lasers[i].getGlobalBounds().intersects(m_mountains[j].getFuelTankSprite().getGlobalBounds())) {
				laserCollided = true;

				// création de l'explosion
				m_explosions.push_back(ParticleSystem(200, m_lasers[i].getPosition(), sf::seconds(0.5), sf::seconds(1)));
				m_mountains[j].destroyFuelTank();

				// on rajoute l'essence
				m_fuelDrain -= 100;
				if (m_fuelDrain < 0) m_fuelDrain = 0;

				break;
			}

		}

		// on supprime le laser s'il y a eu une collision
		if (laserCollided) {
			m_lasers.erase(m_lasers.begin() + i);
			continue;
		}

		// on supprime le laser s'il sort de l'écran
		sf::Vector2i laserCoordinates = m_render.mapCoordsToPixel(sf::Vector2f(m_lasers[i].getGlobalBounds().left, 0));
		if (laserCoordinates.x > screenWidth) {
			m_lasers.erase(m_lasers.begin() + i);
			continue;
		}


		m_render.draw(m_lasers[i]);

	}

	// dessin du sol
	m_render.draw(m_ground);

	// dessin du vaisseau
	if (!m_gameOver) {
		m_render.draw(m_ship.getSprite());
	}

	// dessin de la fumée si le vaisseau n'a plus de fuel
	if (m_noFuel) {
		m_shipSmoke.setEmitter(sf::Vector2f(m_ship.getSprite().getGlobalBounds().left, m_ship.getSprite().getGlobalBounds().top + m_ship.getSprite().getLocalBounds().height / 2));
		m_shipSmoke.update(elapsed);
		m_render.draw(m_shipSmoke);
	}

	// dessin des explosions

	int biggestExplosionShake = 0;

	for (short i = m_explosions.size() - 1; i >= 0; i--) {
		m_explosions[i].update(elapsed);
		if (m_explosions[i].isDone()) {
			m_explosions.erase(m_explosions.begin() + i);
		}
		else {
			int explosionShake = m_explosions[i].getState() * 50;
			if (explosionShake > biggestExplosionShake) biggestExplosionShake = explosionShake;
			m_render.draw(m_explosions[i]);
		}
	}

	// effet de tremblement
	shakeView(m_view, biggestExplosionShake);

	// mise à jour de la vue
	m_render.setView(m_view);

	m_render.display();
	sf::Sprite renderSprite(m_render.getTexture());

	m_window.draw(renderSprite, &m_pixelate);

	// dessin de la GUI
	m_window.draw(m_fuelString);
	m_window.draw(m_emptyFuelMeter);
	m_window.draw(m_fuelMeter);


	// check to see if game is over

	// ship is crashed and two seconds have been spent
	if (m_gameOver && m_gameOverClock.getElapsedTime().asSeconds() > 2) return -1;
	// no, the game is still running
	else return 0;
}


Game::~Game()
{
}