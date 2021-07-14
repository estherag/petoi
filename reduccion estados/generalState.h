#pragma once
#include <vector>
#include <iostream>
#include "catClass.h"
using namespace std;
class catState;
class generalState {
private:
	vector<int> currentState; // estado actual principal, subestado
	vector<int> nextState; // estado siguiente principal, subestado
	vector<int> sensorThreshold; // umbral sensores tabla 1
	vector<vector<double>> thresholdReaction; //umbral reacciones
public:
	// constructores
	generalState(void);	
	generalState(vector<int> current, vector<int> next, vector<int> sensorThres, vector<vector<double>> thresReac);
	// acciones
	vector<int> checkGuardCondition(vector<vector<double>> currentReaction, vector<int> nextStat);// condicion guarda segun tabla 3 modificada
	vector<vector<double>> incrementReaction(vector<vector<double>> currentReaction, vector<int> guarCondResult);
	void checkTransitionState(vector<int> sensorReading, vector<vector<double>> currentReaction, vector<vector<double>> initialReaction);
	void checkTransitionSubState(vector<int> sensorT); // comprobar cambio de estado
	void changePosture(int post); // cambio de postura asociado a transicion
	void changeState();  // solicitar movimiento arduino: enviar mensaje
	vector<vector<double>> entryActivity(vector<int> state, vector<vector<double>> currentReaction, vector<vector<double>> initialReaction); //  modificar tabla 3, aumentar rasgo de personalidad
	// cambio valor variables
	void setCurrentState(vector<int> state);
	void setNextState(vector<int> nextStat);
	void setSensorThreshold(vector<int> sensorThres);
	void setThresholdReaction(vector<vector<double>> thresReac);
	// lectura variables
	vector<int>  getCurrentState(void) { return currentState; }
	vector<int>  getSensorThreshold(void) { return sensorThreshold; }
	vector<int>  getNextState(void) { return nextState; }
	vector<vector<double>>  getThresholdReaction(void) { return thresholdReaction; }
	// destructor
	~generalState();

	friend class catClass;
};

