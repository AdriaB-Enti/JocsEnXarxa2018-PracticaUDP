#pragma once
#include <iostream>
#include <SFML\Network.hpp>
#include <PlayerInfo.h>
#include <Constants.h>

class serverPlayer : public PlayerInfo {
public:
	serverPlayer();
	serverPlayer( sf::IpAddress newIp, unsigned short newPort, std::string newName, sf::Vector2i newPosition, unsigned short newID);
	sf::IpAddress ip;
	unsigned short port;
};
serverPlayer::serverPlayer() {}
serverPlayer::serverPlayer(sf::IpAddress newIp, unsigned short newPort, std::string newName, sf::Vector2i newPosition, unsigned short newID) {
	ip = newIp;
	port = newPort;
	name = newName;
	position = newPosition;
	id = newID;
}

//Constants
sf::Vector2i startPositions[4] = { 
	sf::Vector2i(0,0), 
	sf::Vector2i(TILESIZE*(N_TILES_WIDTH-1),0), 
	sf::Vector2i(0,TILESIZE*(N_TILES_HEIGHT - 1)), 
	sf::Vector2i(TILESIZE*(N_TILES_WIDTH - 1),
		TILESIZE*(N_TILES_HEIGHT - 1)) 
};

//Global vars
std::vector<serverPlayer> players;
unsigned short totalPlayers = 0;
sf::UdpSocket socket;
sf::Clock aknowledgeClock;

//Fw declarations
bool isPlayerSaved(sf::IpAddress ip, unsigned short port, serverPlayer &playerFound);
bool checkMove(int x, int y);
void sendPacket(sf::Packet packet, sf::IpAddress ipClient, unsigned short portClient, float failRate = 0);
void sendAllExcept(sf::Packet packet, unsigned short idClientExcluded, float failRate = 0);

int main()
{
	//Reset random seed
	srand(time(NULL));
	aknowledgeClock.restart();
	//serverPlayer provaPlayer = serverPlayer(sf::IpAddress("127.0.0.1"),50);
	
	socket.setBlocking(false);
	socket.bind(PORTSERVER);

	//TODO: enviar el match_start quan estiguin tots els jugadors necessaris
	while (true)
	{
		sf::IpAddress clientIp;
		unsigned short clientPort;
		sf::Packet pack;
		sf::UdpSocket::Status status = socket.receive(pack, clientIp, clientPort);
		
		switch (status)
		{
		case sf::Socket::Done:
		{

			std::cout << "Package recieved" << std::endl;
			sf::Uint8 comandoInt;
			pack >> comandoInt;
			Cabeceras comando = (Cabeceras)comandoInt;

			switch (comando)
			{
			case HELLO:
			{
				std::cout << "A client has connected (port: " << clientPort << ")\n";
				//Check if the player was not saved
				serverPlayer existingPlayer;
				if (!isPlayerSaved(clientIp,clientPort,existingPlayer))
				{
					//Send welcome, id and position
					sf::Vector2i position = startPositions[totalPlayers];
					serverPlayer newPlayer = serverPlayer(clientIp, clientPort, (std::string)"", position, totalPlayers);
					players.push_back(newPlayer);

					sf::Packet welcomePack;
					welcomePack << (sf::Uint8)Cabeceras::WELCOME;
					welcomePack << (sf::Uint8) totalPlayers;
					welcomePack << (sf::Uint32) position.x;
					welcomePack << (sf::Uint32) position.y;
					sendPacket(welcomePack, clientIp, clientPort, 0.f);

					if (totalPlayers != 0)
					{
						//SEND NEW PLAYER TO OTHERS - paquet critic?
						//sendAllExcept()
					}

					totalPlayers++;
				}
				else
				{
					std::cout << "(client was already saved, sending welcome again)" << std::endl;
					sf::Packet welcomePack;
					welcomePack << (sf::Uint8)Cabeceras::WELCOME;
					welcomePack << (sf::Uint8) existingPlayer.id;
					welcomePack << (sf::Uint32) existingPlayer.position.x;
					welcomePack << (sf::Uint32) existingPlayer.position.y;
					socket.send(welcomePack, clientIp, clientPort);	//usar sendPacket()
					sendPacket(welcomePack, clientIp, clientPort, 0.f);
				}
			}

				break;
			case WELCOME:
				break;
			case ACKNOWLEDGE:
				break;
			case NEW_PLAYER:
				break;
			case MOVE_LEFT:
			{
				std::cout << "player " << clientIp.toString() << " moving left\n";

				//hard coded al primer jugador. buscar el jugador pertinent
				//players.front().position
				//int newPosX = CHARACTER_SPEED
				//checkMove(newPosX,
				sf::Packet movementPacket;
				movementPacket << (sf::Uint8)Cabeceras::OK_POSITION;

			}

				break;
			case MOVE_RIGHT:
				std::cout << "player " << clientIp.toString() << " moving right\n";
				break;
			case MOVE_UP:
				std::cout << "player " << clientIp.toString() << " moving up\n";
				break;
			case MOVE_DOWN:
				std::cout << "player " << clientIp.toString() << " moving down\n";
				break;
			default:
				break;
			}



			break;
		}

		case sf::Socket::NotReady:
			break;
		case sf::Socket::Partial:
			break;
		case sf::Socket::Disconnected:
			break;
		case sf::Socket::Error:
			break;
		default:
			break;
		}

	}





	return 0;
}


bool isPlayerSaved(sf::IpAddress ip, unsigned short port, serverPlayer &playerFound) {
	for (int p = 0; p < players.size(); p++)
	{
		if (players.at(p).ip == ip && players.at(p).port == port)
		{
			playerFound = players.at(p);
			return true;
		}
	}
	return false;
}

//Checks if a position is inside the map
bool checkMove(int x, int y)
{
	if (x > 0 && x < TILESIZE*N_TILES_WIDTH && y > 0 && y < TILESIZE*N_TILES_HEIGHT) {
		return true;
	}

	return false;
}


//Sends a packet but if failRate is greater than 0, there is a chance the packet won't be sent at all (for debug purposes)
//'failRate' has to be in range [0..1] where 0 -> Will always send, and 1-> Will never send
void sendPacket(sf::Packet packet, sf::IpAddress ipClient, unsigned short portClient, float failRate) {
	float random = (rand() % 100) / 100.f;

	if (failRate != 0 && random < failRate)	//fails to send
	{
		std::cout << "A packet could not be sent\n";
	}
	else {
		socket.send(packet, ipClient, portClient);
	}
}

//Sends a packet to all players except the excluded one
void sendAllExcept(sf::Packet packet, unsigned short idClientExcluded, float failRate) {
	for each (serverPlayer aPlayer in players)
	{
		if (aPlayer.id != idClientExcluded)
		{
			sendPacket(packet, aPlayer.ip, aPlayer.port, failRate);
		}
	}
}


