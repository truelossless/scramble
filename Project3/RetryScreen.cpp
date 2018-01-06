#include "RetryScreen.h"

#include <SFML\Graphics.hpp>

#include "constants.h"
#include <iostream>


RetryScreen::RetryScreen(sf::RenderWindow& window) :
	m_window(window)
{

	m_kindlyRewind.loadFromFile("resources/font/kindlyrewind.ttf");

	// game over text
	m_gameOverText.setFont(m_kindlyRewind);
	m_gameOverText.setString("Game Over");
	m_gameOverText.setCharacterSize(128);
	m_gameOverText.setFillColor(sf::Color::White);

	sf::FloatRect textRect = m_gameOverText.getLocalBounds();
	m_gameOverText.setOrigin(textRect.width / 2, textRect.height / 2);
	m_gameOverText.setPosition(screenWidth / 2, -200);

	// retry text
	m_retryText.setFont(m_kindlyRewind);
	m_retryText.setString("retry");
	m_retryText.setCharacterSize(48);
	m_retryText.setFillColor(sf::Color::White);

	textRect = m_retryText.getLocalBounds();
	m_retryText.setOrigin(textRect.width / 2, textRect.height / 2);
	m_retryText.setPosition(screenWidth + 500, 500);

	// quit text
	m_quitText.setFont(m_kindlyRewind);
	m_quitText.setString("quit");
	m_quitText.setCharacterSize(48);
	m_quitText.setFillColor(sf::Color::White);

	textRect = m_quitText.getLocalBounds();
	m_quitText.setOrigin(textRect.width / 2, textRect.height / 2);
	m_quitText.setPosition(-500, 600);

	// dark, semi-transparent background
	m_background.setSize(sf::Vector2f(screenWidth, screenHeight));
	m_background.setPosition(0, 0);
	m_background.setFillColor(sf::Color(0, 0, 0, 255));

	// dot choice selector
	m_selector.setRadius(10);
	m_selector.setPosition(screenWidth / 2 - 200, 500);

}

int RetryScreen::update(bool focus) {

	if (focus) {

		sf::Event event;
		while (m_window.pollEvent(event)) {

			if (sf::Joystick::isConnected(0)) {
				if (sf::Joystick::getAxisPosition(0, sf::Joystick::Y) < -30) m_up = true;
				else if (sf::Joystick::getAxisPosition(0, sf::Joystick::Y) > 30) m_down = true;
				else {
					m_up = false;
					m_down = false;
				}
			}

			if (event.type == sf::Event::Closed) {
				m_window.close();
			}
			else if (event.type == sf::Event::KeyPressed) {
				if (event.key.code == sf::Keyboard::Key::Up) m_up = true;
				else if (event.key.code == sf::Keyboard::Key::Down) m_down = true;
			}
			else if (event.type == sf::Event::KeyReleased) {
				if (event.key.code == sf::Keyboard::Key::Up) m_up = false;
				else if (event.key.code == sf::Keyboard::Key::Down) m_down = false;
			}
		}
	}

	float easedPercent;

	if (m_animPercent != 1) {
		m_animPercent = m_animTimer.getElapsedTime() / sf::seconds(1);

		if (m_animPercent >= 1) {
			m_animPercent = 1;
			m_selector.setFillColor(sf::Color::White);
		}

		// easing function: https://gist.github.com/gre/1650294
		easedPercent = m_animPercent < 0.5 ? 2 * m_animPercent*m_animPercent : -1 + (4 - 2 * m_animPercent)*m_animPercent;
	}
	else {
		easedPercent = 1;
	}

	m_gameOverText.setPosition(m_gameOverText.getPosition().x, -200 + 500 * easedPercent);
	m_retryText.setPosition(screenWidth + 500 - (500 + screenWidth / 2)*easedPercent, m_retryText.getPosition().y);
	m_quitText.setPosition(-500 + (500 + screenWidth / 2)*easedPercent, m_quitText.getPosition().y);
	m_background.setFillColor(sf::Color(0, 0, 0, easedPercent * 120));

	m_window.draw(m_background);
	m_window.draw(m_gameOverText);
	m_window.draw(m_retryText);
	m_window.draw(m_quitText);

	if (m_animPercent == 1) {

		sf::Vector2f selectorPos = m_selector.getPosition();

		if ((m_up || m_down) && m_selectorCoolDown.getElapsedTime().asSeconds() > 0.3) {

			if (selectorPos.y == 500) m_selector.setPosition(selectorPos.x, 600);
			else if (selectorPos.y == 600) m_selector.setPosition(selectorPos.x, 500);
			restartChoice = !restartChoice;

			m_selectorCoolDown.restart();

		}

		m_window.draw(m_selector);
	}

	// the player has made a choice
	if (sf::Keyboard::isKeyPressed(sf::Keyboard::Space)
		|| (sf::Joystick::isConnected(0) && sf::Joystick::isButtonPressed(0, 0))) {
		if (restartChoice) return 1;
		else return -3;
	}
	else {
		return -2;
	}

}

RetryScreen::~RetryScreen()
{
}
