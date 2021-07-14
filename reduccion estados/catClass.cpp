#pragma once

#include "catClass.h"
#include "communications.h"
#include "generalState.h"
#include <iostream>
#include <winsock2.h>


using namespace std;
extern SOCKET ClientSocket;
#include <vector>

// Constructor por defecto de la clase catState
catState::catState(void)
{
	// loop i<8
	for (size_t i = 0; i < Nstates; i++)
	{
		// vector 1-D tempVector
		vector<double> tempVector;

		// loop i<8
		for (size_t j = 0; j < (Nstates); j++)
		{
			// agrega el valor "0" en la parte posterior del vector
			tempVector.push_back(0);
		}

		// agrega el valor "tempVector" en la parte posterior del vector
		catState::currentReaction.push_back(tempVector);
		// agrega el valor "tempVector" en la parte posterior del vector
		catState::initialReaction.push_back(tempVector);
	}


	// loop i<11
	for (size_t i = 0; i < Nsensor; i++)
	{
		// agrega el valor "0" en la parte posterior del vector sensorReading
		sensorReading.push_back(0);

		//si i < 4
		if (i < Npersonality)
		{
			// agrega el valor "0" en la parte posterior del vector personality
			personality.push_back(0);
		}
	}
}


//Constructor parametrizado de la clase catState
catState::catState(vector< vector<double>> initReaction, vector< vector<double> > currReaction, vector<int>  sensor, vector<double>  pers)
{
	// loop i<8 inicializa nuevamenye valores en cero de initial y current reaction
	for (size_t i = 0; i < Nstates; i++)
	{
		// vector 1-D tempVector
		vector<double> tempVector;
		
		// loop i<8
		for (size_t j = 0; j < (Nstates); j++)
		{
			// agrega el valor 0 en la parte posterior del vector
			tempVector.push_back(0);
		}

		// agrega el valor "tempVector" en la parte posterior del vector
		catState::currentReaction.push_back(tempVector);
		catState::initialReaction.push_back(tempVector);
	}

	// loop i<11 agrega valores de sensor y personalidad
	for (size_t i = 0; i < Nsensor; i++)
	{
		// agrega el valor sensor[i] en la parte posterior del vector sensorReading
		sensorReading.push_back(sensor[i]);
		if (i < Npersonality)
		{
			// agrega el valor "pers[i]" en la parte posterior del vector personality
			personality.push_back(pers[i]);
		}
	}

	// loop i<8 agrega valores de initial y current reaction
	for (size_t i = 0; i < Nstates; i++)
	{
		// loop i<8
		for (size_t j = 0; j < (Nstates); j++)
		{
			//Agrega valores de initial y current reaction
			catState::initialReaction[i][j] = initReaction[i][j];
			catState::currentReaction[i][j] = currReaction[i][j];
		}
	}

}

// Funcion restaurar tabla 3 con valores iniciales
void catState::resetReaction()
{
	size_t i, j;
	// loop i<8
	for (i = 0; i < Nstates; i++)
	{
		// loop i<8
		for (j = 0; j < (Nstates); j++)
		{
			// current reaction = initial reaction
			catState::currentReaction[i][j] = catState::initialReaction[i][j];
		}
	}

}

// Funcion lectura sensores
vector<int> catState::readSensors()
{
	// Envia mensajen a raspberry y solicita lectura de sensores
	sendMessage({ 4,0 });
	// Lee datos recibidos de raspberry
	catState::sensorReading==readMessage();
	//Funcion retorna "sensoReading"
	return catState::sensorReading;
}


//Cambio valor initialReaction
void catState::setInitialReaction(vector< vector<double> > initReact)
{
	for (size_t i = 0; i < initReact.size(); ++i)
	{
		for (size_t j = 0; j < initReact[i].size(); ++j)
		{
			catState::initialReaction[i][j] = initReact[i][j];
		}
	}

}

//Cambio valor currentReaction
void catState::setCurrentReaction(vector< vector<double> > currReact)
{
	for (size_t i = 0; i < currReact.size(); ++i)
	{
		for (size_t j = 0; j < currReact[i].size(); ++j)
		{
			catState::currentReaction[i][j] = currReact[i][j];
		}
	}
}

//Cambio valor currentReaction a initialReaction
vector<vector<double>> catState::restoreCurrentReaction()
{

	for (size_t i = 0; i < catState::currentReaction.size(); ++i)
	{
		for (size_t j = 0; j < catState::currentReaction[i].size(); ++j)
		{
			catState::currentReaction[i][j] = catState::initialReaction[i][j];
		}
	}
	return catState::currentReaction;
}

//Cambio valor personality
void catState::setPersonality(vector<double> pers)
{
	for (int i = 0; i < Npersonality; i++)
	{
		catState::personality[i] = pers[i];
	}
}

//Cambio valor sensorReading
void catState::setSensors(vector<int> sens)
{
	for (size_t i = 0; i < sensorReading.size(); ++i)
	{
		catState::sensorReading[i] = sens[i];
	}
}

//Destructor:  función miembro que destruye o elimina un objeto
catState::~catState()
{
}
