#pragma once
#include <vector>
#include <string>
vector<double> readPersonality(std::string fileName);
vector<vector<double>> readReaction(std::string fileName);
void printVector(vector<double> v);
void printVector(vector<int> v);
void printMatrix(vector< vector<double>> v);
vector<vector<double>> adjustProbability(vector< vector<double>> matrix); //convertir en matriz estocástica, columnas suman 1
vector< vector<double>> changeReaction(int personalityIndex, vector< vector<double>> matrixPersonality);