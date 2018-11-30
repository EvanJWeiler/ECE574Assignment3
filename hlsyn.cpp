/*
Name: Brendan Cassidy, Diego Alcantra, Evan Weiler, Logan Knott
Project: ECE 474/574 Project 3
Description:
This project creates an implementation of the Force Directed Schedule taking in the following
format for command line args:
hlsyn cFile latency(int) verilogFile(output.v)
*/
#include <string>
#include <iostream>
#include <fstream>
#include <vector>
#include <sstream>
#include <algorithm>
#include "Variable.hpp"
#include "functionsScheduling.hpp"
#include "Resource.hpp"

using namespace std;

int main(int argc, char *argv[]) {

	if (argc != 4) {
		cout << "Usage: hlsyn cFile latency verilogFile" << endl;
		return EXIT_FAILURE;
	}

	vector<Variable> allVariables;
	vector<Resource> resourceTypes;
	vector<Operation*> allOps;
	vector<Operation*> *allOperations = &allOps;
	// IO, we must set up the writing of the variables to the outputs [Handled once AllVars returns]
	// Create vector of all Variables [Done with allVariables]
	// Map the Operations TODO:[Not Done]
	allVariables = inputFileToVariables(argv[1], (allOperations));

	// Compute dependency for operations.
	for (unsigned int i = 0; i < (*allOperations).size(); i++) {
		dependentOperation((allOperations)->at(i), (allOperations));
	}

	if ((*allOperations).size() == 0) {
		cout << "ERROR: File Empty." << endl;
		return EXIT_FAILURE;
	}

	// Send to the force directed scheduling.
	schedule_ASAP(*allOperations);
	schedule_ALAP(*allOperations, atoi(argv[2]));
	computeProbabilities(*allOperations, atoi(argv[2]));
	vector<Resource> resDistr = computeTypeDistributions(*allOperations, atoi(argv[2]));
	computeForces(resDistr, *allOperations);
	scheduleNodes(*allOperations);
	// Evan's part (actual comment TBD)
	//Check that we have valid latency
	for (Operation *currOp : *allOperations) {
		if (currOp->getAsapTime() > atoi(argv[2])) {
			cout << "Error: Latency issue" << endl;
			return EXIT_FAILURE;
		}
	}
	// writing to output file

	outputFileCreate(allVariables, argv[3], allOperations, atoi(argv[2]));

	return 0;
};