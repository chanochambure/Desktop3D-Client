#ifndef INCLUDED_NETWORK_H
#define INCLUDED_NETWORK_H

#include "LexRisLogic\include\LexRisLogic\Allegro5\Allegro5.h"
#include "LexRisLogic\include\LexRisLogic\Allegro5\Color.h"
#include "LexRisLogic\include\LexRisLogic\Allegro5\Display.h"
#include "LexRisLogic\include\LexRisLogic\Allegro5\Input.h"
#include "LexRisLogic\include\LexRisLogic\Allegro5\Text.h"
#include "LexRisLogic\include\LexRisLogic\Allegro5\Primitives.h"
#include "LexRisLogic\include\LexRisLogic\Allegro5\Special\Interface.h"

#include "LexRisLogic\include\LexRisLogic\Math.h"

#include <winsock2.h>
#include <windows.h>
#include <ws2tcpip.h>
#include <process.h>

struct Network
{
	LL_AL5::Button* network_conn_button = nullptr;
	LL_AL5::Button* network_server_button = nullptr;
	SOCKET socket_server;
	SOCKET socket_cliente;
	bool server_enable = false;
	bool user_connected = false;
	void disable_cliente()
	{
		if (network_conn_button)
		{
			network_conn_button->set_unclick_fill_color(LL_AL5::Color(255));
			network_conn_button->set_click_fill_color(LL_AL5::Color(255));
			network_conn_button->set_button_text("Cliente Desconectado ");
		}
	}
	void disable_servidor()
	{
		if (network_server_button)
		{
			network_server_button->set_unclick_fill_color(LL_AL5::Color(255));
			network_server_button->set_click_fill_color(LL_AL5::Color(255));
			network_server_button->set_button_text("Servicio Apagado     ");
		}
	}
	void enable_cliente()
	{
		if (network_conn_button)
		{
			network_conn_button->set_unclick_fill_color(LL_AL5::Color(0, 255));
			network_conn_button->set_click_fill_color(LL_AL5::Color(0, 255));
			network_conn_button->set_button_text("Cliente Conectado    ");
		}
	}
	void enable_servidor()
	{
		if (network_server_button)
		{
			network_server_button->set_unclick_fill_color(LL_AL5::Color(0, 255));
			network_server_button->set_click_fill_color(LL_AL5::Color(0, 255));
			network_server_button->set_button_text("Servicio Ejecutandose");
		}
	}
	void create_conn(LL_AL5::Input* input, LL_AL5::Font* font, float pos_x, float pos_y)
	{
		network_conn_button = new LL_AL5::Button(input);
		network_conn_button->set_font(font);
		network_conn_button->set_pos(pos_x, pos_y);
	}
	void create_server(LL_AL5::Input* input, LL_AL5::Font* font, float pos_x, float pos_y)
	{
		network_server_button = new LL_AL5::Button(input);
		network_server_button->set_font(font);
		network_server_button->set_pos(pos_x, pos_y);
	}
	void draw(LL_AL5::Display* display)
	{
		if (user_connected)
			enable_cliente();
		else
			disable_cliente();
		display->draw(network_conn_button);
		if (server_enable)
			enable_servidor();
		else
			disable_servidor();
		display->draw(network_server_button);
	}
	std::string start_server(std::string ip, std::string port)
	{
		if (!ip.size())
			return "Debe ingresar una IP";
		if (!port.size())
			return "Debe ingresar el Puerto";
		std::stringstream streamer;
		// create WSADATA object
		WSADATA wsaData;
		// our sockets for the server
		socket_server = INVALID_SOCKET;
		// address info for the server to listen to
		struct addrinfo *result = NULL;
		struct addrinfo hints;
		// Initialize Winsock
		int iResult = WSAStartup(MAKEWORD(2, 2), &wsaData);
		if (iResult != 0)
		{
			streamer << "WSAStartup failed with error: " << iResult;
			return streamer.str();
		}
		// set address information
		ZeroMemory(&hints, sizeof(hints));
		hints.ai_family = AF_INET;
		hints.ai_socktype = SOCK_STREAM;
		// TCP connection!!!
		hints.ai_protocol = IPPROTO_TCP;
		hints.ai_flags = AI_PASSIVE;
		// Resolve the server address and port
		if (ip.size() == 0)
			iResult = getaddrinfo(NULL, port.c_str(), &hints, &result);
		else
			iResult = getaddrinfo(ip.c_str(), port.c_str(), &hints, &result);
		if (iResult != 0)
		{
			streamer << "getaddrinfo failed with error: " << iResult;
			WSACleanup();
			return streamer.str();
		}
		// Create a SOCKET for connecting to server
		socket_server = socket(result->ai_family, result->ai_socktype, result->ai_protocol);
		if (socket_server == INVALID_SOCKET)
		{
			streamer << "socket failed with error: " << WSAGetLastError();
			freeaddrinfo(result);
			WSACleanup();
			return streamer.str();
		}
		// Set the mode of the socket to be nonblocking
		u_long iMode = 1;
		iResult = ioctlsocket(socket_server, FIONBIO, &iMode);
		if (iResult == SOCKET_ERROR)
		{
			streamer << "ioctlsocket failed with error: " << WSAGetLastError();
			closesocket(socket_server);
			WSACleanup();
			return streamer.str();
		}
		// Setup the TCP listening socket
		iResult = bind(socket_server, result->ai_addr, (int)result->ai_addrlen);
		if (iResult == SOCKET_ERROR)
		{
			streamer << "bind failed with error: " << WSAGetLastError();
			freeaddrinfo(result);
			closesocket(socket_server);
			WSACleanup();
			return streamer.str();
		}
		// no longer need address information
		freeaddrinfo(result);
		// start listening for new clients attempting to connect
		iResult = listen(socket_server, SOMAXCONN);
		if (iResult == SOCKET_ERROR)
		{
			streamer << "listen failed with error: " << WSAGetLastError();
			closesocket(socket_server);
			WSACleanup();
			return streamer.str();
		}
		server_enable = true;
		return "";
	}
	void stop_server()
	{
		if (server_enable)
		{
			if (user_connected)
			{
				closesocket(socket_cliente);
				user_connected = false;
			}
			closesocket(socket_server);
			WSACleanup();
			server_enable = false;
		}
	}
	void update_user()
	{
		SOCKET socket = getConnection();
		if (socket != INVALID_SOCKET)
		{
			if (user_connected)
				closesocket(socket);
			else
			{
				user_connected = true;
				socket_cliente = socket;
			}
		}
	}
	SOCKET getConnection()
	{
		if (server_enable)
		{
			// if client waiting, accept the connection and save the socket
			SOCKET ClientSocket = accept(socket_server, NULL, NULL);
			if (ClientSocket != INVALID_SOCKET)
			{
				//disable nagle on the client's socket
				char value = 1;
				setsockopt(ClientSocket, IPPROTO_TCP, TCP_NODELAY, &value, sizeof(value));
				DWORD timeout = 1000;
				setsockopt(ClientSocket, SOL_SOCKET, SO_RCVTIMEO, (char*)&timeout, sizeof(timeout));
				u_long iMode = 0;
				ioctlsocket(ClientSocket, FIONBIO, &iMode);
				return ClientSocket;
			}
		}
		return INVALID_SOCKET;
	}
	void sendPacketObs(int iResult)
	{
		if (iResult == SOCKET_ERROR)
		{
			int error = WSAGetLastError();
			if (error == WSAENETDOWN || error == WSAECONNRESET || error == WSAECONNABORTED)
			{
				closesocket(socket_cliente);
				user_connected = false;
			}
		}
		else if (iResult == 0)
		{
			closesocket(socket_cliente);
			user_connected = false;
		}
	}
	template<typename T>
	void sendData(T* message)
	{
		if (user_connected)
			sendPacketObs(send(socket_cliente, (char*)message, sizeof(T), 0));
	}
	template<typename T>
	void sendListData(std::list<T>& send_list) {
		int bytes_size = send_list.size() * sizeof(T);
		char* data = new char[bytes_size];
		int counter = 0;
		for (float& number : send_list)
		{
			memcpy(&data[counter], &number, sizeof(T));
			counter += sizeof(T);
		}
		sendPacketObs(send(socket_cliente, data, bytes_size, 0));
	}
	template<typename T>
	void recvData(T* buffer)
	{
		if (user_connected)
			int iResult = (recv(socket_cliente, (char*)buffer, sizeof(T), 0));
	}
	~Network()
	{
		stop_server();
		if (network_conn_button)
			delete(network_conn_button);
		network_conn_button = nullptr;
		if (network_server_button)
			delete(network_server_button);
		network_server_button = nullptr;
	}
};

#endif // INCLUDED_NETWORK_H
