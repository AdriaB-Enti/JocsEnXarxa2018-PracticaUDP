#pragma once
#include <SFML\Graphics.hpp>



class PlayerInfo
{
public:
	PlayerInfo();
	~PlayerInfo();
	std::string name;
	sf::Vector2f position;
	bool isAlive = true;
	unsigned short id;
};