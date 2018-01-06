#pragma once

#include <SFML/Graphics.hpp>
#include <vector>

#include "Ship.h"
#include "Mountain.h"

class Game
{
public:
	Game(sf::RenderWindow& window);
	int update(bool focus);
	~Game();

private:
	sf::RenderWindow& m_window;
	
	sf::RenderTexture m_render;
	sf::View m_view;

	// variables de jeu
	float m_gameSpeed;
	bool m_gameOver;

	// horloges
	sf::Clock m_renderClock; // rendu à une framerate donnée
	sf::Clock m_speedClock; // changement de vitesse du vaisseau
	sf::Clock m_laserClock; // vitesse de tir du vaisseau
	sf::Clock m_gameOverClock; // laisse deux secondes pour admirer l'explosion du vaisseau

	Ship m_ship;

	// variables pour le dépalcement du vaisseau
	bool m_mLeft, m_mRight, m_mUp, m_mDown;

	// tableau contenant les lasers lancés par le vaisseau
	std::vector<sf::Sprite> m_lasers;
	// tableau contenant toutes les montagnes
	std::vector<Mountain> m_mountains;
	// tableau contenant les explosions en cours
	std::vector<ParticleSystem> m_explosions;

	// sol
	sf::RectangleShape m_ground;

	// fumée qi sort du vaisseau lorsque celui-ci manque d'essence
	ParticleSystem m_shipSmoke;

	sf::Shader m_pixelate;

	// barre de fuel
	sf::Font m_kindlyRewind;
	sf::Text m_fuelString;
	sf::RectangleShape m_fuelMeter, m_emptyFuelMeter;
	float m_fuelDrain;
	bool m_noFuel;

};

