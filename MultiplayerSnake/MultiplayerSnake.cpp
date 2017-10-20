// MultiplayerSnake.cpp : Defines the entry point for the console application.
//

#include "stdafx.h"
#include <SDL.h>
#include <iostream>
#include <string>

#include <vector>
#include "SDL2net\include\SDL_net.h"
#include <SDL_ttf.h>
#include <sstream>
const int SCREEN_WIDTH = 1000;
const int SCREEN_HEIGHT = 1000;
const int GRID_DIVIDER = 2;
const int FPS = 60;
const int SCREEN_TICKS_PER_FRAME = 1000 / FPS;


const int gridHeight = 50;
const int gridWidth = 50;
using namespace std;

struct vec2
{
	int x;
	int y;
	vec2() {};
	vec2(int x, int y) : x(x), y(y) {};

	friend inline bool operator==(const vec2 lhs, const vec2 rhs) { return(lhs.x == rhs.x && lhs.y == rhs.y); }
};

enum constantDirection
{
	UP,
	DOWN,
	LEFT,
	RIGHT
};
enum dataType
{
	NONE,
	BODY,
	BIP,
	SERVER
};

int grid[gridHeight][gridWidth];

#define PORT 2000

struct packet
{
	int id;
	dataType dataType = NONE;
	int bodylength;
	vec2 body[100];
};

struct handshake
{
	string st = "hello";
	int clientIp;
};

struct player
{
	TCPsocket socket;
	handshake hs;
	std::vector<vec2> playerBody;
	packet playerPacket;

	player() {};
	player(TCPsocket _socket) : socket(_socket) {};
};

vec2 bipLocation;

constantDirection playerDirection = UP;

/**
* Log an SDL error with some error message to the output stream of our choice
* @param os The output stream to write the message to
* @param msg The error message to write, format will be msg error: SDL_GetError()
*/
void logSDLError(std::ostream &os, const std::string &msg) {
	os << msg.c_str() << " error: " << SDL_GetError() << std::endl;
}

std::vector<player> otherplayers;

SDL_Event e;


int main(int, char**) {

	if (SDL_Init(SDL_INIT_EVERYTHING) != 0) {
		logSDLError(std::cout, "SDL_Init");
		return 1;
	}

	//SERVER
	bool isServer = false;
	TCPsocket server = 0;
	TCPsocket client;
	vector<player> clients;
	float serverTickRate = 60;
	float serverTickTimer = -1;
	SDLNet_Init();

	//CLIENT
	IPaddress ip; //90.253.170.185
	SDLNet_ResolveHost(&ip, "127.0.0.1", PORT);
	server = SDLNet_TCP_Open(&ip);
	handshake hs;
	
	if (server)
	{
		int ret = SDLNet_TCP_Recv(server, &hs, sizeof(handshake));
		cout << "handshake :" << sizeof(handshake) << endl;
		cout << "Received :" << ret << endl;
		std::cout << "Connected to " << SDLNet_TCP_GetPeerAddress(server)->host << std::endl;

		std::cout << hs.st << std::endl;
	}
	else
	{
		cout << "Creating Server..." << endl;
		char res;
		//cin >> res;
		if (1)
		{
			isServer = true;
			//SERVER
			IPaddress ip; //90.253.170.185
			SDLNet_ResolveHost(&ip, NULL, PORT);

			server = SDLNet_TCP_Open(&ip);
		}
	}

	bool isGrowing = false;
	float movementCooldownTimer = -1;
	float previousTime = 0;
	bool quit = false;
	cout << "Server Created." << endl;
	cout << "Server Running..." << endl;


	while (!quit) {

		//system("cls");

		while (SDL_PollEvent(&e)) {
			if (e.type == SDL_QUIT) {
				quit = true;
			}
			//If user presses esc
			if (e.key.keysym.sym == SDLK_ESCAPE) {
				quit = true;
			}
		}

		//accept new players
		TCPsocket client = SDLNet_TCP_Accept(server);
		if (client)
		{
			clients.push_back(player(client));
			handshake hs;
			clients.back().hs.clientIp = SDLNet_TCP_GetPeerAddress(client)->host;
			int ret = SDLNet_TCP_Send(client, &clients.back().hs, sizeof(handshake));
			cout << "Sent :" << ret << endl;
			cout << "New player detected : " << clients.back().hs.clientIp << endl;
		}

		packet data;

		if (!clients.empty())
		{
			//recieve packets
			for each (player clis in clients)
			{
				int ret = SDLNet_TCP_Recv(clis.socket, &clis, sizeof(packet));
					cout << "Received :" << ret << endl;

				if (ret != -1)
				{
					cout << "Received :" << ret << endl;

					cout << "Packet Received from " << clis.hs.clientIp << endl;
					cout << data.id << endl;
					cout << data.dataType << endl;
					cout << data.body[0].x << " " << data.body[0].y << endl;

					//new bip requested
					if (data.dataType == dataType::BIP)
					{
						cout << "    Bip requested" << endl;
						packet updatedBip;
						bipLocation = vec2(rand() % 20, rand() % 20);
						updatedBip.dataType = SERVER;
						updatedBip.body[0] = bipLocation;
						for each (player cli in clients)
						{
							int ret = SDLNet_TCP_Send(cli.socket, &updatedBip, sizeof(packet));
							cout << "Sent new bip location to:" << cli.hs.clientIp << endl;
							cout << "Sent :" << ret << endl;
						}


						////updated body
						//if (data.dataType == dataType::BODY)
						//{
						//	cout << "    Body Updated" << endl;

						//	vector<vec2> cliBody;
						//	for each (player var in clients)
						//	{
						//		if (var.hs.clientIp == data.id)
						//		{
						//			var.playerBody.clear();
						//			for each (vec2 location in data.body)
						//			{
						//				var.playerBody.push_back(location);
						//			}
						//		}
						//	}
						//}
					}
					cout << "Packet Closed from " << clis.hs.clientIp << endl;

				}
			}
		}
	}
	//SDLNet_TCP_Close(server);

	SDL_Quit();
	return 0;
}

