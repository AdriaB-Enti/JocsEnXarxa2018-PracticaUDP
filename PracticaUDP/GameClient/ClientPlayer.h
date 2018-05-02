#pragma once
#include <PlayerInfo.h>
#include <SFML\Graphics.hpp>

//Client player is used for all the players in the client
class ClientPlayer : public PlayerInfo {
public:
	sf::Sprite characterSprite;
	ClientPlayer();
	ClientPlayer(std::string _name, sf::Vector2i _position, unsigned short _id, sf::Sprite _sprite);
	void moveTo(sf::Vector2i newPosition);
};