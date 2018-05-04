#pragma once
#include "ClientPlayer.h"

const float offSetText = 10;

ClientPlayer::ClientPlayer() { }

ClientPlayer::ClientPlayer(std::string _name, sf::Vector2f _position, unsigned short _id, sf::Sprite _sprite)
{
	name = _name;
	position = _position;
	id = _id;
	characterSprite = _sprite;
}

void ClientPlayer::moveTo(sf::Vector2f newPosition)
{
	position = newPosition;
	characterSprite.setPosition(newPosition.x, newPosition.y);
	nameText.setPosition(newPosition.x+offSetText, newPosition.y-offSetText);
	if (((newPosition.x - position.x) < 0 && characterSprite.getScale().x < 0) || ((newPosition.x - position.x) > 0 && characterSprite.getScale().x > 0))
	{
		characterSprite.scale(-1, 1);
	}
}

void ClientPlayer::translate(sf::Vector2f displacement)
{
	position += displacement;
	characterSprite.move(displacement);
	nameText.move(displacement);
	//Scale.x > 0 -> looking right  --  Scale.x < 0 -> looking left
	if ((displacement.x > 0 && characterSprite.getScale().x < 0) || (displacement.x < 0 && characterSprite.getScale().x > 0))
	{
		characterSprite.scale(-1, 1);
	}
}
