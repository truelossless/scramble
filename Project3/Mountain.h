#pragma once

#include <SFML/Graphics.hpp>
#include <random>

class Mountain
{
public:
	Mountain(float x, float y);
	sf::ConvexShape& getShape();
	float getX();
	float getY();
	float getLX();
	float getLY();
	sf::Sprite& getFuelTankSprite();
	bool checkForCollision(sf::Sprite ship);
	bool isFlat() const;
	bool isFuelTankDestroyed() const;
	void destroyFuelTank();
	void drawProps(sf::RenderTarget& render);
	~Mountain();

private:
	sf::ConvexShape m_shape;
	float m_x;
	float m_y;
	float m_lx;
	float m_ly;
	bool m_flat = false;
	bool m_fuelTankDestroyed = false;
	sf::Sprite m_fuelTank;

	std::mt19937 randomGenerator;
};

