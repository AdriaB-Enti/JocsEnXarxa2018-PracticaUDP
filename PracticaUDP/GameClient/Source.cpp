#pragma once
#include <SFML\Graphics.hpp>
#include <SFML\Network.hpp>
#include <iostream>
#include <Constants.h>
#include "ClientPlayer.h"

//Constants
#define FAIL_RATE 0
#define ACUM_MOVE_TIME 100

//tenir un client playerInfo? amb el sprite que toqui...

//Global vars
unsigned short myID;
ClientPlayer myPlayer;
std::vector<ClientPlayer> players;
sf::Clock gameClock, acumMoveTime;
sf::Time deltaSeconds;
sf::UdpSocket socket;
sf::Texture mapTexture, characterTexture;
sf::Font font;
float movement_x, movement_y;
sf::Uint32 idPack = 0;
struct move_packet
{
	sf::Uint32 id;
	sf::Packet pack;
	sf::Vector2f finalPos;
};
std::vector<move_packet> acum_move_packs;
//llista de packets crítics

//Fw declarations
void sendPacket(sf::Packet packet, float failRate = 0);
void recieveFromServer();
bool isPlayerAlreadySaved(unsigned short pyID);
sf::Packet akPacket(sf::Uint32 idP) { 
	sf::Packet p; p << (sf::Uint8) Cabeceras::ACKNOWLEDGE; p << idP;
	std::cout << "package AK " << Cabeceras::ACKNOWLEDGE << std::endl;
	return p; }

bool inline isOficialServer(sf::IpAddress ip, unsigned short port) { return (ip.toString()==std::string(IPSERVER))&&(port==PORTSERVER);}

//TODO: demanar nom a l'usuari - mirar practica tcp
int main()
{
	//Load textures and font
	if (!mapTexture.loadFromFile("mapa2.png"))
		std::cout << "Error al cargar la textura del mapa!\n";
	if (!characterTexture.loadFromFile("personatgeTransp.png"))
		std::cout << "Error al cargar la textura del personaje!\n";
	if (!font.loadFromFile("courbd.ttf"))
		std::cout << "Error al cargar la fuente" << std::endl;

	//Reset random seed and work in non-blocking mode
	srand(time(NULL));
	socket.setBlocking(false);

	sf::Packet helloPack;
	helloPack << Cabeceras::HELLO;
	sendPacket(helloPack,0);

	//Recive welcome from server
	sf::Clock welcomeClock;
	welcomeClock.restart();
	bool confirmationRecieved = false;
	while (!confirmationRecieved)
	{
		//Send Hello packet if no welcome was recieved
		if (welcomeClock.getElapsedTime().asMilliseconds() >= 500)
		{
			std::cout << "Seding hello packet again\n";
			sf::Packet helloPack;
			helloPack << Cabeceras::HELLO;
			sendPacket(helloPack,0.f);
			welcomeClock.restart();
		}

		sf::Packet serverPack;
		sf::IpAddress ipServer;
		unsigned short portServer;
		sf::UdpSocket::Status status = socket.receive(serverPack,ipServer, portServer);


		switch (status)
		{
		case sf::Socket::Done:
			if (isOficialServer(ipServer,portServer))
			{
				std::cout << "S'ha rebut el packet del servidor\n";

				sf::Uint8 cabecera8, id8;
				serverPack >> cabecera8;

				if ((Cabeceras)cabecera8 == Cabeceras::WELCOME)
				{
					sf::Uint8 id8;
					sf::Vector2f myPos;
					serverPack >> id8;
					myID = (unsigned short) id8;
					serverPack >> myPos.x;
					serverPack >> myPos.y;
					std::cout << "Welcome, player with ID=" << myID << std::endl;

					myPlayer.characterSprite = sf::Sprite(characterTexture);
					myPlayer.nameText = sf::Text("player" + std::to_string(myID), font, 18);
					myPlayer.nameText.setFillColor(sf::Color::Blue);
					myPlayer.moveTo(myPos);
					std::cout << "Your position is =" << (int)myPlayer.position.x << ":" << (int)myPlayer.position.y << std::endl;

					confirmationRecieved = true;
				}

			}

			break;
		case sf::Socket::NotReady:
			break;
		case sf::Socket::Partial:
			std::cout << "Partial package\n";
			break;
		case sf::Socket::Disconnected:
			break;
		case sf::Socket::Error:
			break;
		default:
			break;
		}
	}

	//Creación de la ventana
	sf::Vector2i screenDimensions(800, 900);
	sf::RenderWindow window;
	window.create(sf::VideoMode(screenDimensions.x, screenDimensions.y), "UDPGame"+ std::to_string(myID));
	window.setFramerateLimit(60);
	window.setKeyRepeatEnabled(false);
	
	//Texturas, Sprites y fuentes
	sf::RectangleShape mapShape(sf::Vector2f(TILESIZE*N_TILES_WIDTH, TILESIZE*N_TILES_HEIGHT));
	mapShape.setTexture(&mapTexture);

	gameClock.restart();
	acumMoveTime.restart();
	while (window.isOpen())
	{
		deltaSeconds = gameClock.restart();
		float deltaTime = deltaSeconds.asSeconds();

		recieveFromServer();
		//sendInputMovement();
		
		sf::Event event;
		while (window.pollEvent(event))	//TODO: esborrar o netejar/organitzar
		{
			//Cerrar la ventana
			if (event.type == sf::Event::Closed)
				window.close();
			//Detectar eventos de teclado
			if (event.type == sf::Event::KeyPressed && (event.key.code == sf::Keyboard::Escape)){
				window.close();
			}
			//Detectar si estamos escribiendo algo, enviar el texto si presionamos enter, borrar la ultima letra si apretamos Backspace
			if (event.type == sf::Event::TextEntered)
			{
				if (event.text.unicode > 31 && event.text.unicode < 128) {
					/*mensajeTeclado.push_back(static_cast<char>(event.text.unicode));*/
				}
			}
			if (event.type == sf::Event::KeyPressed && (event.key.code == sf::Keyboard::Return)) {		//Si apretamos enter, se envia el mensaje que teniamos escrito - TODO: controlar que no s'envii si està buit
				/*if (!mensajeTeclado.empty()) {
					sendToServer(user + ": " + mensajeTeclado);
					mensajeTeclado = "";
				}*/
			}
			if (event.type == sf::Event::KeyPressed && (event.key.code == sf::Keyboard::BackSpace)) {
				/*if (!mensajeTeclado.empty())
					mensajeTeclado.pop_back();*/
			}
		}

		//detect input only if window has focus
		if (window.hasFocus())
		{
			if (sf::Keyboard::isKeyPressed(sf::Keyboard::Left)) {
				movement_x -= deltaTime;
				myPlayer.translate(sf::Vector2f(-deltaTime * CHARACTER_SPEED, 0));
			}
			if (sf::Keyboard::isKeyPressed(sf::Keyboard::Right)) {
				movement_x += deltaTime;
				myPlayer.translate(sf::Vector2f(deltaTime *CHARACTER_SPEED, 0));
			}
			if (sf::Keyboard::isKeyPressed(sf::Keyboard::Up)) {
				movement_y -= deltaTime;
				myPlayer.translate(sf::Vector2f(0, -deltaTime * CHARACTER_SPEED));
			}
			if (sf::Keyboard::isKeyPressed(sf::Keyboard::Down)) {
				movement_y += deltaTime;
				myPlayer.translate(sf::Vector2f(0, deltaTime*CHARACTER_SPEED));
			}
		}

		//Draw map and players
		window.clear();
		window.draw(mapShape);
		for each (ClientPlayer cplayer in players)
		{
			window.draw(cplayer.characterSprite);
			window.draw(cplayer.nameText);
		}
		window.draw(myPlayer.characterSprite);
		window.draw(myPlayer.nameText);


		if (acumMoveTime.getElapsedTime().asMilliseconds() > ACUM_MOVE_TIME)	//Send all acumulated movement (time moving on x and y)
		{
			if (movement_x != 0 || movement_y != 0)	//no need to send if we haven't moved
			{
				sf::Packet acum_pack;
				acum_pack << (sf::Uint8) Cabeceras::ACUM_MOVE;
				acum_pack << (sf::Uint32) idPack;
				acum_pack << movement_x;
				acum_pack << movement_y;

				move_packet acumMove;
				acumMove.id = idPack;
				acumMove.pack = acum_pack;
				acumMove.finalPos = myPlayer.position;
				acum_move_packs.push_back(acumMove);
				std::cout << "mida acums: " << acum_move_packs.size() << std::endl;

				sendPacket(acum_pack,0);

				movement_x = 0;
				movement_y = 0;
				idPack++;
			}
			acumMoveTime.restart();
		}




		window.display();
	}

	

	return 0;
}

//Sends a packet but if failRate is greater than 0, there is a chance the packet won't be sent at all (for debug purposes)
//'failRate' has to be in range [0..1] where 0 -> Will always send, and 1-> Will never send
void sendPacket(sf::Packet packet, float failRate) {
	float random = (rand() % 100) / 100.f;

	if (failRate != 0 && random < failRate)	//fails to send
	{
		std::cout << "A packet could not be sent\n";
	} else {
		socket.send(packet, IPSERVER, PORTSERVER);
	}
}

void recieveFromServer()
{
	sf::Packet serverPacket;
	sf::IpAddress ip;
	unsigned short port;
	sf::UdpSocket::Status status = socket.receive(serverPacket, ip,port);
	switch (status)
	{
	case sf::Socket::Done:
	{
		if (isOficialServer(ip, port)) {
			sf::Uint8 comandoInt;
			serverPacket >> comandoInt;
			Cabeceras comando = (Cabeceras)comandoInt;
			switch (comando)
			{
			case ACKNOWLEDGE:
				break;
			case NEW_PLAYER:
			{
				std::cout << "New player has connected!";	//moure dintre del if

				ClientPlayer newcPlayer;
				sf::Uint32 idPacket;
				sf::Uint8 id8;
				sf::Vector2f newPlayerPos;

				serverPacket >> idPacket;
				serverPacket >> id8;
				newcPlayer.id = (unsigned short) id8;
				serverPacket >> newPlayerPos.x;
				serverPacket >> newPlayerPos.y;
				
				if (!isPlayerAlreadySaved(newcPlayer.id)) { //Check if we already have that player
					newcPlayer.characterSprite = sf::Sprite(characterTexture);
					//newcPlayer.characterSprite.setPosition(newcPlayer.position.x, newcPlayer.position.y);
					newcPlayer.nameText = sf::Text("player" + std::to_string(newcPlayer.id), font, 18);
					newcPlayer.nameText.setFillColor(sf::Color::Red);
					newcPlayer.moveTo(newPlayerPos);
					players.push_back(newcPlayer);

					std::cout << "id: " << newcPlayer.id << " position x: " << newcPlayer.position.x << " position y: " << newcPlayer.position.y << std::endl;
					//std::cout << "idpack " << idPacket << std::endl;
				}
				sf::Packet akPacket;
				akPacket << (sf::Uint8) Cabeceras::ACKNOWLEDGE;
				akPacket << idPacket;
				sendPacket(akPacket, 0);
			}
			break;
			case PING:
			{
				sf::Uint32 idPacket;
				serverPacket >> idPacket;

				sf::Packet akPacket;
				akPacket << (sf::Uint8) Cabeceras::ACKNOWLEDGE;
				akPacket << idPacket;
				sendPacket(akPacket, 0);
			}
				break;
			case DISCONNECTED:
			{
				sf::Uint32 idPack;
				sf::Uint8 id8player;
				serverPacket >> idPack;
				serverPacket >> id8player;
				unsigned short idDisconnected = (unsigned short)id8player;
				std::cout << "player " << idDisconnected << " disconnected\n";
				for (auto i = players.begin(); i != players.end(); i++)
				{
					if (i->id == idDisconnected)
					{
						std::cout << "player " << idDisconnected << " ESBORRAT\n";
						players.erase(i);
						break;
					}
				}
				sendPacket(akPacket(idPack), 0);
			}
				break;
			case OK_POSITION:
			{
				sf::Uint32 idMove;
				sf::Uint8 idPlayerMoved;
				float newPosX, newPosY;

				serverPacket >> idMove;
				serverPacket >> idPlayerMoved;
				serverPacket >> newPosX;
				serverPacket >> newPosY;
				
				sf::Vector2f newPosition = sf::Vector2f((float)newPosX, (float)newPosY);

				if (myID == (unsigned short)idPlayerMoved)		//it's for our player
				{
					for (auto movePack = acum_move_packs.begin(); movePack != acum_move_packs.end();) {
						if (movePack->id < idMove)
						{
							movePack = acum_move_packs.erase(movePack);
						}
						if (movePack->id == idMove) {
							if ((movePack->finalPos.x != newPosition.x) || (movePack->finalPos.y != newPosition.y))
							{
								std::cout << "DIFFERENT POS. X:" << movePack->finalPos.x <<"-" << newPosX << " Y:" << movePack->finalPos.y << "-" << newPosY << std::endl;
								myPlayer.moveTo(newPosition);			//TODO: fer-ho dins del for, només si resulta
							}
							movePack = acum_move_packs.erase(movePack);
							//esborrar la resta de paquets si?
						}
						else {
							movePack++;
						}
					}
				}
				else                      //for other players
				{
					for (auto &aPlayer : players)
					{
						if (aPlayer.id == (unsigned short)idPlayerMoved)
						{
							aPlayer.moveTo(newPosition);
						}
					}

				}
			}
				break;
			default:
				break;

			}
		}
	}
		break;
	case sf::Socket::NotReady:
		break;
	case sf::Socket::Partial:
		break;
	case sf::Socket::Disconnected:
		break;
	case sf::Socket::Error:
		std::cout << "Error reciving packet from server\n";
		break;
	default:
		break;
	}
}

bool isPlayerAlreadySaved(unsigned short pyID) {
	for (auto &nplayer : players)
	{
		if (nplayer.id == pyID)
			return true;
	}
	return false;
}



