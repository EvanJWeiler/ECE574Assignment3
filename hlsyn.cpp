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
#include "functionsIO.hpp"
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
	vector<Operation> allOps;
	vector<Operation> *allOperations = &allOps;
	// IO, we must set up the writing of the variables to the outputs [Handled once AllVars returns]
	// Create vector of all Variables [Done with allVariables]
	// Map the Operations TODO:[Not Done]
	allVariables = inputFileToVariables(argv[1], allOperations);
	
	// Compute dependency for operations.
	for (int i = 0; i < (*allOperations).size(); i++) {
		dependentOperation(&(*allOperations).at(i), allOperations);
	}

	// Send to the force directed scheduling.
    schedule_ASAP(*allOperations);
    schedule_ALAP(*allOperations, atoi(argv[2]));
    computeProbabilities(*allOperations, atoi(argv[2]));
    vector<Resource> resDistr = computeTypeDistributions(*allOperations, atoi(argv[2]));

	// Evan's part (actual comment TBD)

	// writing to output file
	ofstream oFile;
	oFile.open(argv[2]);
	oFile << "'timescale 1ns / 1ps" << endl;
	oFile << "module TimeVerifier(Clk, Rst, CStart, CEnd, ErrorRst, Error);" << endl;
	string tempstring = "";
	for (Variable var : allVariables) {
		if (var.getVarType().compare("input") == 0 || var.getVarType().compare("output") == 0) {
			tempstring += var.getName() + ", ";
		}
	}
	oFile << tempstring << ");" << endl;
	oFile << "   input Clk, Rst;" << endl;
	for (Variable var : allVariables) {
		oFile << "   " << var.getVarType();
		if (var.getVarType().compare("output") == 0)
			oFile << " reg";
		if (var.getUnSigned() == false)
			oFile << " signed";
		if (var.getVarType().compare("parameter") == 0) {
			oFile << var.getName() << " = " << i << "," << endl << "			";
		}
		else {
			oFile << " [" << var.getBitWidth() - 1 << ":0] " << var.getName() << ";" << endl;
		}
		i = i + 1;
	}

	oFile << "always @(";
	//sensing for variable in for if statement (output sample line 23)
	for (Variable var : allVariables) {
		if (var.getName().compare("State") == 0)
			oFile << "State, ";
		if (var.getName().compare("CStart") == 0)
			oFile << "CStart, ";
		if (var.getName().compare("CEnd") == 0)
			oFile << "CEnd, ";
		if (var.getName().compare("ErroeRst") == 0)
			oFile << "ErrorRst, ";
		if (var.getName().compare("dLTe") == 0)
			oFile << "dLTe";
	}

	oFile << ") begin" << endl;


	

	


	
	// need to make some loop to iterate through and find how many param variables we have 
	// use that loop to determine code below
	oFile.close();
	//second param var
	//third param var
	//to wherever we need
	//will be contained in loop


//    outputFileCreate(allVariables, argv[3]);
	
	return 0;
};;

	//second param var
	//third param var
	//to wherever we need
	//will be contained in loop
//    outputFileCreate(allVariables, argv[3]);
	
	return 0;
};
