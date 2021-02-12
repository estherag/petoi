#pragma once
#include "catClass.h"
#include "communications.h"
#include "generalState.h"
#include <iostream>
#include <winsock2.h>
using namespace std;
extern SOCKET ClientSocket;
#include <vector>

catState::catState(void)
{
	for (size_t i = 0; i < Nstates; i++)
	{
		vector<double> tempVector;
		for (size_t j = 0; j < (Nstates); j++)
		{
			tempVector.push_back(0);
		}
		catState::currentReaction.push_back(tempVector);
		catState::initialReaction.push_back(tempVector);
	}
	for (size_t i = 0; i < Nsensor; i++)
	{
		sensorReading.push_back(0);
		if (i < Npersonality)
		{
			personality.push_back(0);
		}
	}
}
catState::catState(vector< vector<double>> initReaction, vector< vector<double> > currReaction, vector<int>  sensor, vector<double>  pers)
{

	for (size_t i = 0; i < Nstates; i++)
	{
		vector<double> tempVector;
		for (size_t j = 0; j < (Nstates); j++)
		{
			tempVector.push_back(0);
		}
		catState::currentReaction.push_back(tempVector);
		catState::initialReaction.push_back(tempVector);
	}

	for (size_t i = 0; i < Nsensor; i++)
	{
		sensorReading.push_back(sensor[i]);
		if (i < Npersonality)
		{
			personality.push_back(pers[i]);
		}
	}

	for (size_t i = 0; i < Nstates; i++)
	{
		for (size_t j = 0; j < (Nstates); j++)
		{
			catState::initialReaction[i][j] = initReaction[i][j];
			catState::currentReaction[i][j] = currReaction[i][j];
		}
	}

}
void catState::resetReaction()
{
	size_t i, j;
	for (i = 0; i < Nstates; i++)
	{
		for (j = 0; j < (Nstates); j++)
		{
			catState::currentReaction[i][j] = catState::initialReaction[i][j];
		}
	}

}
vector<int> catState::readSensors()
{
	sendMessage({ 4,0 });
	catState::sensorReading==readMessage();
	return catState::sensorReading;
}
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
void catState::setPersonality(vector<double> pers)
{
	for (int i = 0; i < Npersonality; i++)
	{
		catState::personality[i] = pers[i];
	}
}
void catState::setSensors(vector<int> sens)
{
	for (size_t i = 0; i < sensorReading.size(); ++i)
	{
		catState::sensorReading[i] = sens[i];
	}
}
catState::~catState()
{
}
