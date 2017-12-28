#pragma once

#include <string>
#include <SFML/Graphics.hpp>
#include <SFML/Audio.hpp>

namespace FileManager {
	
	struct SoundFile {
		sf::SoundBuffer buffer;
		sf::Sound sound;
	};

	extern std::map<std::string, sf::Texture> textures;
	extern std::map<std::string, SoundFile> sounds;
	sf::Texture& getTexture(std::string path);
	sf::Sound& getSound(std::string path);
}