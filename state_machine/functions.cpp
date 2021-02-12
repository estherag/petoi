#pragma once
#include "catClass.h"
#include "functions.h"
#include "generalState.h"
#include <fstream>
#include <sstream>
#include <iostream>
#include <vector>
using namespace std;
enum personalityValue { DESCONFIANZA, JUEGO, SOCIABILIDAD, AGRESIVIDAD };
vector<double> readPersonality(std::string fileName)
{
	vector<double> personality(Npersonality);
	ifstream inFile;
	string line, field;
	inFile.open(fileName);
	vector<string> v;

	if (!inFile)
	{
		std::cout << "Unable to open file" << endl;
		exit(1); // terminate with error
	}

	while (getline(inFile, line))    // get next line in file
	{
		v.clear();
		stringstream ss(line);
		int i = 0;
		while (getline(ss, field, ';'))  // break line into comma delimitted fields
		{
			v.push_back(field);  // add each field to the 1D array
			personality[i] = stod(v[i]);
			i++;
		}

	}
	/*int j = 0;
	std::cout << "Vector string" << endl;
	for (size_t i = 0; i < v.size(); ++i)
	{

		std::cout << v[i] << "|";

	}
	std::cout << endl;
	std::cout << "Vector double" << endl;
	for (j = 0; j < v.size(); j++)
	{
		std::cout << personality[j] << "|";
	}*/
	inFile.close();
	return personality;
}
vector< vector<double> > readReaction(std::string fileName)
{
	vector< vector<double> > reaction(Nstates, vector<double>(Nstates));
	ifstream inFile;
	string line, field;
	inFile.open(fileName);
	vector<string> v;
	vector< vector<string> > array; // matrix
	if (!inFile)
	{
		std::cout << "Unable to open file" << endl;
		exit(1); // terminate with error
	}

	while (getline(inFile, line))    // get next line in file
	{
		v.clear();
		stringstream ss(line);
		int i = 0;
		while (getline(ss, field, ';'))  // break line into comma delimitted fields
		{
			v.push_back(field);  // add each field to the 1D array
		}
		array.push_back(v);
	}

	for (size_t i = 0; i < array.size(); ++i)
	{
		for (size_t j = 0; j < array[i].size(); ++j)
		{
			reaction[i][j] = stod(array[i][j]);
			//cout << reaction[i][j] << "|";
		}
		//cout << endl;
	}

	inFile.close();
	return reaction;
}
void printVector(vector<double> v)
{
	for (size_t i = 0; i < v.size(); ++i)
	{
		cout << v[i] << "|";
	}
	cout << endl;
}

void printVector(vector<int> v)
{
	for (size_t i = 0; i < v.size(); ++i)
	{
		cout << v[i] << "|";
	}
	cout << endl;
}

void printMatrix(vector< vector<double>> v)
{
	for (size_t i = 0; i < v.size(); ++i)
	{
		for (size_t j = 0; j < v[i].size(); ++j)
		{
			cout << v[i][j] << "|";
		}
		cout << endl;

	}
	cout << endl;
}

vector<vector<double>> adjustProbability(vector< vector<double>> matrix)
{
	double colSum = 0;
	for (size_t j = 0; j < Nstates; ++j)
	{
		colSum = 0;
		for (size_t i = 0; i < Nstates; ++i)
		{
			colSum += matrix[i][j];

		}
		for (size_t i = 0; i < Nstates; ++i)
		{
			matrix[i][j] = (matrix[i][j] / colSum);
		}
	}
	return matrix;
}

vector<vector<double>> changeReaction(int personalityIndex, vector< vector<double>> matrixPersonality)
{
	switch (personalityIndex)
	{
	case DESCONFIANZA: // 0
		for (size_t j = 0; j < matrixPersonality[6].size(); ++j)
		{
			matrixPersonality[5][j] += 0.1;
			matrixPersonality[6][j] += 0.1;
		}
		break;
	case JUEGO: // juego
		for (size_t j = 0; j < matrixPersonality[2].size(); ++j)
		{
			matrixPersonality[2][j] = 0.1 + matrixPersonality[2][j];
		}
		break;
	case SOCIABILIDAD: // sociabilidad
		for (size_t j = 0; j < matrixPersonality[0].size(); ++j)
		{
			matrixPersonality[0][j] += 0.1;
			matrixPersonality[4][j] += 0.1;
		}
		break;
	case AGRESIVIDAD: // agresividad
		for (size_t j = 0; j < matrixPersonality[1].size(); ++j)
		{
			matrixPersonality[1][j] += 0.1;
		}
		break;
	default:
		break;
	}

	cout << endl;
	double colSum;
	cout << "Antes de ajustar, matriz aumentada" << endl;
	printMatrix(matrixPersonality);
	for (size_t j = 0; j < Nstates; ++j)
	{
		colSum = 0;
		for (size_t i = 0; i < Nstates; ++i)
		{
			colSum += matrixPersonality[i][j];

		}
		cout << colSum << " ";
	}
	cout << endl;

	return adjustProbability(matrixPersonality);
}

