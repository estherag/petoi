//Para asegurar que el codigo fuente que lo invoca sea incluido una sola vez
#pragma once

//Para asegurar que el codigo fuente que lo invoca sea incluido una sola vez
#include <winsock2.h>
#include <vector>

//Region declarativa para organizar codigo en grupos logicos y evitar colisiones de nombres
using namespace std;

//Variable socket que puede ser llamada en diferentes archivos (extern type)
extern SOCKET ClientSocket;

//Metodo de inicio de comunicacion, retorna un entero 
int startCommunication();

//Metodo de envio de mensaje con parametros de entrada
void sendMessage(vector <unsigned char> sendVector);

//Metodo de cierre de comunicacion 
void closeSocket();

//Metodo de lectura de mensajes, retorna un vector de 1-D de enteros
vector <int> readMessage();

