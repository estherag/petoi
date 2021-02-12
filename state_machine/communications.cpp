#pragma once
#undef UNICODE
#define WIN32_LEAN_AND_MEAN
#include <windows.h>
#include <winsock2.h>
#include <ws2tcpip.h>
#include <stdlib.h>
#include <stdio.h>
#include "communications.h"
#include <iostream>
#include <vector>
#pragma comment (lib, "Ws2_32.lib") // Need to link with Ws2_32.lib

#define DEFAULT_PORT "8000"
using namespace std;
extern SOCKET ClientSocket;

int startCommunication()
{
	WSADATA wsaData;
	int iResult;

	SOCKET ListenSocket = INVALID_SOCKET;
	ClientSocket = INVALID_SOCKET;

	struct addrinfo* result = NULL;
	struct addrinfo hints;
	cout << "Starting server" << endl;
	// Initialize Winsock
	iResult = WSAStartup(MAKEWORD(2, 2), &wsaData);

	if (iResult != 0) {
		printf("WSAStartup failed with error: %d\n", iResult);
		return 0;
	}

	ZeroMemory(&hints, sizeof(hints));
	hints.ai_family = AF_INET;
	hints.ai_socktype = SOCK_STREAM;
	hints.ai_protocol = IPPROTO_TCP;
	hints.ai_flags = AI_PASSIVE;

	// Resolve the server address and port
	iResult = getaddrinfo(NULL, DEFAULT_PORT, &hints, &result);
	if (iResult != 0) {
		printf("getaddrinfo failed with error: %d\n", iResult);
		WSACleanup();
		return 0;
	}
	// Create a SOCKET for connecting to server
	ListenSocket = socket(result->ai_family, result->ai_socktype, result->ai_protocol);
	if (ListenSocket == INVALID_SOCKET) {
		printf("socket failed with error: %ld\n", WSAGetLastError());
		freeaddrinfo(result);
		WSACleanup();
		return 0;
	}

	// Setup the TCP listening socket
	iResult = bind(ListenSocket, result->ai_addr, (int)result->ai_addrlen);

	if (iResult == SOCKET_ERROR) {
		printf("bind failed with error: %d\n", WSAGetLastError());
		freeaddrinfo(result);
		closesocket(ListenSocket);
		WSACleanup();
		return 0;
	}

	freeaddrinfo(result);

	iResult = listen(ListenSocket, SOMAXCONN);
	if (iResult == SOCKET_ERROR) {
		printf("listen failed with error: %d\n", WSAGetLastError());
		closesocket(ListenSocket);
		WSACleanup();
		return 0;
	}

	// Accept a client socket
	ClientSocket = accept(ListenSocket, NULL, NULL);
	if (ClientSocket == INVALID_SOCKET) {
		printf("accept failed with error: %d\n", WSAGetLastError());
		closesocket(ListenSocket);
		WSACleanup();
		return 0;
	}
	cout << "Communication started" << endl;
	closesocket(ListenSocket);
	return 1;
}
void sendMessage(vector <unsigned char> sendVector)
{
	int iSendResult;
	char sendbuf[2];

	for (int i = 0; i < 2; i++)
	{
		sendbuf[i] = sendVector[i];
	}

	iSendResult = send(ClientSocket, sendbuf, sizeof(sendbuf), 0);
	cout << "Sending" << endl;
	for (int j = 0; j < 2; j++)
	{
		cout << (int)sendbuf[j] << ", ";
	}
	cout << endl;
	return;
}
vector <int> readMessage()
{
	vector <int> message = { 0,0,0,0,0,0,0,0,0,0,0 };
	//Recieve data from client
	char recvbuf[44]; // 44: 11 sensores de tipo entero, 4 bytes por entero
	int recvbuflen = sizeof(recvbuf);
	int iResult = 0, i = 0;

	iResult = recv(ClientSocket, recvbuf, sizeof(recvbuf), 0);
	printf("Bytes received: %d\n", iResult);
	printf("Data received: \n");
	for (int j = 0; j < sizeof(recvbuf); j = j + 4)
	{
		unsigned char bytes[4] = { reinterpret_cast<unsigned char&>(recvbuf[j]),reinterpret_cast<unsigned char&>(recvbuf[j + 1]),reinterpret_cast<unsigned char&>(recvbuf[j + 2]),reinterpret_cast<unsigned char&>(recvbuf[j + 3]) };
		int value;
		memcpy(&value, bytes, sizeof(int)); // cadena de 4 char desde raspberry -> 1 entero

		cout << value;
		cout << ", ";
		message[i] = value;
		i++;
	}
	cout << endl;

	return message;
}
void closeSocket()
{
	// shutdown the connection since we're done
	int iResult = shutdown(ClientSocket, SD_SEND);
	if (iResult == SOCKET_ERROR) {
		printf("shutdown failed with error: %d\n", WSAGetLastError());
		closesocket(ClientSocket);
		WSACleanup();
		return;
	}
	// cleanup
	closesocket(ClientSocket);
	WSACleanup();
	return;
}