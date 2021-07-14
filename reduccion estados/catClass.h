//Para asegurar que el codigo fuente que lo invoca sea incluido una sola vez
#pragma once						

//Definicion de constantes y nombres simbolicos
#define Nstates 6
#define Npersonality 4
#define Nsensor 11

//Incluir el contenido de los archivos especificados
#include <vector>
#include <iostream>
#include "generalState.h"

//Region declarativa para organizar codigo en grupos logicos y evitar colisiones de nombres
using namespace std;

//Clase catSatate
class catState {

//Especificador de acceso "privado": los miembros (atributos y metodos) de la clase 
//							         no son accesibles desde fuera de la clase.

private:
	// Atributo vector 2-D initialReaction: Tabla 3 inicial para restaurar en estado espera
	vector<vector<double>> initialReaction;
	// Atributo vector 2-D currentReaction: Tabla 3 modificada al permanecer en un estado
	vector<vector<double>> currentReaction;
	// Atributo vector 1-D personality: Tabla 2
	vector<double>  personality;  
	// Atributo vector 1-D sensorReading
	vector<int>  sensorReading;


//Especificador de acceso "publico": los miembros (atributos y metodos) de la clase 
//							         son accesibles desde fuera de la clase.

public:
	// Constructores: metodos para inicializar las variables y ejecutar algunos metodos de la clase. Tienen mismo nombre de la clase y no tienen valor de retorno
	catState(void);																													   //Constructor por defecto (no tiene parametros de entrada)--inicializa atributos privados de la clase
	catState(vector< vector<double> > initReaction, vector< vector<double> > currReaction, vector<int>  sensor, vector<double>  pers); //Constructor parametrizado (con parametros de entrada)
	
	// Acciones
	void resetReaction();		// Funcion restaurar tabla 3
	vector<int> readSensors();  // Leer mensajes de los sensores, retorna "sensoReading"
	
	// Cambio valor variables
	void setInitialReaction(vector< vector<double> > initReact); //initialReaction
	void setCurrentReaction(vector< vector<double> > currReact); //currentReaction
	void setPersonality(vector<double>  pers);
	void setSensors(vector <int> sens);

	vector< vector<double> > restoreCurrentReaction();	// retorna "currentReaction"
	
	// Lectura variables
	vector< vector<double> > getInitialReaction(void) { return initialReaction; }
	vector< vector<double> > getCurrentReaction(void) { return currentReaction; }
	vector<double>  getPersonality(void) { return personality; }
	vector<int>  getSensors(void) { return sensorReading; }
	
	// destructor
	~catState();

	//clase  generalState puede acceder a miembros privados de catClass
	friend class generalState;

};
