#include "shakeView.h"

sf::Clock shakeClock;
bool toggleShake = false;
int shakeOffset;

void shakeView(sf::View& view, int strength) {

	if (strength == 0) return;

	int elapsed = shakeClock.getElapsedTime().asMilliseconds();
	sf::Vector2f shake(shakeOffset * elapsed / 100, 0);
	view.setCenter(view.getCenter() + shake);

	if (elapsed >= 100) {
		toggleShake = !toggleShake;
		shakeClock.restart();
		toggleShake ? shakeOffset = -strength : shakeOffset = strength;
	}
}
