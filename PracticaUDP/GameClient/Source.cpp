#pragma once
#include <PlayerInfo.h>
#include <SFML\Graphics.hpp>
#include <SFML\Network.hpp>
#include <iostream>
#include <Constants.h>

//Constants
#define FAIL_RATE 0

//Global vars
unsigned short myID;
PlayerInfo myPlayer;
sf::Sprite characterSprite;
sf::Clock gameClock;
sf::Time deltaSeconds;
sf::UdpSocket socket;

//Fw declarations
void sendPacket(sf::Packet packet, float failRate = 0);
void sendInputMovement();
void recieveFromServer();

//TODO: demanar nom a l'usuari - mirar practica tcp

int main()
{
	//Reset random seed and work in non-blocking mode
	srand(time(NULL));
	socket.setBlocking(false);

	sf::Packet helloPack;
	helloPack << Cabeceras::HELLO;
	sendPacket(helloPack,1);

	//Recibir el welcome del servidor
	sf::Clock welcomeClock;
	welcomeClock.restart();
	bool confirmationRecieved = false;
	while (!confirmationRecieved)
	{
		if (welcomeClock.getElapsedTime().asMilliseconds() >= 500)
		{
			std::cout << "Seding welcome packet again\n";
			sf::Packet helloPack;
			helloPack << Cabeceras::HELLO;
			sendPacket(helloPack,0.5f);
			welcomeClock.restart();
		}

		sf::Packet serverPack;
		sf::IpAddress ipServer;
		unsigned short portServer;
		sf::UdpSocket::Status status = socket.receive(serverPack,ipServer, portServer);

		switch (status)
		{
		case sf::Socket::Done:
			std::cout << "S'ha rebut el packet del servidor\n";

			sf::Uint8 cabecera8, id8;
			serverPack >> cabecera8;

			if ((Cabeceras)cabecera8 == Cabeceras::WELCOME)
			{
				sf::Uint8 id8;
				serverPack >> id8;
				myID = (unsigned short)id8;
				serverPack >> myPlayer.position.x;
				serverPack >> myPlayer.position.y;
				std::cout << "Welcome, player with ID=" << myID << std::endl;
				std::cout << "Your position is =" << (int)myPlayer.position.x << ":" << (int)myPlayer.position.y << std::endl;
				confirmationRecieved = true;
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
		//fer un sleep per si de cas?
		//reenviar el packet
	}

	/*int n;
	std::cin >> n;*/

	//Creación de la ventana
	sf::Vector2i screenDimensions(800, 900);
	sf::RenderWindow window;
	window.create(sf::VideoMode(screenDimensions.x, screenDimensions.y), "UDPGame");
	
	//Texturas, Sprites y fuentes
	sf::RectangleShape mapShape(sf::Vector2f(TILESIZE*N_TILES_WIDTH, TILESIZE*N_TILES_HEIGHT));

	sf::Texture texture, characterTexture;
	if (!texture.loadFromFile("mapa2.png"))
		std::cout << "Error al cargar la textura del mapa!\n";
	if (!characterTexture.loadFromFile("personatgeTransp.png"))
		std::cout << "Error al cargar la textura del personaje!\n";
	/*if (!font.loadFromFile("courbd.ttf"))
		std::cout << "Error al cargar la fuente" << std::endl;*/
	mapShape.setTexture(&texture);

	//Create character sprite  -- Sprite y InfoPlayer tienen posiciones que se tendran que actualizar a la vez
	characterSprite = sf::Sprite(characterTexture);
	characterSprite.setPosition(myPlayer.position.x+10, myPlayer.position.y+50);

	//sf::Clock gameClock;
	gameClock.restart();
	while (window.isOpen())
	{
		deltaSeconds = gameClock.restart();

		sf::Event event;
		while (window.pollEvent(event))	//TODO: esborrar o netejar/organitzar
		{
			//Cerrar la ventana
			if (event.type == sf::Event::Closed)
				window.close();
			//Detectar eventos de teclado
			if (event.type == sf::Event::KeyPressed && (event.key.code == sf::Keyboard::Escape))
				window.close();
			//Detectar eventos de ratón
			if (event.type == sf::Event::MouseButtonPressed) {
				std::cout << "Mouse Pressed at position: " << event.mouseButton.x << ":"
					<< event.mouseButton.y << std::endl;
				/*if (Game::currentTurn == miTurno)
				{
					sendMove(event.mouseButton.x, event.mouseButton.y);
				}*/
				//sf::Mouse::getPosition(window)
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

		//recieveFromServer();
		sendInputMovement();
		



		window.clear();

		//Dibujar el mapa i jugadores
		window.draw(mapShape);
		//Nuestro jugador
		window.draw(characterSprite);
		

		/*for (int i = 0; i < MAXPLAYERS; i++)
		{
			characterSprite.setPosition(sf::Vector2f(jugadores.at(i).position*TILESIZE));
			if (jugadores.at(i).isDead)
			{
				characterSprite.rotate(-90);
				characterSprite.move(sf::Vector2f(0, TILESIZE));
			}
			window.draw(characterSprite);
			characterSprite.setRotation(0);
			window.draw(jugadores.at(i).nameText);
			window.draw(gameResult);
		}*/

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

void sendInputMovement()
{
	float speed = 1.f; //Pixels / ms
	//O se envia left, o se envia right, nunca los dos a la vez
	if (sf::Keyboard::isKeyPressed(sf::Keyboard::Left)) {
		//characterSprite.move(-speed*deltaSeconds.asMilliseconds(), 0);

		//Enviar al servidor
		sf::Packet movePack;
		movePack << (sf::Uint8)Cabeceras::MOVE_LEFT;
		socket.send(movePack, IPSERVER, PORTSERVER);

	} else if (sf::Keyboard::isKeyPressed(sf::Keyboard::Right)) {
		//characterSprite.move(speed*deltaSeconds.asMilliseconds(), 0);

		//Enviar al servidor
		sf::Packet movePack;
		movePack << (sf::Uint8)Cabeceras::MOVE_RIGHT;
		socket.send(movePack, IPSERVER, PORTSERVER);
	}

	if (sf::Keyboard::isKeyPressed(sf::Keyboard::Up)) {
		//characterSprite.move(0, -speed*deltaSeconds.asMilliseconds());

		//Enviar al servidor
		sf::Packet movePack;
		movePack << (sf::Uint8)Cabeceras::MOVE_UP;
		socket.send(movePack, IPSERVER, PORTSERVER);
	} else if (sf::Keyboard::isKeyPressed(sf::Keyboard::Down)) {
		//characterSprite.move(0, speed*deltaSeconds.asMilliseconds());
		
		//Enviar al servidor
		sf::Packet movePack;
		movePack << (sf::Uint8)Cabeceras::MOVE_DOWN;
		socket.send(movePack, IPSERVER, PORTSERVER);
	}
}

void recieveFromServer()
{
	sf::Packet serverPacket;
	sf::IpAddress ip;
	unsigned short port;
	//Suponemos que solo recibiremos del servidor
	sf::UdpSocket::Status status = socket.receive(serverPacket, ip,port);
	switch (status)
	{
	case sf::Socket::Done:
	{
		std::cout << "recieved" << std::endl;
		int comandoInt;
		serverPacket >> comandoInt;
		Cabeceras comando = (Cabeceras)comandoInt;
		switch (comando)
		{
		case ACKNOWLEDGE:
			break;
		case NEW_PLAYER:
			//Mostrar mensaje por pantalla?
			std::cout << "New player has connected";
			break;
		case OK_POSITION:		//TODO-- enviar id jugador
			sf::Uint32 newPosX, newPosY;
			serverPacket >> newPosX;
			serverPacket >> newPosY;
			characterSprite.setPosition(newPosX, newPosY);

			break;
		default:
			break;
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
