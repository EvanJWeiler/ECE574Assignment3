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
	oFile << "   input Clk, Rst;" << endl;
	oFile << "   CStart, CEnd, ErrorRst;" << endl;
	oFile << "   output reg Error;" << endl << endl;
	//question: is there a variable i can pull here to figure how many parameters I will need for a file? 
	//string tempstring = "";
	//get var code here
	// need to make some loop to iterate through and find how many param variables we have 
	// use that loop to determine code below
	oFile << "   reg [2:0] State, StateNext;" << endl <<endl;
	oFile << "   // Comb logic for outputs and next state transitions" << endl;
	oFile << "   always@(State, CStart, Cend, ErrorRst) begin" << endl;
	oFile << "		case (State);" << endl;
	
	//first param var
	//pull S_wait from somewhere
	oFile << " : begin" << endl;
	oFile << "				Error <= 0;" << endl;
	oFile << "				if(";
	//pull the cstart variable or whatever it is contained in
	oFile << " ==1) begin" << endl;
	oFile << "					StateNext <= ";
	//pull the S_Cycle1 var from somewhere
	oFile << "				end" << endl;
	oFile << "				else begin" << endl;
	oFile << "					StateNext <= " << endl;
	//pull S_wait from var from somewhere
	oFile << "				end" << endl;
	oFile << "			end" << endl;

	//second param var
	//third param var
	//to wherever we need
	//will be contained in loop
//    outputFileCreate(allVariables, argv[3]);
	
	return 0;
};
