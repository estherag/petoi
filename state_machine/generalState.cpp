#pragma once
#include "generalState.h"
#include "functions.h"
#include "catClass.h"
#include "communications.h"
#include <vector>
#include <iostream>
#include <algorithm>
#include <winsock2.h>
#include <stdlib.h>     /* srand, rand */
#include <time.h>       /* time */
using namespace std;
extern SOCKET ClientSocket;

enum estados {START, EXPLORE, HUNT, WAIT, GROOM, ATTENTION, SLEEP, PLAY };
enum posture {NEUTRAL, HEADLEFT, HEADRIGHT, HAPPY, FEAR, AGGRESIVE };
generalState::generalState(void)
{
	for (size_t i = 0; i < Nstates; i++)
	{
		vector<double> tempVector;
		for (size_t j = 0; j < (Nstates); j++)
		{

			tempVector.push_back(0);
		}
		generalState::thresholdReaction.push_back(tempVector);
	}

	for (size_t i = 0; i < Nsensor; i++)
	{
		generalState::sensorThreshold.push_back(0);
		if (i < 2)
		{
			generalState::nextState.push_back(0);
			generalState::currentState.push_back(0);
		}
	}

}
generalState::generalState(vector<int> current, vector<int> next, vector<int> sensorThres, vector<vector<double>> thresReac)
{
	for (size_t i = 0; i < Nstates; i++)
	{
		vector<double> tempVector;
		for (size_t j = 0; j < (Nstates); j++)
		{
			tempVector.push_back(0);
		}
		generalState::thresholdReaction.push_back(tempVector);
	}

	for ( size_t i = 0; i < Nstates; i++)
	{
		for (size_t j = 0; j < (Nstates); j++)
		{
			generalState::thresholdReaction[i][j] = thresReac[i][j];
		}
	}
	for (size_t i = 0; i < Nsensor; i++)
	{
		generalState::sensorThreshold.push_back(sensorThres[i]);
		if (i < 2)
		{
			generalState::nextState.push_back(next[i]);
			generalState::currentState.push_back(current[i]);
		}
	}
}
vector<int> generalState::checkGuardCondition(vector<vector<double>> currentReaction, vector<int> nextStat)
{
	
	double randomReact = ((double)rand() / (RAND_MAX));
	//cout << "Random: " << randomReact << endl;
	int previousState = generalState::currentState[0];
	int nextState = nextStat[0]; // estado principal\\

	if ((currentReaction[previousState][nextState] > generalState::thresholdReaction[previousState][nextState]) || (randomReact > generalState::thresholdReaction[previousState][nextState]))
	{
		generalState::changeState();
		return { 1,0,0 }; //  1 indica que se cumple la condicion
	}
	else
	{
		return { 0, previousState, nextState }; // 0 -> no se cumple la condicion
	}	

}
vector<vector<double>> generalState::incrementReaction(vector<vector<double>> currentReaction, vector<int> guarCondResult)
{
	int previousState = guarCondResult[1];
	int nextState = guarCondResult[2];
	currentReaction[previousState][nextState] += 0.05;
	currentReaction = adjustProbability(currentReaction);
	return currentReaction;
}
void generalState::checkTransitionState(vector<int> sensorReading, vector<vector<double>> currentReaction, vector<vector<double>> initialReaction)
{
	vector<int> guarCondResult={ 0,0,0 };
	int nextState=0;
	size_t postureSensor = 0;
	vector<int> diffSensor;
	for (size_t i = 0; i < sensorReading.size(); ++i)
	{
		diffSensor.push_back(abs(sensorReading[i] - generalState::sensorThreshold[i]));
	}
	
	size_t maxElementIndex = max_element(diffSensor.begin(), diffSensor.end()) - diffSensor.begin(); //indice de la mayor diferencia entre sensores

	switch (generalState::currentState[0]) // verificar estado principal
	{
		
	case START:
		if (diffSensor[maxElementIndex] <= 0) //no sensor
		{
			generalState::setNextState({ EXPLORE,0 }); //estado -> explorar
		}
		else
		{
			switch (maxElementIndex)
			{
			case 0: //US
				generalState::setNextState({ ATTENTION,0 });
				break;
			case 1: //mic1
				generalState::setNextState({ ATTENTION,0 });
				changePosture(HEADLEFT);
				break;
			case 2: //mic2
				generalState::setNextState({ ATTENTION,0 });
				changePosture(HEADRIGHT);
				break;
			case 3: // cabeza 1
			case 4: // cabeza 2
			case 5:	// cabeza 3

			case 6: // lomo 1
			case 7:	// lomo 2
			case 8:	// lomo 3
			case 9:	// lomo 4
			case 10: // lomo 5

				generalState::setNextState({ WAIT,0 });
				break;
			default:
				break;
			}
		}
		break;
	case EXPLORE:
		if (diffSensor[maxElementIndex] <= 0) //no sensor
		{
			generalState::setNextState({ WAIT,0 });
			changePosture(HAPPY);
		}
		else
		{
			switch (maxElementIndex)
			{
			case 0: //US
				generalState::setNextState({ ATTENTION,0 });
				while (guarCondResult[0] == 0)
				{
					guarCondResult = checkGuardCondition(currentReaction, generalState::nextState);
					if (guarCondResult[0] == 0) //no cumple condicion
					{
						currentReaction = incrementReaction(currentReaction, guarCondResult);
					}
				} //se cumple la condicion de guarda -> ejecutar cambio de estado
				diffSensor[0] = *min_element(diffSensor.begin(), diffSensor.end()); // sensor de la transicion al minimo para no influir en la postura
				postureSensor = max_element(diffSensor.begin(), diffSensor.end()) - diffSensor.begin(); //indice de la mayor diferencia entre sensores
				if (postureSensor == 3 || postureSensor == 4 || postureSensor == 5)//cabeza
				{
					changePosture(HAPPY);
				}
				else
				{
					changePosture(NEUTRAL);
				}
				break;
			case 1: //mic1
			case 2: //mic2
				generalState::setNextState({ ATTENTION,0 });
				while (guarCondResult[0] == 0)
				{
					guarCondResult = checkGuardCondition(currentReaction, generalState::nextState);
					if (guarCondResult[0] == 0) //no cumple condicion
					{
						currentReaction = incrementReaction(currentReaction, guarCondResult);
					}
				}
				diffSensor[1] = *min_element(diffSensor.begin(), diffSensor.end()); // sensor de la transicion al minimo para no influir en la postura
				diffSensor[2] = *min_element(diffSensor.begin(), diffSensor.end()); // sensor de la transicion al minimo para no influir en la postura
				postureSensor = max_element(diffSensor.begin(), diffSensor.end()) - diffSensor.begin(); //indice de la mayor diferencia entre sensores
				switch (postureSensor)
				{
				case 0://US
					changePosture(HAPPY);
					break;
				case 3://cabeza
				case 4:
				case 5:
					changePosture(FEAR);
					break;
				case 6://lomo
				case 7:
				case 8:
				case 9:
				case 10:
					changePosture(AGGRESIVE);
					break;
				default:
					changePosture(NEUTRAL);
					break;
				}
				break;
			case 3: // cabeza 1
			case 4: // cabeza 2
			case 5:	// cabeza 3
				generalState::setNextState({ PLAY,0 });
				while (guarCondResult[0] == 0)
				{
					guarCondResult = checkGuardCondition(currentReaction, generalState::nextState);
					if (guarCondResult[0] == 0) //no cumple condicion
					{
						currentReaction = incrementReaction(currentReaction, guarCondResult);
					}
				}
				diffSensor[3] = *min_element(diffSensor.begin(), diffSensor.end()); // sensor de la transicion al minimo para no influir en la postura
				diffSensor[4] = *min_element(diffSensor.begin(), diffSensor.end()); // sensor de la transicion al minimo para no influir en la postura
				diffSensor[5] = *min_element(diffSensor.begin(), diffSensor.end()); // sensor de la transicion al minimo para no influir en la postura
				postureSensor = max_element(diffSensor.begin(), diffSensor.end()) - diffSensor.begin(); //indice de la mayor diferencia entre sensores
				switch (postureSensor)
				{
				case 0://US
				case 1: //mic1
				case 2:
					changePosture(HAPPY);
					break;
				default:
					changePosture(NEUTRAL);
					break;
				}
				break;
			case 6: // lomo 1
			case 7:	// lomo 2
			case 8:	// lomo 3
			case 9:	// lomo 4
			case 10: // lomo 5
				generalState::setNextState({ HUNT,0 });
				while (guarCondResult[0] == 0)
				{
					guarCondResult = checkGuardCondition(currentReaction, generalState::nextState);
					if (guarCondResult[0] == 0) //no cumple condicion
					{
						currentReaction = incrementReaction(currentReaction, guarCondResult);
					}
				}
				diffSensor[6] = *min_element(diffSensor.begin(), diffSensor.end()); // sensor de la transicion al minimo para no influir en la postura
				diffSensor[7] = *min_element(diffSensor.begin(), diffSensor.end()); // sensor de la transicion al minimo para no influir en la postura
				diffSensor[8] = *min_element(diffSensor.begin(), diffSensor.end()); // sensor de la transicion al minimo para no influir en la postura
				diffSensor[9] = *min_element(diffSensor.begin(), diffSensor.end()); // sensor de la transicion al minimo para no influir en la postura
				diffSensor[10] = *min_element(diffSensor.begin(), diffSensor.end()); // sensor de la transicion al minimo para no influir en la postura
				postureSensor = max_element(diffSensor.begin(), diffSensor.end()) - diffSensor.begin(); //indice de la mayor diferencia entre sensores
				switch (postureSensor)
				{
				case 0://US
					changePosture(FEAR);
					break;
				case 1: //mic1
				case 2:
					changePosture(AGGRESIVE);
					break;
				default:
					changePosture(NEUTRAL);
					break;
				}
				break;
			default:
				break;
			}
		}
		break;
	case HUNT: 
		if (diffSensor[maxElementIndex] <= 0) //esperar sensor>umbral
		{
			generalState::setNextState({ WAIT,0 });
			changePosture(AGGRESIVE);
		}
		else
		{
			switch (maxElementIndex)
			{
			case 0: //US
				generalState::setNextState({ PLAY,0 });
				while (guarCondResult[0] == 0)
				{
					guarCondResult = checkGuardCondition(currentReaction, generalState::nextState);
					if (guarCondResult[0] == 0) //no cumple condicion
					{
						currentReaction = incrementReaction(currentReaction, guarCondResult);
					}
				} //se cumple la condicion de guarda -> ejecutar cambio de estado
				diffSensor[0] = *min_element(diffSensor.begin(), diffSensor.end()); // sensor de la transicion al minimo para no influir en la postura
				postureSensor = max_element(diffSensor.begin(), diffSensor.end()) - diffSensor.begin(); //indice de la mayor diferencia entre sensores
				if (postureSensor == 3 || postureSensor == 4 || postureSensor == 5)//cabeza
				{
					changePosture(HAPPY); 
				}
				else if (postureSensor == 1 || postureSensor == 2)
				{
					changePosture(AGGRESIVE);
				}
				else
				{
					changePosture(NEUTRAL);
				}
				break;
			case 1: //mic1
			case 2: //mic2

			case 3: // cabeza 1
			case 4: // cabeza 2
			case 5:	// cabeza 3
				generalState::setNextState({ ATTENTION,0 });
				while (guarCondResult[0] == 0)
				{
					guarCondResult = checkGuardCondition(currentReaction, generalState::nextState);
					if (guarCondResult[0] == 0) //no cumple condicion
					{
						currentReaction = incrementReaction(currentReaction, guarCondResult);
					}
				}
				changePosture(AGGRESIVE);
				break;
			case 6: // lomo 1
			case 7:	// lomo 2
			case 8:	// lomo 3
			case 9:	// lomo 4
			case 10: // lomo 5
				generalState::setNextState({ WAIT,0 });
				while (guarCondResult[0] == 0)
				{
					guarCondResult = checkGuardCondition(currentReaction, generalState::nextState);
					if (guarCondResult[0] == 0) //no cumple condicion
					{
						currentReaction = incrementReaction(currentReaction, guarCondResult);
					}
				}
				changePosture(AGGRESIVE);
				break;
			default:
				break;
			}
		}
		break;
	case WAIT:
		if (diffSensor[maxElementIndex] <= 0) //no sensor
		{
			generalState::setNextState({ WAIT,0 }); //estado -> explorar
			changePosture(NEUTRAL);
		}
		else
		{
			switch (maxElementIndex)
			{
			case 0: //US
				generalState::setNextState({ ATTENTION,0 });
				changePosture(NEUTRAL);
				break;
			case 1: //mic1
			case 2: //mic2
				generalState::setNextState({ GROOM,0 });
				changePosture(NEUTRAL);
				break;
			case 3: // cabeza 1
			case 4: // cabeza 2
			case 5:	// cabeza 3
				generalState::setNextState({ SLEEP,0 });
				while (guarCondResult[0] == 0)
				{
					guarCondResult = checkGuardCondition(currentReaction, generalState::nextState);
					if (guarCondResult[0] == 0) //no cumple condicion
					{
						currentReaction = incrementReaction(currentReaction, guarCondResult);
					}
				}
				diffSensor[3] = *min_element(diffSensor.begin(), diffSensor.end()); // sensor de la transicion al minimo para no influir en la postura
				diffSensor[4] = *min_element(diffSensor.begin(), diffSensor.end()); // sensor de la transicion al minimo para no influir en la postura
				diffSensor[5] = *min_element(diffSensor.begin(), diffSensor.end()); // sensor de la transicion al minimo para no influir en la postura
				postureSensor = max_element(diffSensor.begin(), diffSensor.end()) - diffSensor.begin(); //indice de la mayor diferencia entre sensores
				switch (postureSensor)
				{
				case 0://US
					changePosture(FEAR);
					break;
				case 1: //mic1
				case 2:
					changePosture(AGGRESIVE);
					break;
				case 6: // lomo 1
				case 7:	// lomo 2
				case 8:	// lomo 3
				case 9:	// lomo 4
				case 10: // lomo 5
					changePosture(HAPPY);
					break;
				default:
					changePosture(NEUTRAL);
					break;
				}
				break;
			case 6: // lomo 1
			case 7:	// lomo 2
			case 8:	// lomo 3
			case 9:	// lomo 4
			case 10: // lomo 5
				generalState::setNextState({ EXPLORE,0 });
				while (guarCondResult[0] == 0)
				{
					guarCondResult = checkGuardCondition(currentReaction, generalState::nextState);
					if (guarCondResult[0] == 0) //no cumple condicion
					{
						currentReaction = incrementReaction(currentReaction, guarCondResult);
					}
				}
				diffSensor[6] = *min_element(diffSensor.begin(), diffSensor.end()); // sensor de la transicion al minimo para no influir en la postura
				diffSensor[7] = *min_element(diffSensor.begin(), diffSensor.end()); // sensor de la transicion al minimo para no influir en la postura
				diffSensor[8] = *min_element(diffSensor.begin(), diffSensor.end()); // sensor de la transicion al minimo para no influir en la postura
				diffSensor[9] = *min_element(diffSensor.begin(), diffSensor.end()); // sensor de la transicion al minimo para no influir en la postura
				diffSensor[10] = *min_element(diffSensor.begin(), diffSensor.end()); // sensor de la transicion al minimo para no influir en la postura
				postureSensor = max_element(diffSensor.begin(), diffSensor.end()) - diffSensor.begin(); //indice de la mayor diferencia entre sensores
				switch (postureSensor)
				{
				case 0://US
					changePosture(HAPPY);
					break;
				case 3:
				case 4:
				case 5:
					changePosture(FEAR);
					break;
				default:
					changePosture(NEUTRAL);
					break;
				}
				break;
			default:
				break;
			}
		}
		break;
	case GROOM:
		if (diffSensor[maxElementIndex] <= 0) //no sensor
		{
			generalState::setNextState({ SLEEP,0 });
		}
		else
		{
			switch (maxElementIndex)
			{
			case 0: //US
				generalState::setNextState({ ATTENTION,0 });
				while (guarCondResult[0] == 0)
				{
					guarCondResult = checkGuardCondition(currentReaction, generalState::nextState);
					if (guarCondResult[0] == 0) //no cumple condicion
					{
						currentReaction = incrementReaction(currentReaction, guarCondResult);
					}
				}
				diffSensor[0] = *min_element(diffSensor.begin(), diffSensor.end()); // sensor de la transicion al minimo para no influir en la postura
				postureSensor = max_element(diffSensor.begin(), diffSensor.end()) - diffSensor.begin(); //indice de la mayor diferencia entre sensores
				switch (postureSensor)
				{
				
				case 1: //mic1
				case 2:
					changePosture(AGGRESIVE);
					break;
				case 3:
				case 4:
				case 5:
					changePosture(HAPPY);
					break;
				default:
					changePosture(NEUTRAL);
					break;
				}
				break;
			case 1: //mic1
			case 2: //mic2
				generalState::setNextState({ EXPLORE,0 });
				while (guarCondResult[0] == 0)
				{
					guarCondResult = checkGuardCondition(currentReaction, generalState::nextState);
					if (guarCondResult[0] == 0) //no cumple condicion
					{
						currentReaction = incrementReaction(currentReaction, guarCondResult);
					}
				}
				diffSensor[1] = *min_element(diffSensor.begin(), diffSensor.end()); // sensor de la transicion al minimo para no influir en la postura
				diffSensor[2] = *min_element(diffSensor.begin(), diffSensor.end()); // sensor de la transicion al minimo para no influir en la postura
				postureSensor = max_element(diffSensor.begin(), diffSensor.end()) - diffSensor.begin(); //indice de la mayor diferencia entre sensores
				switch (postureSensor)
				{

				case 0: //US
					changePosture(HAPPY);
					break;
				case 3:
				case 4:
				case 5:
					changePosture(FEAR);
					break;
				case 6:
				case 7:
				case 8:
				case 9:
				case 10:
					changePosture(AGGRESIVE);
					break;
				default:
					changePosture(NEUTRAL);
					break;
				}
				break;
			case 3: // cabeza 1
			case 4: // cabeza 2
			case 5:	// cabeza 3

			case 6: // lomo 1
			case 7:	// lomo 2
			case 8:	// lomo 3
			case 9:	// lomo 4
			case 10: // lomo 5
				generalState::setNextState({ HUNT,0 });
				while (guarCondResult[0] == 0)
				{
					guarCondResult = checkGuardCondition(currentReaction, generalState::nextState);
					if (guarCondResult[0] == 0) //no cumple condicion
					{
						currentReaction = incrementReaction(currentReaction, guarCondResult);
					}
				}
				diffSensor[3] = *min_element(diffSensor.begin(), diffSensor.end()); // sensor de la transicion al minimo para no influir en la postura
				diffSensor[4] = *min_element(diffSensor.begin(), diffSensor.end()); // sensor de la transicion al minimo para no influir en la postura
				diffSensor[5] = *min_element(diffSensor.begin(), diffSensor.end()); // sensor de la transicion al minimo para no influir en la postura
				diffSensor[6] = *min_element(diffSensor.begin(), diffSensor.end()); // sensor de la transicion al minimo para no influir en la postura
				diffSensor[7] = *min_element(diffSensor.begin(), diffSensor.end()); // sensor de la transicion al minimo para no influir en la postura
				diffSensor[8] = *min_element(diffSensor.begin(), diffSensor.end()); // sensor de la transicion al minimo para no influir en la postura
				diffSensor[9] = *min_element(diffSensor.begin(), diffSensor.end()); // sensor de la transicion al minimo para no influir en la postura
				diffSensor[10] = *min_element(diffSensor.begin(), diffSensor.end()); // sensor de la transicion al minimo para no influir en la postura
				postureSensor = max_element(diffSensor.begin(), diffSensor.end()) - diffSensor.begin(); //indice de la mayor diferencia entre sensores
				switch (postureSensor)
				{
				case 0://US
					changePosture(FEAR);
					break;
				case 1:
				case 2:
					changePosture(AGGRESIVE);
					break;
				default:
					changePosture(NEUTRAL);
					break;
				}
				break;

			default:
				break;
			}
		}
		break;
	case ATTENTION:
		if (diffSensor[maxElementIndex] <= 0) //no sensor
		{
			generalState::setNextState({ WAIT,0 });
			changePosture(NEUTRAL);
		}
		else
		{
			switch (maxElementIndex)
			{
			case 0: //US
			case 1: //mic1
			case 2: //mic2
				generalState::setNextState({ PLAY,0 });
				while (guarCondResult[0] == 0)
				{
					guarCondResult = checkGuardCondition(currentReaction, generalState::nextState);
					if (guarCondResult[0] == 0) //no cumple condicion
					{
						currentReaction = incrementReaction(currentReaction, guarCondResult);
					}
				}
				diffSensor[0] = *min_element(diffSensor.begin(), diffSensor.end()); // sensor de la transicion al minimo para no influir en la postura
				diffSensor[1] = *min_element(diffSensor.begin(), diffSensor.end()); // sensor de la transicion al minimo para no influir en la postura
				diffSensor[2] = *min_element(diffSensor.begin(), diffSensor.end()); // sensor de la transicion al minimo para no influir en la postura
				postureSensor = max_element(diffSensor.begin(), diffSensor.end()) - diffSensor.begin(); //indice de la mayor diferencia entre sensores
				switch (postureSensor)
				{
				case 3:
				case 4:
				case 5:

				case 6:
				case 7:
				case 8:
				case 9:
				case 10:
					changePosture(HAPPY);
					break;
				default:
					changePosture(NEUTRAL);
					break;
				}
				break;
			case 3: // cabeza 1
			case 4: // cabeza 2
			case 5:	// cabeza 3
				generalState::setNextState({ GROOM,0 });
				while (guarCondResult[0] == 0)
				{
					guarCondResult = checkGuardCondition(currentReaction, generalState::nextState);
					if (guarCondResult[0] == 0) //no cumple condicion
					{
						currentReaction = incrementReaction(currentReaction, guarCondResult);
					}
				}
				diffSensor[3] = *min_element(diffSensor.begin(), diffSensor.end()); // sensor de la transicion al minimo para no influir en la postura
				diffSensor[4] = *min_element(diffSensor.begin(), diffSensor.end()); // sensor de la transicion al minimo para no influir en la postura
				diffSensor[5] = *min_element(diffSensor.begin(), diffSensor.end()); // sensor de la transicion al minimo para no influir en la postura
				postureSensor = max_element(diffSensor.begin(), diffSensor.end()) - diffSensor.begin(); //indice de la mayor diferencia entre sensores
				switch (postureSensor)
				{
				case 0:
				case 1:
				case 2:
					changePosture(HAPPY);
					break;
				default:
					changePosture(NEUTRAL);
					break;
				}
				break;
			case 6: // lomo 1
			case 7:	// lomo 2
			case 8:	// lomo 3
			case 9:	// lomo 4
			case 10: // lomo 5
				generalState::setNextState({ HUNT,0 });
				while (guarCondResult[0] == 0)
				{
					guarCondResult = checkGuardCondition(currentReaction, generalState::nextState);
					if (guarCondResult[0] == 0) //no cumple condicion
					{
						currentReaction = incrementReaction(currentReaction, guarCondResult);
					}
				}
				diffSensor[6] = *min_element(diffSensor.begin(), diffSensor.end()); // sensor de la transicion al minimo para no influir en la postura
				diffSensor[7] = *min_element(diffSensor.begin(), diffSensor.end()); // sensor de la transicion al minimo para no influir en la postura
				diffSensor[8] = *min_element(diffSensor.begin(), diffSensor.end()); // sensor de la transicion al minimo para no influir en la postura
				diffSensor[9] = *min_element(diffSensor.begin(), diffSensor.end()); // sensor de la transicion al minimo para no influir en la postura
				diffSensor[10] = *min_element(diffSensor.begin(), diffSensor.end()); // sensor de la transicion al minimo para no influir en la postura
				postureSensor = max_element(diffSensor.begin(), diffSensor.end()) - diffSensor.begin(); //indice de la mayor diferencia entre sensores
				switch (postureSensor)
				{
				case 0://US
					changePosture(FEAR);
					break;
				case 1:
				case 2:
					changePosture(AGGRESIVE);
					break;
				default:
					changePosture(NEUTRAL);
					break;
				}
				break;

			default:
				break;
			}
		}
		break;
	case SLEEP:
		if (diffSensor[maxElementIndex] <= 0) //no sensor
		{
			generalState::setNextState({ WAIT,0 });
			changePosture(NEUTRAL);
		}
		else
		{
			switch (maxElementIndex)
			{
			case 0: //US
			case 1: //mic1
			case 2: //mic2
				generalState::setNextState({ GROOM,0 });
				while (guarCondResult[0] == 0)
				{
					guarCondResult = checkGuardCondition(currentReaction, generalState::nextState);
					if (guarCondResult[0] == 0) //no cumple condicion
					{
						currentReaction = incrementReaction(currentReaction, guarCondResult);
					}
				}
				break;
			case 3: // cabeza 1
			case 4: // cabeza 2
			case 5:	// cabeza 3
				generalState::setNextState({ PLAY,0 });
				while (guarCondResult[0] == 0)
				{
					guarCondResult = checkGuardCondition(currentReaction, generalState::nextState);
					if (guarCondResult[0] == 0) //no cumple condicion
					{
						currentReaction = incrementReaction(currentReaction, guarCondResult);
					}
				}
				diffSensor[3] = *min_element(diffSensor.begin(), diffSensor.end()); // sensor de la transicion al minimo para no influir en la postura
				diffSensor[4] = *min_element(diffSensor.begin(), diffSensor.end()); // sensor de la transicion al minimo para no influir en la postura
				diffSensor[5] = *min_element(diffSensor.begin(), diffSensor.end()); // sensor de la transicion al minimo para no influir en la postura
				postureSensor = max_element(diffSensor.begin(), diffSensor.end()) - diffSensor.begin(); //indice de la mayor diferencia entre sensores
				switch (postureSensor)
				{
				case 0:
					changePosture(HAPPY);
					break;
				case 1:
				case 2:
					changePosture(AGGRESIVE);
					break;
				default:
					changePosture(NEUTRAL);
					break;
				}
				break;
			case 6: // lomo 1
			case 7:	// lomo 2
			case 8:	// lomo 3
			case 9:	// lomo 4
			case 10: // lomo 5
				generalState::setNextState({ HUNT,0 });
				while (guarCondResult[0] == 0)
				{
					guarCondResult = checkGuardCondition(currentReaction, generalState::nextState);
					if (guarCondResult[0] == 0) //no cumple condicion
					{
						currentReaction = incrementReaction(currentReaction, guarCondResult);
					}
				}
				diffSensor[6] = *min_element(diffSensor.begin(), diffSensor.end()); // sensor de la transicion al minimo para no influir en la postura
				diffSensor[7] = *min_element(diffSensor.begin(), diffSensor.end()); // sensor de la transicion al minimo para no influir en la postura
				diffSensor[8] = *min_element(diffSensor.begin(), diffSensor.end()); // sensor de la transicion al minimo para no influir en la postura
				diffSensor[9] = *min_element(diffSensor.begin(), diffSensor.end()); // sensor de la transicion al minimo para no influir en la postura
				diffSensor[10] = *min_element(diffSensor.begin(), diffSensor.end()); // sensor de la transicion al minimo para no influir en la postura
				postureSensor = max_element(diffSensor.begin(), diffSensor.end()) - diffSensor.begin(); //indice de la mayor diferencia entre sensores
				switch (postureSensor)
				{
				case 0://US
					changePosture(FEAR);
					break;
				case 1:
				case 2:
					changePosture(AGGRESIVE);
					break;
				default:
					changePosture(NEUTRAL);
					break;
				}
				break;
			default:
				break;
			}
		}
		break;
	case PLAY:
		if (diffSensor[maxElementIndex] <= 0) //no sensor
		{
			generalState::setNextState({ WAIT,0 });
			changePosture(HAPPY);
		}
		else
		{
			switch (maxElementIndex)
			{
			case 0: //US
				generalState::setNextState({ HUNT,0 });
				while (guarCondResult[0] == 0)
				{
					guarCondResult = checkGuardCondition(currentReaction, generalState::nextState);
					if (guarCondResult[0] == 0) //no cumple condicion
					{
						currentReaction = incrementReaction(currentReaction, guarCondResult);
					}
				}
				diffSensor[0] = *min_element(diffSensor.begin(), diffSensor.end()); // sensor de la transicion al minimo para no influir en la postura
				postureSensor = max_element(diffSensor.begin(), diffSensor.end()) - diffSensor.begin(); //indice de la mayor diferencia entre sensores
				switch (postureSensor)
				{
				case 1:
				case 2:
					changePosture(AGGRESIVE);
					break;
				case 6:
				case 7:
				case 8:
				case 9:
				case 10:
					changePosture(FEAR);
					break;
				default:
					changePosture(NEUTRAL);
					break;
				}
				break;
			case 1: //mic1
			case 2: //mic2
				generalState::setNextState({ ATTENTION,0 });
				while (guarCondResult[0] == 0)
				{
					guarCondResult = checkGuardCondition(currentReaction, generalState::nextState);
					if (guarCondResult[0] == 0) //no cumple condicion
					{
						currentReaction = incrementReaction(currentReaction, guarCondResult);
					}
				}
				diffSensor[1] = *min_element(diffSensor.begin(), diffSensor.end()); // sensor de la transicion al minimo para no influir en la postura
				diffSensor[2] = *min_element(diffSensor.begin(), diffSensor.end()); // sensor de la transicion al minimo para no influir en la postura
				postureSensor = max_element(diffSensor.begin(), diffSensor.end()) - diffSensor.begin(); //indice de la mayor diferencia entre sensores
				switch (postureSensor)
				{
				case 0:
					changePosture(FEAR);
					break;
				case 3:
				case 4:
				case 5:
				case 6:
				case 7:
				case 8:
				case 9:
				case 10:
					changePosture(AGGRESIVE);
					break;
				default:
					changePosture(NEUTRAL);
					break;
				}
				break;
			case 3: // cabeza 1
			case 4: // cabeza 2
			case 5:	// cabeza 3
				generalState::setNextState({ SLEEP,0 });
				while (guarCondResult[0] == 0)
				{
					guarCondResult = checkGuardCondition(currentReaction, generalState::nextState);
					if (guarCondResult[0] == 0) //no cumple condicion
					{
						currentReaction = incrementReaction(currentReaction, guarCondResult);
					}
				}
				diffSensor[3] = *min_element(diffSensor.begin(), diffSensor.end()); // sensor de la transicion al minimo para no influir en la postura
				diffSensor[4] = *min_element(diffSensor.begin(), diffSensor.end()); // sensor de la transicion al minimo para no influir en la postura
				diffSensor[5] = *min_element(diffSensor.begin(), diffSensor.end()); // sensor de la transicion al minimo para no influir en la postura
				postureSensor = max_element(diffSensor.begin(), diffSensor.end()) - diffSensor.begin(); //indice de la mayor diferencia entre sensores
				switch (postureSensor)
				{
				case 0:
					changePosture(FEAR);
					break;
				case 1:
				case 2:
					changePosture(AGGRESIVE);
					break;
				case 6: 
				case 7:	
				case 8:	
				case 9:	
				case 10: 
					changePosture(HAPPY);
					break;
				default:
					changePosture(NEUTRAL);
					break;
				}
				break;
			case 6: // lomo 1
			case 7:	// lomo 2
			case 8:	// lomo 3
			case 9:	// lomo 4
			case 10: // lomo 5
				generalState::setNextState({ GROOM,0 });
				while (guarCondResult[0] == 0)
				{
					guarCondResult = checkGuardCondition(currentReaction, generalState::nextState);
					if (guarCondResult[0] == 0) //no cumple condicion
					{
						currentReaction = incrementReaction(currentReaction, guarCondResult);
					}
				}
				diffSensor[6] = *min_element(diffSensor.begin(), diffSensor.end()); // sensor de la transicion al minimo para no influir en la postura
				diffSensor[7] = *min_element(diffSensor.begin(), diffSensor.end()); // sensor de la transicion al minimo para no influir en la postura
				diffSensor[8] = *min_element(diffSensor.begin(), diffSensor.end()); // sensor de la transicion al minimo para no influir en la postura
				diffSensor[9] = *min_element(diffSensor.begin(), diffSensor.end()); // sensor de la transicion al minimo para no influir en la postura
				diffSensor[10] = *min_element(diffSensor.begin(), diffSensor.end()); // sensor de la transicion al minimo para no influir en la postura
				postureSensor = max_element(diffSensor.begin(), diffSensor.end()) - diffSensor.begin(); //indice de la mayor diferencia entre sensores
				switch (postureSensor)
				{
				case 0://US
					changePosture(FEAR);
					break;
				case 1:
				case 2:
					changePosture(AGGRESIVE);
					break;
				case 3:
				case 4:
				case 5:
					changePosture(HAPPY);
					break;
				default:
					changePosture(NEUTRAL);
					break;
				}
				break;
			default:
				break;
			}
		}
		break;
	default:
		break;
	}
}
void generalState::checkTransitionSubState(vector<int> sensorReading )
{
	vector<int> diffSensor;
	for (size_t i = 0; i < sensorReading.size(); ++i)
	{
		diffSensor.push_back(sensorReading[i] - generalState::sensorThreshold[i]);
	}

	size_t maxElementIndex = max_element(diffSensor.begin(), diffSensor.end()) - diffSensor.begin(); //indice de la mayor diferencia entre sensores

	switch (generalState::currentState[0])
	{
	case EXPLORE:
		if ((maxElementIndex != 6 && maxElementIndex != 7 && maxElementIndex != 8 && maxElementIndex != 9 && maxElementIndex != 10) && (generalState::currentState[1]) < 9)
		{
			generalState::setNextState({ EXPLORE,currentState[1] + 1 });
		}
		else
		{
			generalState::setNextState({ EXPLORE,0 });//finSubestado

		}
		break;
	case HUNT:
		if (generalState::currentState[1] < 8)
		{
			generalState::setNextState({ HUNT,currentState[1] + 1 });
		}
		else
		{
			generalState::setNextState({ HUNT,0 });//finSubestado
		}
		break;
	case WAIT:
		if (diffSensor[maxElementIndex] > 0) //hay sensor
		{
			generalState::setNextState({ WAIT,currentState[1] });//mantener postura
		}
		break;
	case GROOM:
		if (generalState::currentState[1] < 9)
		{
			generalState::setNextState({ GROOM,currentState[1] + 1 });
		}
		else if (generalState::currentState[1] > 16)
		{
			generalState::setNextState({ GROOM,0 });//finSubestado
		}
		else
		{
			if (diffSensor[maxElementIndex] > 0) //hay sensor
			{
				generalState::setNextState({ GROOM,0 });//finSubestado
			}
			else
			{
				generalState::setNextState({ GROOM,currentState[1] + 1 });
			}

		}
		break;
	case ATTENTION:
		if (diffSensor[maxElementIndex] <= 0 && generalState::currentState[1] < 9) // no hay sensor
		{
			generalState::setNextState({ ATTENTION,currentState[1] + 1 });
		}
		else
		{
			generalState::setNextState({ ATTENTION,0 });//finSubestado
		}
		break;
	case SLEEP:
		if (diffSensor[maxElementIndex] <= 0 && generalState::currentState[1] < 4) //no hay sensor
		{
			generalState::setNextState({ SLEEP,currentState[1] + 1 });
		}
		else
		{
			generalState::setNextState({ SLEEP,0 });//finSubestad
		}
		break;
	case PLAY:
		if ((generalState::currentState[1] < 5) || (generalState::currentState[1] < 8 && diffSensor[maxElementIndex] <= 0)) //no sensor
		{
			generalState::setNextState({ PLAY,currentState[1] + 1 });
		}
		else
		{
			generalState::setNextState({ PLAY,0 });//finSubestad
		}
		
		break;
	default:
		break;
	}
}
void generalState::changePosture(int post)
{
	switch (post)
	{
	case NEUTRAL:
		sendMessage({1,1});
		cout << "Current posture set to NEUTRAL " << endl;
		break;
	case HEADLEFT:
		sendMessage({ 1,2 });
		cout << "Current posture set to HEADLEFT " << endl;
		break;
	case HEADRIGHT:
		sendMessage({ 1,3 });
		cout << "Current posture set to HEADRIGHT " << endl;
		break;
	case HAPPY:
		sendMessage({ 1,4 });
		cout << "Current posture set to HAPPY " << endl;
		break;
	case FEAR:
		sendMessage({ 1,5 });
		cout << "Current posture set to FEAR " << endl;
		break;
	case AGGRESIVE:
		sendMessage({ 1,6 });
		cout << "Current posture set to AGGRESIVE " << endl;
		break;
	default:
		break;
	}
}
void generalState::changeState()
{
	switch (generalState::nextState[0])
	{
	case START:
		generalState::setCurrentState(generalState::nextState);
		cout << "Current State set to START and substate " << generalState::nextState[1] << endl;
		break;
	case EXPLORE:
		switch (generalState::nextState[1])
		{
		case 0:
			sendMessage({ 2,0 });
			break;
		case 1:
			sendMessage({ 2,1 });
			break;
		case 2:
			sendMessage({ 2,2 });
			break;
		case 3:
			sendMessage({ 2,3 });
			break;
		case 4:
			sendMessage({ 2,4 });
			break;
		case 5:
			sendMessage({ 2,5 });
			break;
		case 6:
			sendMessage({ 2,6 });
			break;
		case 7:
			sendMessage({ 2,4 });
			break;
		case 8:
			sendMessage({ 2,7});
			break;
		case 9:
			sendMessage({ 2,18 });
			break;
		default:
			break;
		}
		generalState::setCurrentState(generalState::nextState);
		cout << "Current State set to EXPLORE and substate " << generalState::nextState[1] << endl;
		break;
	case HUNT:
		switch (generalState::nextState[1])
		{
		case 1:
			sendMessage({ 2,8 });
			break;
		case 2:
			sendMessage({ 2,9 });
			break;
		case 3:
			sendMessage({ 2,10 });
			break;
		case 4:
			sendMessage({ 2,11 });
			break;
		case 5:
			sendMessage({ 2,18 });
			break;
		case 6:
			sendMessage({ 2,12 });
			break;
		case 7:
			sendMessage({ 2,4 });
			break;
		case 8:
			sendMessage({ 2,7 });
			break;
		default:
			break;
		}
		generalState::setCurrentState(generalState::nextState);
		cout << "Current State set to HUNT and substate " << generalState::nextState[1] << endl;
		break;
	case WAIT:
		generalState::setCurrentState(generalState::nextState);
		cout << "Current State set to WAIT and substate " << generalState::nextState[1] << endl;
		break;
	case GROOM:
		switch (generalState::nextState[1])
		{
		case 1:
			sendMessage({ 2,13 });
			break;
		case 2:
			sendMessage({ 2,14 });
			break;
		case 3:
		case 5:
		case 7:
			sendMessage({ 2,3 });
			break;
		case 4:
		case 6:
		case 8:
			sendMessage({ 2,5 });
			break;
		case 9:
			sendMessage({ 2,13 });
			break;
		case 10:
			sendMessage({ 2,3 });
			break;
		case 11:
		case 13:
		case 15:
			sendMessage({ 2,5 });
			break;
		case 12:
		case 14:
		case 16:
			sendMessage({ 2,3 });
			break;
		default:
			break;
		}
		generalState::setCurrentState(generalState::nextState);
		cout << "Current State set to GROOM and substate " << generalState::nextState[1] << endl;
		break;
	case ATTENTION:
		switch (generalState::nextState[1])
		{
		case 1:
		case 4:
		case 7:
			sendMessage({ 2,15 });
			break;
		case 2:
		case 5:
		case 8:
			sendMessage({ 2,4 });
			break;
		case 3:
		case 6:
		case 9:
			sendMessage({ 2,7 });
			break;
		default:
			break;
		}
		generalState::setCurrentState(generalState::nextState);
		cout << "Current State set to ATTENTION and substate " << generalState::nextState[1] << endl;
		break;
	case SLEEP: 
		switch (generalState::nextState[1])
		{
		case 1:
			sendMessage({ 2,13 });
			break;
		case 2:
			sendMessage({ 2,16 });
			break;
		case 3:
			sendMessage({ 2,3 });
			break;
		case 4:
			sendMessage({ 2,17 });
			break;
		default:
			break;
		}
		generalState::setCurrentState(generalState::nextState);
		cout << "Current State set to SLEEP and substate " << generalState::nextState[1] << endl;
		break;
	case PLAY:
		switch (generalState::nextState[1])
		{
		case 1:
			sendMessage({ 2,9 });
			break;
		case 2:
			sendMessage({ 2,10 });
			break;
		case 3:
			sendMessage({ 2,11 });
			break;
		case 4:
			sendMessage({ 2,4 });
			break;
		case 5:
			sendMessage({ 2,12 });
			break;
		case 6:
		case 8:
			sendMessage({ 2,4 });
			break;
		case 7:
			sendMessage({ 2,7 });
			break;
		default:
			break;
		}
		generalState::setCurrentState(generalState::nextState);
		cout << "Current State set to PLAY and substate " << generalState::nextState[1] << endl;
		break;
	default:
		break;
	}
	
}
vector<vector<double>> generalState::entryActivity(vector<int> state, vector<vector<double>> currentReaction, vector<vector<double>> initialReaction)
{
	switch (state[0])
	{
	case WAIT:
		//restaurar actitud
		for (size_t i = 0; i < currentReaction.size(); ++i)
		{
			for (size_t j = 0; j < currentReaction[i].size(); ++j)
			{
				currentReaction[i][j] = initialReaction[i][j];
			}
		}
		break;
	case EXPLORE:
		//aumentar sociabilidad
		for (size_t j = 0; j < currentReaction[0].size(); ++j)
		{
			currentReaction[0][j] += 0.1;
			currentReaction[4][j] += 0.1;
		}
		break;
	case ATTENTION:
		//aumentar desconfianza
		for (size_t j = 0; j < currentReaction[6].size(); ++j)
		{
			currentReaction[5][j] += 0.1;
			currentReaction[6][j] += 0.1;
		}
		break;
	case PLAY:
		//aumentar juego
		for (size_t j = 0; j < currentReaction[2].size(); ++j)
		{
			currentReaction[2][j] += 0.1;
		}
		break;
	case GROOM:
		//aumentar sociabilidad
		for (size_t j = 0; j < currentReaction[0].size(); ++j)
		{
			currentReaction[0][j] += 0.1;
			currentReaction[4][j] += 0.1;
		}
		break;
	case SLEEP:
		//aumentar desconf
		for (size_t j = 0; j < currentReaction[6].size(); ++j)
		{
			currentReaction[5][j] += 0.1;
			currentReaction[6][j] += 0.1;
		}
		break;
	case HUNT:
		//aumentar agresiv
		for (size_t j = 0; j < currentReaction[1].size(); ++j)
		{
			currentReaction[1][j] += 0.1;
		}
		break;
	default:
		break;
	}

	return adjustProbability(currentReaction);
}
void generalState::setCurrentState(vector<int>  state)
{
	for (size_t i=0; i<2; i++)
	{
		generalState::currentState[i] = state[i];
	}
}
void generalState::setNextState(vector<int> nextStat)
{
	for (size_t i = 0; i < 2; i++)
	{
		generalState::nextState[i] = nextStat[i];
	}
}
void generalState::setSensorThreshold(vector<int> sensorThres)
{
	for (size_t i = 0; i < Nsensor; i++)
	{
		generalState::sensorThreshold[i] = sensorThres[i];
	}
}
void generalState::setThresholdReaction(vector<vector<double>> thresReac)
{
	for (size_t i = 0; i < Nstates; i++)
	{
		for (size_t j = 0; j < (Nstates); j++)
		{
			generalState::thresholdReaction[i][j] = thresReac[i][j];
		}
	}
}
generalState::~generalState()
{
}

