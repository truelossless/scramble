#pragma once

#include <SFML/Graphics.hpp>

extern sf::Clock shakeClock;
extern bool toggleShake;
extern int shakeOffset;
void shakeView(sf::View& view, int strength);