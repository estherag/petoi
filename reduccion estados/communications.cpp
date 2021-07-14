//Server Socket
#pragma once
#pragma comment (lib, "Ws2_32.lib") // Need to link with Ws2_32.lib

#undef UNICODE

#define WIN32_LEAN_AND_MEAN
#define DEFAULT_PORT "8000"

#include <windows.h>
#include <winsock2.h>
#include <ws2tcpip.h>
#include <stdlib.h>
#include <stdio.h>
#include <iostream>
#include <vector>
#include "communications.h"


using namespace std;
extern SOCKET ClientSocket;

//Inicio de comunicacion
//Entradas: ninguna
//Salidas: un entero "1" si comunicacion ok "0" error
int startCommunication()
{
	//Define estructura que contiene informacion sobre la 
	//implementacion de sockets en windows
	WSADATA wsaData;
	//Define variable entero
	int iResult;

	//Define socket listen y lo asigna como tipo no asignado (INVALID_SOCKET=0)
	SOCKET ListenSocket = INVALID_SOCKET;
	//Asigna socket cliente como tipo no asignado (INVALID_SOCKET=0)
	ClientSocket = INVALID_SOCKET;

	//se define puntero results igual a 0, su indireccion representa ka esrructura addrinfo
	struct addrinfo* result = NULL;
	//se define estructura addtinfo: hints 
	struct addrinfo hints;

	//Escritura en la terminal
	cout << "Starting server" << endl;
	
	//Inicializacion de Winsock
	iResult = WSAStartup(MAKEWORD(2, 2), &wsaData);

	if (iResult != 0) {
		printf("WSAStartup failed with error: %d\n", iResult);
		return 0;
	}

	//Llena un bloque de memoria con ceros en la direccion inicial de hits
	ZeroMemory(&hints, sizeof(hints));

	//Define informacion de la estructura 
	hints.ai_family = AF_INET;
	hints.ai_socktype = SOCK_STREAM;
	hints.ai_protocol = IPPROTO_TCP;
	hints.ai_flags = AI_PASSIVE;

	// Resuleve direccion y puerto de servidor
	iResult = getaddrinfo(NULL, DEFAULT_PORT, &hints, &result);
	if (iResult != 0) {
		printf("getaddrinfo failed with error: %d\n", iResult);
		WSACleanup();
		return 0;
	}

	// Creacion de socket para conectar con el cliente
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

	//libera estructuras addrinfo
	freeaddrinfo(result);

	// Socket servidor en espera de una conexion 
	iResult = listen(ListenSocket, SOMAXCONN);
	if (iResult == SOCKET_ERROR) {
		printf("listen failed with error: %d\n", WSAGetLastError());
		closesocket(ListenSocket);
		WSACleanup();
		return 0;
	}
	printf("socket LISTENING");
	
	// Permite intento de conexion entrante con el socket cliente
	ClientSocket = accept(ListenSocket, NULL, NULL);
	if (ClientSocket == INVALID_SOCKET) {
		printf("accept failed with error: %d\n", WSAGetLastError());
		closesocket(ListenSocket);
		WSACleanup();
		return 0;
	}
	cout << "Communication started" << endl;
	
	// Cierra socket existente
	closesocket(ListenSocket);
	return 1;
	
}

//Envio de mensaje
//Entradas: vector de char sendVector
//Salidas: ninguna
void sendMessage(vector <unsigned char> sendVector)
{
	int iSendResult;
	char sendbuf[2];

	for (int i = 0; i < 2; i++)
	{
		sendbuf[i] = sendVector[i];
	}

	//Se envia datos "sendbuf" al socket conectado
	iSendResult = send(ClientSocket, sendbuf, sizeof(sendbuf), 0);
	cout << "Sending" << endl;
	
	//Se imprime en pantalla mensaje enviado
	for (int j = 0; j < 2; j++)
	{
		cout << (int)sendbuf[j] << ", ";
	}
	cout << endl;

	return;
}

//Lectura de mensaje
//Entradas: ninguna
//Salidas: entero message con valor de los 11 sensores
vector <int> readMessage()
{
	//inicializa mensaje con 0's
	vector <int> message = { 0,0,0,0,0,0,0,0,0,0,0 };
	
	//Define variables locales
	char recvbuf[44]; // 44: 11 sensores de tipo entero, 4 bytes por entero
	int recvbuflen = sizeof(recvbuf);
	int iResult = 0, i = 0;

	//Recibir datos del cliente
	iResult = recv(ClientSocket, recvbuf, sizeof(recvbuf), 0);
	printf("Bytes received: %d\n", iResult);
	printf("Data received: \n");
	
	//Almacenar datos en vector mensaje
	for (int j = 0; j < sizeof(recvbuf); j = j + 4)
	{
		unsigned char bytes[4] = { reinterpret_cast<unsigned char&>(recvbuf[j]),reinterpret_cast<unsigned char&>(recvbuf[j + 1]),reinterpret_cast<unsigned char&>(recvbuf[j + 2]),reinterpret_cast<unsigned char&>(recvbuf[j + 3]) };
		int value;
		//copia bytes en la ubicacion apuntada 
		memcpy(&value, bytes, sizeof(int)); // cadena de 4 char desde raspberry -> 1 entero

		cout << value;
		cout << ", ";
		message[i] = value;
		i++;
	}
	cout << endl;

	return message;
}

//Cerrar conexion
//Entradas: ninguna
//Salidas: ninguna
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