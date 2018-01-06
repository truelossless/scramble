#pragma once

#include <SFML\Graphics.hpp>

class RetryScreen
{
public:
	RetryScreen(sf::RenderWindow& window);
	~RetryScreen();
	int update(bool focus);

private:
	sf::Font m_kindlyRewind;
	sf::Text m_gameOverText;
	sf::RenderWindow& m_window;
	sf::Clock m_animTimer;

	sf::Text m_retryText;
	sf::Text m_quitText;
	
	sf::CircleShape m_selector;
	sf::Clock m_selectorCoolDown;
	bool m_up = false;
	bool m_down = false;
	bool restartChoice = true;

	sf::RectangleShape m_background;

	float m_animPercent = 0;

};

