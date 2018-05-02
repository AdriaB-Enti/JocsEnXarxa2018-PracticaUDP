#pragma once
#include "ClientPlayer.h"

ClientPlayer::ClientPlayer() { }

ClientPlayer::ClientPlayer(std::string _name, sf::Vector2i _position, unsigned short _id, sf::Sprite _sprite)
{
	name = _name;
	position = _position;
	id = _id;
	characterSprite = _sprite;
}

void ClientPlayer::moveTo(sf::Vector2i newPosition)
{
	position = newPosition;
	characterSprite.setPosition(newPosition.x, newPosition.y);
}
