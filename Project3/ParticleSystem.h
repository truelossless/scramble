#pragma once
#include <SFML/Graphics.hpp>
#include <random>
#include <vector>

// adapté d'un exemple de la bibliothèque SFML
class ParticleSystem : public sf::Drawable, public sf::Transformable
{
public:
	ParticleSystem(unsigned int count, sf::Vector2f pos, sf::Time particleLifeTime, sf::Time SystemLifetime);
	void update(sf::Time time);
	void setEmitter(sf::Vector2f pos);
	void setSpawnRate(unsigned int count);
	void setParticleLifeTime(sf::Time lifetime);
	void setParticleSize(int size);
	void setRandomFade(sf::Time timeDelta);
	void setSpeed(unsigned int speed);
	void setStartColor(sf::Color color);
	void setEndColor(sf::Color color);
	float getState() const;
	bool isDone() const;

private:
	struct Particle {
		sf::Vector2f velocity;
		sf::Time lifetime;
		sf::Time originalLifetime;
	};

	static std::mt19937 randomGenerator;

	std::vector<sf::Vertex> m_vertices;
	std::vector<Particle> m_particles;
	sf::Vector2f m_emitter;
	sf::Time m_originalLifetime;
	sf::Time m_lifetime;
	sf::Time m_particleLifetime;
	sf::Time m_randomFade;
	sf::Color m_startColor;
	sf::Color m_endColor;

	bool m_hasFinished = false;
	int m_spawnRate;
	int m_speed;
	int m_size = 10;
	float m_tmpParticle = 0;

	void addParticles(float count);
	virtual void draw(sf::RenderTarget& target, sf::RenderStates states) const;
};

