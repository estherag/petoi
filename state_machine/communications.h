#pragma once
#include <winsock2.h>
#include <vector>
using namespace std;
extern SOCKET ClientSocket;
int startCommunication();
void sendMessage(vector <unsigned char> sendVector);
vector <int> readMessage();
void closeSocket();