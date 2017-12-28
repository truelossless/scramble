#include <random>
#include <iostream>
#include "ParticleSystem.h"

std::mt19937 ParticleSystem::randomGenerator((std::random_device()()));

ParticleSystem::ParticleSystem(unsigned int count, sf::Vector2f pos, sf::Time particleLifetime, sf::Time systemLifetime) :
	m_spawnRate(count),
	m_emitter(pos),
	m_lifetime(systemLifetime),
	m_originalLifetime(systemLifetime),
	m_particleLifetime(particleLifetime),
	m_speed(300),
	m_startColor(255, 255, 255),
	m_endColor(255, 255, 255)
{

}

void ParticleSystem::update(sf::Time time) {

	m_lifetime -= time;
	if (m_lifetime <= sf::Time::Zero && m_particles.size() == 0) {
		m_hasFinished = true;
		return;
	}

	if (m_lifetime > sf::Time::Zero) {
		addParticles(m_spawnRate * time.asSeconds());
	}

	for (int i = m_particles.size() - 1; i >= 0; i--) {
		m_particles[i].lifetime -= time;

		// on supprime la particule si elle est finie
		if (m_particles[i].lifetime <= sf::Time::Zero) {

			for (short j = 3; j >= 0; j--) {
				m_vertices.erase(m_vertices.begin() + (i * 4 + j));
			}

			m_particles.erase(m_particles.begin() + i);

			// on passe à la prochaine itération
			continue;
		}

		float ratio = m_particles[i].lifetime.asSeconds() / m_particles[i].originalLifetime.asSeconds();

		for (short j = 0; j < 4; j++) {
			m_vertices[i * 4 + j].position += m_particles[i].velocity * time.asSeconds();
			// on transforme la couleur de la particule
			m_vertices[i * 4 + j].color = sf::Color(m_startColor.r*ratio, m_startColor.g*ratio, m_startColor.b*ratio) + sf::Color(m_endColor.r * (1-ratio), m_endColor.g * (1 - ratio), m_endColor.b * (1 - ratio));
			m_vertices[i * 4 + j].color.a = static_cast<sf::Uint8>(ratio*255);
		}
	}
}

void ParticleSystem::addParticles(float count) {

	float add = std::floor(count);

	// dans le cas ou on ne doit pas ajouter une particule mais 1/x de particule
	m_tmpParticle += count - add;

	// il n'y a pas assez de particules à ajouter
	if (add < 1 && m_tmpParticle < 1) return;
	else if (m_tmpParticle >= 1) {
		add += 1;
		m_tmpParticle -= 1;
	}

	std::uniform_real_distribution<float> angleRange(0, 360);
	std::uniform_real_distribution<float> randomFadeRange(0, m_randomFade.asSeconds());

	// création des particules avec un angle aléatoire
	for (unsigned int i = 0; i < add; i++) {
		m_particles.push_back(Particle());

		Particle &particle = m_particles[m_particles.size() - 1];

		if (m_randomFade != sf::Time::Zero) {
			particle.lifetime = m_particleLifetime + sf::seconds(randomFadeRange(randomGenerator));
			particle.originalLifetime = particle.lifetime;
		}
		else {
			particle.lifetime = m_particleLifetime;
			particle.originalLifetime = m_particleLifetime;
		}


		float angle = angleRange(randomGenerator);
		particle.velocity = sf::Vector2f(std::cos(angle) * m_speed, std::sin(angle) * m_speed);

		// pour chaque arête du carré
		m_vertices.push_back(sf::Vertex(m_emitter));
		m_vertices.push_back(sf::Vertex(m_emitter + sf::Vector2f(m_size, 0)));
		m_vertices.push_back(sf::Vertex(m_emitter + sf::Vector2f(m_size, m_size)));
		m_vertices.push_back(sf::Vertex(m_emitter + sf::Vector2f(0, m_size)));
	}
}

void ParticleSystem::setEmitter(sf::Vector2f pos) {
	m_emitter = pos;
}

void ParticleSystem::setRandomFade(sf::Time timeDelta) {
	m_randomFade = timeDelta;
}

void ParticleSystem::setSpeed(unsigned int speed) {
	m_speed = speed;
}

void ParticleSystem::setParticleLifeTime(sf::Time lifetime) {
	m_particleLifetime = lifetime;
}

void ParticleSystem::setSpawnRate(unsigned int count) {
	m_spawnRate = count;
}

void ParticleSystem::setParticleSize(int size) {
	m_size = size;
}

void ParticleSystem::setStartColor(sf::Color color) {
	m_startColor = color;
}

void ParticleSystem::setEndColor(sf::Color color) {
	m_endColor = color;
}

void ParticleSystem::stop() {
	m_lifetime = sf::Time::Zero;
}

bool ParticleSystem::isDone() const {
	return m_hasFinished;
}

float ParticleSystem::getState() const {
	return  m_lifetime / m_originalLifetime;
}

void ParticleSystem::draw(sf::RenderTarget & target, sf::RenderStates states) const {
	states.transform *= getTransform();
	states.texture = NULL;

	target.draw(m_vertices.data(), m_vertices.size(), sf::Quads, states);
}