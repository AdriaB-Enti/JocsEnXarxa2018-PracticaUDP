#pragma once
#include "ClientPlayer.h"

const float offSetTextX = -40;
const float offSetTextY = -35;

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
	nameText.setPosition(newPosition.x+offSetTextX, newPosition.y-offSetTextY);
}

void ClientPlayer::translate(sf::Vector2f displacement)
{
	//Scale.x > 0 -> looking right  --  Scale.x < 0 -> looking left
	if ((displacement.x > 0 && characterSprite.getScale().x < 0) || (displacement.x < 0 && characterSprite.getScale().x > 0))
	{
		characterSprite.scale(-1, 1);	//rotate character
	}
	position += displacement;
	characterSprite.move(displacement);
	nameText.move(displacement);
}

void ClientPlayer::prepareInterpolation(sf::Vector2f finalPos)
{
	if (((finalPos.x - position.x) > 0 && characterSprite.getScale().x < 0) || ((finalPos.x - position.x) < 0 && characterSprite.getScale().x > 0))
	{
		characterSprite.scale(-1, 1);	//rotate character
	}

	const int STEP_COUNT = 5;
	sf::Vector2f step = (finalPos - position) / (float)STEP_COUNT;		//Direction with distance moved at each step to reach finalPos
	for (int i = 1; i <= STEP_COUNT; i++)
	{
		interpolationMoves.push_back(position+step*(float)i);
	}
}

void ClientPlayer::moveStep()
{
	if (!interpolationMoves.empty()) {
		moveTo(interpolationMoves.front());						//move to first interpolation pos
		interpolationMoves.erase(interpolationMoves.begin());	//delete used interp. pos
	}
}
