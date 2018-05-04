#pragma once
#include <PlayerInfo.h>
#include <SFML\Graphics.hpp>

//Client player is used for all the players in the client
class ClientPlayer : public PlayerInfo {
public:
	ClientPlayer();
	ClientPlayer(std::string _name, sf::Vector2f _position, unsigned short _id, sf::Sprite _sprite);

	sf::Sprite characterSprite;
	sf::Text nameText;
	bool lookingRight = true;
	std::vector<sf::Vector2f> interpolationMoves;
	
	void moveTo(sf::Vector2f newPosition);
	void translate(sf::Vector2f displacement);
	void prepareInterpolation(sf::Vector2f finalPos);
	void moveStep();
};