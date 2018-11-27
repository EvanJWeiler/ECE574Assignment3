#include "functionsIO.hpp"

vector<Variable> inputFileToVariables(string fileName, vector<Operation*> *allOps)
{
	//Variable Declaration
	int maxDatawidth, pos = 0, operandCount = 0, count = 0;
	bool signedFlag, flagIncDec, validVar = false;

	ifstream iFile;

	string line, operand, currType, bitWidth, varNames, currName;
	string delimiter = ", ";
	string modules = "";

	Variable tempVar;
	vector<Variable> allVariables;
	vector<Variable> currOperand;
	vector<Operation*> allOperations = *allOps;
	iFile.open(fileName); //THIS IS THE ONE BOYS

	if (iFile.is_open()) {
		while (!iFile.eof()) {
			string val;
			count = 0;
			maxDatawidth = 0;
			signedFlag = true;
			flagIncDec = false;
			validVar = false;
			currOperand.clear();
			operand = "="; //Assume Reg

			getline(iFile, line); //Pull in line

								  //Decompisition of Input, Output, Wire
			if (line.find("=") == string::npos && !line.empty()) {
				//Make call to the compileVariables function
				allVariables = compileListOfVariables(line, allVariables, currType, bitWidth, varNames, tempVar, delimiter, currName, pos);
			}
			//else if (line.find("if") || line.find("else")) {
				//Found and IF or else
				//Look for that operation/variable. Need to contain everything until next '}' within an IF/Else, will need to break the schedule in a sense.
				//Set flag "Contained in first if" or "contained in second if" or "neither" if neither output is fine.
			//}
			//else if (line.find("for")) {
				//Found a for loop. EC worth 20pts. Run the for loop in a state then continue
			//}
			else {
				//Make call to the create operations function
				compileListOfOperations(line, allVariables, &allOperations,
					currOperand, val, count, validVar, signedFlag, maxDatawidth, operandCount, flagIncDec);
			}
		}
	}
	else {
		cout << "ERROR" << endl;
		//TODO: Return failure ;
	}
	*allOps = allOperations;
	return allVariables;
}

void outputFileCreate(vector<Variable> allVariables, string outFile)
{
	ofstream oFile;
	string modules = "";
	double critPath = 0.0;

	oFile.open(outFile);
	oFile << "'timescale 1ns / 1ps" << endl << endl;
	oFile << "module TopModule(";
	string tempString = "";
	for (Variable var : allVariables) {
		if (var.getVarType().compare("input") == 0 || var.getVarType().compare("output") == 0) {
			tempString += var.getName() + ", ";
		}
	}
	oFile << tempString << "Clk, Rst);" << endl;
	oFile << "   input Clk, Rst;" << endl;
	for (Variable var : allVariables) {
		oFile << "   " << var.getVarType();
		if (var.getVarType().compare("output") == 0)
			oFile << " reg";
		if (var.getUnSigned() == false)
			oFile << " signed";
		oFile << " [" << var.getBitWidth() - 1 << ":0] " << var.getName() << ";" << endl;
	}
	oFile << endl << modules << endl;
	oFile << "endModule" << endl;
	oFile.close();

	cout << "Critical Path : " << critPath << "ns" << endl;
}

//For seperation and creating variables by an input line.  Return the newest vector.
vector<Variable> compileListOfVariables(string line, vector<Variable> allVariables,
	string currType, string bitWidth, string varNames,
	Variable tempVar, string delimiter, string currName,
	int pos) {
	istringstream lineStream(line);
	lineStream >> currType >> bitWidth;
	getline(lineStream, varNames);
	varNames = varNames.substr(1, varNames.length() - 1);

	size_t begin = bitWidth.find_first_of("01234456789");
	tempVar.setUnSigned(false);	//Needs to be set false again after seeing an unsigned number
	while ((pos = varNames.find(delimiter)) != string::npos) {
		tempVar.setVarType(currType);
		if (bitWidth.at(0) == 'U')
			tempVar.setUnSigned(true);
		tempVar.setBitWidth(stoi(bitWidth.substr(begin, bitWidth.length() - 1)));

		currName = varNames.substr(0, pos);
		varNames.erase(0, pos + delimiter.length());
		tempVar.setName(currName);

		for (int i = 0; i < tempVar.getName().length(); i++) {//new needs testing
			if (isspace(tempVar.getName().at(i))) {
				if (!isspace(tempVar.getName().at(i - 1)))
					tempVar.setName(tempVar.getName().substr(0, (i)));
			}
		}

		allVariables.push_back(tempVar);
	}

	if (!varNames.empty()) {
		tempVar.setVarType(currType);
		if (bitWidth.at(0) == 'U')
			tempVar.setUnSigned(true);
		tempVar.setBitWidth(stoi(bitWidth.substr(begin, bitWidth.length() - 1)));
		tempVar.setName(varNames);
		for (int i = 0; i < tempVar.getName().length(); i++) {//new needs testing
			if (isspace(tempVar.getName().at(i))) {
				if (!isspace(tempVar.getName().at(i - 1)))
					string temp = tempVar.getName().substr(0, i);
				tempVar.setName(tempVar.getName().substr(0, i));
			}
		}

		allVariables.push_back(tempVar);
	}
	return allVariables;
}


void compileListOfOperations(string line, vector<Variable> allVariables, vector<Operation*> *allOperations,
	vector<Variable> currOperand, string val, int count, bool validVar, bool signedFlag,
	int maxDatawidth, int operandCount,
	bool flagIncDec) {
	istringstream opStream(line);
	istringstream tempStream(line);
	int i;
	if (line.compare("") == 0) //Continue on empty lines
		return;
	vector<Operation*> allOps = *allOperations;
	//determining dependencies
	string var1, var2, operand;
	tempStream >> var1;
	while (tempStream >> operand >> var2) {
		for (unsigned int i = 0; i < allVariables.size(); i++) {
			if (allVariables.at(i).getName().compare(var2) == 0) {
				for (unsigned int j = 0; j < allVariables.size(); j++) {
					if (allVariables.at(j).getName().compare(var1) == 0) {
						allVariables.at(j).addToDependencies(allVariables.at(i));
					}
				}
			}
		}
	}

	//Decompisition of Operator
	while (opStream >> val) {
		//Check the variables validity
		if (count == 0 || count == 2 || count == 4 || count == 6) {
			for (i = 0; i < allVariables.size(); i++) {
				//Make sure var exists
				validVar = false; //reset validVar flag after each iteration otherwise one valid var will make whole circuit "valid"
				if (allVariables.at(i).getName().compare(val) == 0) {
					validVar = true;
					currOperand.push_back(allVariables[i]);
					break;
				}

			}
			//Specifically in case we come across a "+ 1" or "- 1"
			if (count == 4 && val.compare("1") == 0) {
				validVar = true;
				flagIncDec = true;
			}
			if (validVar == false) {
				cout << "ERROR" << endl;
				//TODO: Return a failure code
			}
		}
		//Get the operand (= for reg, +,-... for others)
		if (count == 1 || count == 3)
			operand = val; //Update the operator class. Curroperand class should also contain all variables
		count += 1;
	}
	//Check if we need special operand for Inc and Dec
	if (flagIncDec == true) {
		if (operand.compare("+") == 0)
			operand = "++";
		else
			operand = "--";
	}
	//Create operand.
	Operation *tempOperation = new Operation;// (Operation*)malloc(sizeof(Operation));
	tempOperation->setOperation(operand);
	tempOperation->setOutput(currOperand.at(0));
	currOperand.erase(currOperand.begin());
	tempOperation->setInputs(currOperand);

	(allOps).push_back(tempOperation);
	operandCount += 1;
	*allOperations = allOps;
	return;
}

//Function will set the currOperations predecessor nodes and then set it as a successor of any of these nodes
void dependentOperation(Operation *currOperation, vector<Operation*> *allOperations) {
	Operation currOp = *currOperation;
	int i, j;
	//Null Check
	if ((*allOperations).empty() == true) return;

	for (i = 0; i < (*allOperations).size(); i++) {
		//Check all output vars against inputs
		printf("%lu", currOp.getInputs().size());
		for (j = 0; j < currOp.getInputs().size(); j++) {
			if (currOp.getInputs().at(j).getName().compare((allOperations)->at(i)->getOutput().getName()) == 0) {
				//This node is a predecessor
				(allOperations)->at(i)->addSuccessor(currOperation);
				currOp.addPredecessor((*allOperations).at(i));
			}
		}
	}
	//*allOperations = &allOps;
	*currOperation = currOp;
	//Return
}
