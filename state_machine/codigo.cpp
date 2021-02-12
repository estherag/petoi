#pragma once
#include <WinSock2.h>
#include "catClass.h"
#include "communications.h"
#include "functions.h"
#include <fstream>
#include <sstream>
#include <iostream>
#include <vector>
#include <algorithm>
#include <stdlib.h>     /* srand, rand */
#include <time.h>       /* time */
using namespace std;

SOCKET ClientSocket;
int main(){
	srand(time(NULL));
	if (startCommunication() == 0) { return 1; }  // si el sistema no conecta, finalizar
	/*
	cout << "explore" << endl;
	sendMessage({ 2, 0 });
	sendMessage({ 2, 1 });
	sendMessage({ 2, 2 });
	sendMessage({ 2, 3 });
	sendMessage({ 2, 4 });
	sendMessage({ 2, 5 });
	sendMessage({ 2, 6 });
	sendMessage({ 2, 4 });
	sendMessage({ 2, 7 });
	sendMessage({ 2, 18 });
	getchar();
	
	cout << "hunt" << endl;
	sendMessage({ 2, 8 });
	sendMessage({ 2, 9 });
	sendMessage({ 2, 10 });
	sendMessage({ 2, 11 });
	sendMessage({ 2, 18 });
	sendMessage({ 2, 12 });
	sendMessage({ 2, 4 });
	sendMessage({ 2, 7 });
	getchar();
	
	cout << "groom" << endl;
	sendMessage({ 2, 13 });
	sendMessage({ 2, 14 });
	sendMessage({ 2, 3 });
	sendMessage({ 2, 5 });
	sendMessage({ 2, 3 });
	sendMessage({ 2, 5 });
	sendMessage({ 2, 3 });
	sendMessage({ 2, 5 });
	sendMessage({ 2, 13 });
	sendMessage({ 2, 3 });
	sendMessage({ 2, 5 });
	sendMessage({ 2, 3 });
	sendMessage({ 2, 5 });
	sendMessage({ 2, 3 });
	sendMessage({ 2, 5 });
	sendMessage({ 2, 3 });
	getchar();
	
	cout << "attention" << endl;
	sendMessage({ 2, 15 });
	sendMessage({ 2, 4 });
	sendMessage({ 2, 7 });
	sendMessage({ 2, 15 });
	sendMessage({ 2, 4 });
	sendMessage({ 2, 7 });
	sendMessage({ 2, 15 });
	sendMessage({ 2, 4 });
	sendMessage({ 2, 7 });
	getchar();
	
	cout << "sleep" << endl;
	sendMessage({ 2, 13 });
	sendMessage({ 2, 16 });	
	sendMessage({ 2, 3 });	
	sendMessage({ 2, 17 });
	getchar();
	
	cout << "play" << endl;
	sendMessage({ 2, 9 });
	sendMessage({ 2, 10 });
	sendMessage({ 2, 11 });
	sendMessage({ 2, 4 });
	sendMessage({ 2, 12 });
	sendMessage({ 2, 4 });
	sendMessage({ 2, 7 });
	sendMessage({ 2, 4 });
	sendMessage({ 2, 7 });		
	sendMessage({ 2, 4 });
	getchar();

	*/
	//maquina estados
	vector<int> sensor;
	for (int i = 0; i < Nsensor; i++)
	{
		sensor.push_back(0);
	}
	vector<vector<double>> reactionInitial(Nstates, vector<double>(Nstates));
	reactionInitial = readReaction("files/reactions.csv");
	vector<double> personality(Npersonality);
	personality = readPersonality("files/personality.csv");
	reactionInitial = adjustProbability(reactionInitial);
	//printMatrix(reactionInitial);
	catState gato(reactionInitial, reactionInitial, sensor, personality);
	generalState maquina({ 0,0 }, { 0,0 }, {-100,40,40,7,7,7,7,7,7,7,7}, reactionInitial);
	//gato.readSensors(); 
	//gato.setSensors({0,0,0,1,0,0,0,0,0,0,0 });
	
	while (1) //ejecutar maquina estados
	{
		gato.setSensors(gato.readSensors());
		maquina.checkTransitionState(gato.getSensors(), gato.getCurrentReaction(), gato.getInitialReaction());
		do
		{
			gato.setSensors(gato.readSensors());
			maquina.checkTransitionSubState(gato.getSensors());
			maquina.changeState();
			gato.setCurrentReaction(maquina.entryActivity(maquina.getCurrentState(), gato.getCurrentReaction(), gato.getInitialReaction()));
			//Sleep(1000);
		} while (maquina.getCurrentState()[1] != 0);
	}
	
	cout << "Fin";
	return 0;
}

