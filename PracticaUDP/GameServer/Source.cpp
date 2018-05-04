#pragma once
#include <iostream>
#include <SFML\Network.hpp>
#include <PlayerInfo.h>
#include <Constants.h>

class serverPlayer : public PlayerInfo {
public:
	serverPlayer();
	serverPlayer( sf::IpAddress newIp, unsigned short newPort, std::string newName, sf::Vector2f newPosition, unsigned short newID);
	sf::IpAddress ip;
	unsigned short port;
	sf::Clock pingClock;
	std::map<sf::Uint32, sf::Packet> unconfirmedPackets;
};
serverPlayer::serverPlayer() {
	pingClock.restart();
}
serverPlayer::serverPlayer(sf::IpAddress newIp, unsigned short newPort, std::string newName, sf::Vector2f newPosition, unsigned short newID) {
	ip = newIp;
	port = newPort;
	name = newName;
	position = newPosition;
	id = newID;
	pingClock.restart();
}

//Constants
#define MAX_PING_MS 2500
#define RESEND_TIME_MS 200
sf::Vector2f startPositions[4] = { 
	sf::Vector2f(0,0), 
	sf::Vector2f(TILESIZE*(N_TILES_WIDTH-1),0), 
	sf::Vector2f(0,TILESIZE*(N_TILES_HEIGHT - 1)), 
	sf::Vector2f(TILESIZE*(N_TILES_WIDTH - 1),
		TILESIZE*(N_TILES_HEIGHT - 1)) 
};

//Global vars
std::vector<serverPlayer> players;
unsigned short totalPlayers = 0;
sf::UdpSocket socket;
sf::Clock aknowledgeClock;
sf::Uint32 idPacket = 0;

//Fw declarations
bool isPlayerSaved(sf::IpAddress ip, unsigned short port, serverPlayer* &playerFound);
bool checkMove(int x, int y);
void sendPacket(sf::Packet packet, sf::IpAddress ipClient, unsigned short portClient, float failRate = 0);
void sendAllExcept(sf::Uint32 idPack, sf::Packet packet, unsigned short idClientExcluded, float failRate = 0);
sf::Packet pingPack() { sf::Packet p; p << (sf::Uint8)Cabeceras::PING; p << idPacket++; return p; }
sf::Packet disconnPack(unsigned short idPlayer) { sf::Packet p; p << (sf::Uint8)Cabeceras::DISCONNECTED; p << idPacket; p << (sf::Uint8)idPlayer; return p; }
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
			sf::Uint8 comandoInt;
			pack >> comandoInt;
			Cabeceras comando = (Cabeceras)comandoInt;

			switch (comando)
			{
			case HELLO:
			{
				std::cout << "A client has connected (port: " << clientPort << ")\n";
				//Check if the player was not saved
				serverPlayer* existingPlayer = nullptr;
				if (!isPlayerSaved(clientIp,clientPort,existingPlayer))
				{
					//Send welcome, id and position
					sf::Vector2f position = startPositions[totalPlayers];
					serverPlayer newPlayer = serverPlayer(clientIp, clientPort, (std::string)"", position, totalPlayers);
					players.push_back(newPlayer);

					sf::Packet welcomePack;
					welcomePack << (sf::Uint8)Cabeceras::WELCOME;
					welcomePack << (sf::Uint8) totalPlayers;
					welcomePack << (float) position.x;
					welcomePack << (float) position.y;
					sendPacket(welcomePack, clientIp, clientPort, 0.f);

					//Send that player to others (if there are more)
					if (totalPlayers != 0)
					{
						sf::Packet newPlayerPack;
						newPlayerPack << (sf::Uint8)Cabeceras::NEW_PLAYER;
						newPlayerPack << (sf::Uint32) idPacket;
						newPlayerPack << (sf::Uint8) newPlayer.id;
						newPlayerPack << (float) newPlayer.position.x;
						newPlayerPack << (float) newPlayer.position.y;
						//afegir a cada jugador el paquet critic - posar-ho dins la funcio? afegir un parametre per si es vol fer critic-afegir a les llistes d ecritics? - posar en els comentaris que es una funcio per critics
						sendAllExcept(idPacket, newPlayerPack, newPlayer.id, 0);
						std::cout << "mida unconf. packets" << players.at(0).unconfirmedPackets.size() << "\n";
						idPacket++;

						//Send other players to that one
						for (auto &aPlayer : players) {
							if (aPlayer.id != totalPlayers)	//totalPlayers = id of new player
							{
								sf::Packet oldPlayerPack;
								oldPlayerPack << (sf::Uint8)Cabeceras::NEW_PLAYER;
								oldPlayerPack << (sf::Uint32) idPacket;
								oldPlayerPack << (sf::Uint8) aPlayer.id;
								oldPlayerPack << (float) aPlayer.position.x;
								oldPlayerPack << (float) aPlayer.position.y;
								std::cout << "sending to " << aPlayer.id << "\n";
								newPlayer.unconfirmedPackets[idPacket] = oldPlayerPack;
								sendPacket(oldPlayerPack, clientIp, clientPort, 0);
								idPacket++;
							}
						}
					}

					totalPlayers++;
				}
				else
				{
					std::cout << "(client was already saved, sending welcome again)" << std::endl;
					sf::Packet welcomePack;
					welcomePack << (sf::Uint8)Cabeceras::WELCOME;
					welcomePack << (sf::Uint8) existingPlayer->id;
					welcomePack << (float) existingPlayer->position.x;
					welcomePack << (float) existingPlayer->position.y;
					sendPacket(welcomePack, clientIp, clientPort, 0.f);
				}
			}

				break;
			case WELCOME:
				break;
			case ACKNOWLEDGE:
			{
				serverPlayer* akPlayer = nullptr;
				if (isPlayerSaved(clientIp, clientPort, akPlayer)) {
					sf::Uint32 akidPacket;
					pack >> akidPacket;
					if (akPlayer->unconfirmedPackets.find(akidPacket) != akPlayer->unconfirmedPackets.end()) {	//if packet is inside unconfirmed packets
						akPlayer->unconfirmedPackets.erase(akPlayer->unconfirmedPackets.find(akidPacket));		//delete it
						std::cout << "paquet esborrat!\n";
					}
					akPlayer->pingClock.restart();
				}
			}
				break;
			case NEW_PLAYER:
				break;
			case ACUM_MOVE:
			{
				sf::Uint32 acumIdPacket;
				float movement_x, movement_y;
				pack >> acumIdPacket;
				pack >> movement_x;
				pack >> movement_y;
				std::cout << "movement ";
				serverPlayer* akPlayer = nullptr;
				if (isPlayerSaved(clientIp, clientPort, akPlayer)) {
					std::cout << "from player " << akPlayer->id << " with x:" << movement_x << " y: " << movement_y << std::endl;
					sf::Vector2f finalPos = akPlayer->position + CHARACTER_SPEED*sf::Vector2f(movement_x, movement_y);
					if (finalPos.x < 0 || finalPos.x > TILESIZE*N_TILES_WIDTH ||
						finalPos.y < 0 || finalPos.y > TILESIZE*N_TILES_HEIGHT)
					{
						std::cout << "Wrong movement!!\n";
						finalPos = akPlayer->position;
					}

					akPlayer->position = finalPos;
					sf::Packet ok_movePack;
					ok_movePack << (sf::Uint8) Cabeceras::OK_POSITION;
					ok_movePack << (sf::Uint32) acumIdPacket;
					ok_movePack << (sf::Uint8) akPlayer->id;
					ok_movePack << (float) finalPos.x;
					ok_movePack << (float) finalPos.y;
					
					for (auto &aPlayer : players) {	//send to all
						sendPacket(ok_movePack, aPlayer.ip, aPlayer.port, 0);
					}

				}
			}
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

		//Send unconfirmed packets if no aknowledge was recieved
		if (aknowledgeClock.getElapsedTime().asMilliseconds() >= RESEND_TIME_MS)			//TODO: POSAR A DALT, ABANS DE TOT?
		{
			auto aPlayer = players.begin();
			while ( aPlayer != players.end()) {
				//for (auto &aPlayer : players) {
				if (aPlayer->pingClock.getElapsedTime().asMilliseconds() > MAX_PING_MS)
				{
					std::cout << "DESCONNECTAT PLAYER ID: " << aPlayer->id << "\n";
					/*sf::Packet p; 
					p << (sf::Uint8)Cabeceras::DISCONNECTED; 
					p << (sf::Uint32)idPacket;
					p << (sf::Uint8)aPlayer->id;
					sendAllExcept(idPacket, p, aPlayer->id, 0);*/
					sendAllExcept(idPacket, disconnPack(aPlayer->id), aPlayer->id, 0);
					idPacket++;
					aPlayer = players.erase(aPlayer);
				}
				else
				{
					//Iterar el map i fer send
					for (auto it : aPlayer->unconfirmedPackets)
					{
						std::cout << "ReSending to " << aPlayer->id << "\n";
						sendPacket(it.second, aPlayer->ip, aPlayer->port, 0);
					}
					sendPacket(pingPack(), aPlayer->ip, aPlayer->port);

					++aPlayer;
				}
			}
			aknowledgeClock.restart();
		}


	}

	return 0;
}


bool isPlayerSaved(sf::IpAddress ip, unsigned short port, serverPlayer* &playerFound) {
	for (int p = 0; p < players.size(); p++)
	{
		if (players.at(p).ip == ip && players.at(p).port == port)
		{
			playerFound = &players.at(p);
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

//Sends a packet to all players except the excluded one. Saves to critical package map
void sendAllExcept(sf::Uint32 idPack, sf::Packet packet, unsigned short idClientExcluded, float failRate) {
	std::cout << "sending package to all\n";
	for (auto &aPlayer : players) {
		if (aPlayer.id != idClientExcluded)
		{
			std::cout << "sending to " << aPlayer.id << "\n";
			aPlayer.unconfirmedPackets[idPack] = packet;
			sendPacket(packet, aPlayer.ip, aPlayer.port, failRate);
		}
	}
}


