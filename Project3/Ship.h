#pragma once
#include <SFML/Graphics.hpp>
#include "ParticleSystem.h"

class Ship
{
public:
	Ship(std::string texturePath);
	sf::Sprite& getSprite();
	sf::Vector2f getPosition() const;
	sf::Vector2f getSize() const;
	void setPosition(sf::Vector2f pos);
	void move(sf::Vector2f pos);

private:
	sf::Sprite m_sprite;
	sf::Texture& m_texture;
};

