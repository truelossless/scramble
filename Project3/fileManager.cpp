#include <string>
#include <SFML/Graphics.hpp>

#include "fileManager.h"

std::map<std::string, sf::Texture> FileManager::textures;
std::map<std::string, FileManager::SoundFile> FileManager::sounds;

sf::Texture& FileManager::getTexture(std::string path) {
	// chargement d'une texture depuis la mémoire ou le disque

	std::map<std::string, sf::Texture>::iterator i = textures.find(path);
	if (i != textures.end()) {
		// la texture est déja chargée
		return i->second;
	}

	// la texture n'est pas chargée
	textures[path].loadFromFile(path);
	return textures[path];

}

sf::Sound& FileManager::getSound(std::string path) {
	// chargement d'une texture depuis la mémoire ou le disque

	std::map<std::string, SoundFile>::iterator i = sounds.find(path);
	if (i != sounds.end()) {
		// la texture est déja chargée
		return i->second.sound;
	}

	// la texture n'est pas chargée
	sounds[path].buffer.loadFromFile(path);
	sounds[path].sound.setBuffer(sounds[path].buffer);
	return sounds[path].sound;
}