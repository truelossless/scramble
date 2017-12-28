#include "Ship.h"
#include "fileManager.h"
#include "ParticleSystem.h"
#include <iostream>

Ship::Ship(std::string texturePath):
	m_texture(FileManager::getTexture(texturePath))
{
	m_sprite.setTexture(m_texture);
}

sf::Vector2f Ship::getPosition() const {
	return sf::Vector2f(m_sprite.getGlobalBounds().left, m_sprite.getGlobalBounds().top);
}

sf::Vector2f Ship::getSize() const {
	return sf::Vector2f(m_sprite.getLocalBounds().width, m_sprite.getLocalBounds().height);
}

sf::Sprite& Ship::getSprite() {
	return m_sprite;
}

void Ship::move(sf::Vector2f pos) {
	m_sprite.move(pos);
}

void Ship::setPosition(sf::Vector2f pos) {
	m_sprite.setPosition(pos);
}