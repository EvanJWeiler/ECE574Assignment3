#include "functionsIO.hpp"

vector<Variable> inputFileToVariables(string fileName, vector<Operation> *allOps)
{
	//Variable Declaration
	int i, maxDatawidth, pos = 0, flagExtendLocVar1, flagExtendLocVar2, operandCount = 0, count = 0;
	bool signedFlag, flagIncDec, validVar = false;

	ifstream iFile;

	string line, operand, currType, bitWidth, varNames, currName;
	string delimiter = ", ";
	string modules = "";

	Variable tempVar;
	vector<Variable> allVariables;
	vector<Variable> currOperand;
	vector<Operation> allOperations;

	iFile.open(fileName); //THIS IS THE ONE BOYS

	if (iFile.is_open()) {
		while (!iFile.eof()) {
			string val;
			count = 0;
			maxDatawidth = 0;
			flagExtendLocVar1 = 0;
			flagExtendLocVar2 = 0;
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
			else {
				//Make call to the create operations function
				allOperations = compileListOfOperations(line, allVariables, allOperations,
					currOperand, val, count, validVar, signedFlag, maxDatawidth, flagExtendLocVar1,
					flagExtendLocVar2, operandCount, flagIncDec);
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


vector<Operation> compileListOfOperations(string line, vector<Variable> allVariables, vector<Operation> allOperations,
										vector<Variable> currOperand, string val, int count, bool validVar, bool signedFlag,
										int maxDatawidth, int flagExtendLocVar1, int flagExtendLocVar2, int operandCount,
										bool flagIncDec) {
	istringstream opStream(line);
	istringstream tempStream(line);
	int i;
	if (line.compare("") == 0) {
		return allOperations;	//Continue on empty lines
	}

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
					//Default is signed, swap flag on unsigned
					if (allVariables[i].getUnSigned() == false)
						signedFlag = false;
					if (maxDatawidth < allVariables[i].getBitWidth())
						maxDatawidth = allVariables[i].getBitWidth();
					if (count == 2 && maxDatawidth != allVariables[i].getBitWidth() && allVariables[i].getUnSigned() == false)
						flagExtendLocVar1 = 1;
					if (count == 2 && maxDatawidth != allVariables[i].getBitWidth() && allVariables[i].getUnSigned() == true)
						flagExtendLocVar1 = 2;
					if (count == 4 && maxDatawidth != allVariables[i].getBitWidth() && allVariables[i].getUnSigned() == false)
						flagExtendLocVar2 = 1;
					if (count == 4 && maxDatawidth != allVariables[i].getBitWidth() && allVariables[i].getUnSigned() == true)
						flagExtendLocVar2 = 2;

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
	Operation tempOperation;
	tempOperation.setOperation(operand);
	tempOperation.setOutput(currOperand.at(0));
	currOperand.erase(currOperand.begin());
	tempOperation.setInputs(currOperand);
	operandCount += 1;
	allOperations.push_back(tempOperation);
	return allOperations;
}
