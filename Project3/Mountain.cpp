#include <random>
#include <iostream>
#include <SFML/Graphics.hpp>
#include "fileManager.h"
#include "Mountain.h"


Mountain::Mountain(float x, float y) :
	randomGenerator((std::random_device()())) {

	m_x = x;
	m_y = y;

	// genère une montagne de dimension et forme aléatoire
	m_shape.setPointCount(4);
	m_shape.setPoint(0, sf::Vector2f(m_x, m_y));

	std::uniform_int_distribution<short> patternRange(0, 6);
	short randomPattern = patternRange(randomGenerator);

	std::uniform_real_distribution<float> distPointX(m_x + 100, m_x + 400);

	float inf = m_y - 300;
	if (inf < 1080 / 2) inf = 1080 / 2;
	float sup = m_y + 300;
	if (sup > 980) sup = 980;
	std::uniform_real_distribution<float> distPointY(inf, sup);

	m_lx = distPointX(randomGenerator);

	// faire un plat (une chance sur 7)
	if (randomPattern == 6) {
		m_ly = m_y;
		// spawn d'un tank de fuel si il y a assez de place
		m_fuelTank.setTexture(FileManager::getTexture("resources/img/sprite/fueltank.png"));

		sf::FloatRect fuelTankHitbox = m_fuelTank.getLocalBounds();
		if ((m_lx - m_x) > fuelTankHitbox.width) {
			m_fuelTank.setPosition(m_x + (m_lx - m_x) / 2 - m_fuelTank.getLocalBounds().width / 2, m_ly - m_fuelTank.getLocalBounds().height);
			m_flat = true;
		}

	}
	// faire une montée (3 chances sur 7)
	else if (randomPattern <= 2) {
		m_ly = distPointY(randomGenerator);
	}
	// faire une descente (3 chances sur 7)
	else {
		m_ly = distPointY(randomGenerator);
	}



	m_shape.setPoint(1, sf::Vector2f(m_lx, m_ly));

	// POUR PLUS TARD: A CHANGER AVEC LA CONSTANTE
	m_shape.setPoint(2, sf::Vector2f(m_lx, m_ly + (980 - m_ly)));
	m_shape.setPoint(3, sf::Vector2f(m_x, m_y + (980 - m_y)));

	//m_shape.setOutlineColor(sf::Color(255, 0, 0));
	//m_shape.setOutlineThickness(10);
	m_shape.setFillColor(sf::Color(100, 250, 50));
}

sf::ConvexShape& Mountain::getShape() {
	return m_shape;
}

sf::Sprite& Mountain::getFuelTankSprite() {
	return m_fuelTank;
}

float Mountain::getX() {
	return m_x;
}

float Mountain::getY() {
	return m_y;
}

float Mountain::getLX() {
	return m_lx;
}

float Mountain::getLY() {
	return m_ly;
}

bool Mountain::isFlat() const {
	return m_flat;
}

bool Mountain::isFuelTankDestroyed() const {
	return m_fuelTankDestroyed;
}

bool Mountain::checkForCollision(sf::Sprite ship) {

	// si la montagne est plate un réservoir peut dépasser de sa hitbox
	if (m_flat && !m_fuelTankDestroyed) {
		if (ship.getGlobalBounds().intersects(m_fuelTank.getGlobalBounds())) {
			return true;
		}
	}

	if (m_shape.getGlobalBounds().intersects(ship.getGlobalBounds())) {
		// les hitboxes on l'air d'être en collision
		
		// si le terrain est plat pas besoin d'aller plus loin
		if (m_flat) {
			return true;
		}

		// on utilise le SAT

		// points de la montagne
		sf::Vector2f mountainPoints[4];
		for (short i = 0; i < 4; i++) {
			mountainPoints[i] = m_shape.getPoint(i);
		}

		// points du sprite (on fait une collision gentille)
		sf::Vector2f rectPoints[4];
		sf::Transform transform = ship.getTransform();
		sf::FloatRect local = ship.getLocalBounds();
		rectPoints[0] = sf::Vector2f(transform.transformPoint(10, 10));
		rectPoints[1] = sf::Vector2f(transform.transformPoint(local.width - 10, 10));
		rectPoints[2] = sf::Vector2f(transform.transformPoint(local.width - 10, local.height - 10));
		rectPoints[3] = sf::Vector2f(transform.transformPoint(10, local.height - 10));

		// on récupère les axes de la montagne
		sf::Vector2f mountainAxis[4];
		for (short i = 0; i < 4; i++) {
			sf::Vector2f p1 = mountainPoints[i]; // point 1
			sf::Vector2f p2 = mountainPoints[i + 1 > 4 ? 0 : i + 1]; // point 2
			sf::Vector2f edge = p1 - p2; // vecteur du côté
			mountainAxis[i] = sf::Vector2f(-edge.y, edge.x); // vecteur perpendiculaire
		}

		// on récupère les axes du sprite
		sf::Vector2f rectAxis[4];
		for (short i = 0; i < 4; i++) {
			sf::Vector2f p1 = rectPoints[i]; // point 1
			sf::Vector2f p2 = rectPoints[i + 1 > 4 ? 0 : i + 1]; // point 2
			sf::Vector2f edge = p1 - p2; // vecteur du côté
			rectAxis[i] = sf::Vector2f(-edge.y, edge.x); // vecteur perpendiculaire
		}

		// on parcourt les axes de la montagne et on regarde si les projections se chevauchent
		for (short i = 0; i < 4; i++) {

			// on fait la projection de la montagne selon l'axe de la montagne
			float minMountain = mountainAxis[i].x * mountainPoints[0].x + mountainAxis[i].y * mountainPoints[0].y;
			float maxMountain = minMountain;

			for (short j = 1; j < 4; j++) {
				float projection = mountainAxis[i].x * mountainPoints[j].x + mountainAxis[i].y * mountainPoints[j].y;
				if (projection < minMountain) minMountain = projection;
				if (projection > maxMountain) maxMountain = projection;
			}

			// on fait la projection du rectangle selon l'axe de la montagne
			float minRect = mountainAxis[i].x * rectPoints[0].x + mountainAxis[i].y * rectPoints[0].y;
			float maxRect = minRect;

			for (short j = 1; j < 4; j++) {
				float projection = mountainAxis[i].x * rectPoints[j].x + mountainAxis[i].y * rectPoints[j].y;
				if (projection < minRect) minRect = projection;
				if (projection > maxRect) maxRect = projection;
			}

			if ((minMountain > maxRect) || (minRect > maxMountain)) {
				// les projections ne se chevauchent pas, donc pas de collision
				return false;
			}

		}

		// on parcourt les axes du rectangle et on regarde si les projections se chevauchent
		for (short i = 0; i < 4; i++) {

			// on fait la projection de la montagne selon l'axe du rectancle
			float minMountain = rectAxis[i].x * mountainPoints[0].x + rectAxis[i].y * mountainPoints[0].y;
			float maxMountain = minMountain;

			for (short j = 1; j < 4; j++) {
				float projection = rectAxis[i].x * mountainPoints[j].x + rectAxis[i].y * mountainPoints[j].y;
				if (projection < minMountain) minMountain = projection;
				if (projection > maxMountain) maxMountain = projection;
			}

			// on fait la projection du rectangle selon l'axe du rectangle
			float minRect = rectAxis[i].x * rectPoints[0].x + rectAxis[i].y * rectPoints[0].y;
			float maxRect = minRect;

			for (short j = 1; j < 4; j++) {
				float projection = rectAxis[i].x * rectPoints[j].x + rectAxis[i].y * rectPoints[j].y;
				if (projection < minRect) minRect = projection;
				if (projection > maxRect) maxRect = projection;
			}

			if ((minMountain > maxRect) || (minRect > maxMountain)) {
				// les projections ne se chevauchent pas, donc pas de collision
				return false;
			}

		}

		// si toutes les projections se chevauchent, il y a une collision
		return true;

	}
	else {
		// une collsion n'est pas possible, les AABB ne sont pas en collsion
		return false;
	}

}

void Mountain::destroyFuelTank() {
	m_fuelTankDestroyed = true;
	FileManager::getSound("resources/sound/explosion.wav").play();
}

void Mountain::drawProps(sf::RenderTarget& render) {
	if (!m_fuelTankDestroyed) {
		render.draw(m_fuelTank);
	}
}

Mountain::~Mountain()
{
}
