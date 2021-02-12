#pragma once
#define Nstates 8
#define Npersonality 4
#define Nsensor 11
#include <vector>
#include <iostream>
#include "generalState.h"
using namespace std;
class catState {
private:
	vector<vector<double>> initialReaction; // tabla 3 inicial para restaurar en estado espera
	vector<vector<double>> currentReaction;// tabla 3 modificada al permanecer en un estado
	vector<double>  personality;  // tabla 2
	vector<int>  sensorReading;
public:
	// constructores
	catState(void);
	catState(vector< vector<double> > initReaction, vector< vector<double> > currReaction, vector<int>  sensor, vector<double>  pers);
	// acciones
	void resetReaction(); // funcion restaurar tabla 3
	vector<int> readSensors(); // leer mensajes de los sensores
	// cambio valor variables
	void setInitialReaction(vector< vector<double> > initReact);
	void setCurrentReaction(vector< vector<double> > currReact);
	vector< vector<double> > restoreCurrentReaction();
	void setPersonality(vector<double>  pers);
	void setSensors(vector <int> sens);
	// lectura variables
	vector< vector<double> > getInitialReaction(void) { return initialReaction; }
	vector< vector<double> > getCurrentReaction(void) { return currentReaction; }
	vector<double>  getPersonality(void) { return personality; }
	vector<int>  getSensors(void) { return sensorReading; }
	// destructor
	~catState();
	friend class generalState;
};
