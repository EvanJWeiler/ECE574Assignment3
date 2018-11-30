#include "functionsIO.hpp"
#include "math.h"
//#include <cmath>

vector<Variable> inputFileToVariables(string fileName, vector<Operation*> *allOps)
{
	//Variable Declaration
	int maxDatawidth, firstInd, secondInd, currLoop = 0, ifNum = 0, pos = 0, operandCount = 0, count = 0;
	bool flagIncDec, validVar = false;

	ifstream iFile;

	string loopType, loopVar, line, operand, currType, bitWidth, varNames, currName;
	string delimiter = ", ";
	string modules = "";

	Variable tempVar;
	vector<Variable> allVariables;
	vector<string> statements;
	vector<Variable> currOperand;
	vector<Operation*> allOperations = *allOps;
	iFile.open(fileName); //THIS IS THE ONE BOYS

	if (iFile.is_open()) {
		while (!iFile.eof()) {
			string val;
			count = 0;
			maxDatawidth = 0;
			flagIncDec = false;
			validVar = false;
			currOperand.clear();
			operand = "="; //Assume Reg

			getline(iFile, line); //Pull in line

			if (line.find("if") != string::npos && !line.empty()) {
				//Signal Going into an If statement, string for the variable.
				ifNum += 1;
				currLoop += 1;
				loopType = "if";
				firstInd = line.find("(");
				secondInd = line.find(")");
				loopVar = line.substr(firstInd + 2, secondInd - 6); //Fix when second if ends!!!
				statements.push_back(loopVar);

			}
			else if (line.find("else") != string::npos && !line.empty()) {
				//ifNum += 1;
				currLoop += 1;
				loopType = "else";
			}
			else if (line.find("}") != string::npos && !line.empty()) {
				//Signal and End Of Loop
				if (loopType.find("else")) {// != string::npos) {
					currLoop -= 1;
					loopVar = statements.at(0);
					//statements.pop_back();
				}
			}
			else if (line.find_first_not_of(" ") == string::npos) {
				continue; //Line of spaces
			}
			else if (line.find("=") == string::npos && !line.empty()) {
				//Make call to the compileVariables function
				allVariables = compileListOfVariables(line, allVariables, currType, bitWidth, varNames, tempVar, delimiter, currName, pos);
			}
			else {
				//Make call to the create operations function
				compileListOfOperations(line, allVariables, &allOperations,
					currOperand, val, count, validVar, currLoop, maxDatawidth, operandCount, loopType, loopVar, flagIncDec);
			}
		}
	}
	else {
		cout << "ERROR" << endl;
		std::terminate();
		//TODO: Return failure ;
	}
	*allOps = allOperations;
	return allVariables;
}

void outputFileCreate(vector<Variable> allVariables, string outFile, vector<Operation*> *allOps, int latency) {
	ofstream oFile;
	int i = 1;
	//oFile.open("C:/Users/lknot/OneDrive/Desktop/output.txt");
	oFile.open(outFile);
	oFile << "`timescale 1ns / 1ps" << endl;
	oFile << "module TimeVerifier( ";
	string tempstring = "";
	string temp2 = "";
	for (Variable var : allVariables) {
		if (var.getVarType().compare("input") == 0 || var.getVarType().compare("output") == 0) {
			tempstring += var.getName() + ", ";
		}
	}
	oFile << tempstring << " Clk, Rst, CStart, CEnd, ErrorRst, Error);" << endl;
	oFile << "   input Clk, Rst;" << endl;
	oFile << "   input CStart, CEnd, ErrorRst;" << endl;
	oFile << "   output Error;" << endl << endl;
	for (Variable var : allVariables) {
		if (var.getVarType().compare("output") == 0) {
			oFile << "   " << var.getVarType();
			oFile << " reg";
			if (var.getUnSigned() == false)
				oFile << " [" << var.getBitWidth() - 1 << ":0] " << var.getName() << ";" << endl;
			else
				oFile << " unsigned [" << var.getBitWidth() - 1 << ":0] " << var.getName() << ";" << endl;
		}
		if (var.getVarType().compare("variable") == 0) {
			oFile << "   " << "reg";
			if (var.getName().length() == 1 && var.getUnSigned() == false) {
				oFile << " [" << var.getBitWidth() - 1 << ":0] " << var.getName() << ";" << endl;
			}
			else if (var.getUnSigned() == false) {
				oFile << " " << var.getName() << ";" << endl;
			}
			else if (var.getName().length() == 1 && var.getUnSigned() == true) {
				oFile << " unsigned [" << var.getBitWidth() - 1 << ":0] " << var.getName() << ";" << endl;
			}
			else {
				oFile << " unsigned " << var.getName() << ";" << endl;
			}
		}

		if ((var.getUnSigned() == false) && var.getVarType().compare("variable") != 0 && var.getVarType().compare("output") != 0) {
			oFile << "   " << var.getVarType();
			oFile << " [" << var.getBitWidth() - 1 << ":0] " << var.getName() << ";" << endl;
		}
		if ((var.getUnSigned() == true) && var.getVarType().compare("variable") != 0 && var.getVarType().compare("output") != 0) {
			oFile << "   " << var.getVarType();
			oFile << " unsigned [" << var.getBitWidth() - 1 << ":0] " << var.getName() << ";" << endl;
		}
	}
	oFile << endl << endl << endl;
	for (Operation* op : *allOps) {
		if (op->getScheduledTime() > i)
			i = op->getScheduledTime();
	}
	int numBits = int(pow(i + 2, .5));
	//    int numBits = 3;

	oFile << "   parameter S_CycleEnd = " << i + 1 << "," << endl;
	while (i >> 0) {
		oFile << "	     State" << i << " = " << i << "," << endl;
		i = i - 1;
	}
	oFile << "	     S_Wait = 0;" << endl;
	oFile << "   reg [" << numBits << ":0] State, StateNext;" << endl;
	oFile << endl << endl;
	oFile << "always @(CStart, CEnd, ErrorRst, Error) begin";
	oFile << endl << endl;
	oFile << "	case(State)" << endl;
	oFile << "	   S_Wait: begin" << endl;
	oFile << "	      if(CStart == 1)" << endl;
	oFile << "	         StateNext <= State1;" << endl;
	oFile << "	      else" << endl;
	oFile << "	         StateNext <= S_Wait;" << endl;
	oFile << "	   end" << endl;
	int countLoop1 = 0, countLoop1e = 0, countLoop2 = 0, countLoop2e = 0, countLoop3 = 0, j = 1;
	bool state1 = false;
	bool alreadyPrinted1 = false, alreadyPrinted1e = false, alreadyPrinted = false;
	for (Operation* op : *allOps) {
		if (op->getScheduledTime() > i)
			i = op->getScheduledTime();
	}
	//Count amount in the 'if'
	for (Operation* op : *allOps) {
		if (op->getLoopContain() == 1 && op->getloopType().find("if")) {
			countLoop1 += 1;
		}
	}
	//Count amount in the 'else'
	for (Operation* op : *allOps) {
		if (op->getLoopContain() == 1 && op->getloopType().find("else")) {
			countLoop1e += 1;
		}
	}
	//Count amount in the nested 'if'
	for (Operation* op : *allOps) {
		if (op->getLoopContain() == 2 && op->getloopType().find("if")) {
			countLoop2 += 1;
		}
	}
	//Count amount in the 2nd nested 'else'
	for (Operation* op : *allOps) {
		if (op->getLoopContain() == 2 && op->getloopType().find("else")) {
			countLoop2e += 1;
		}
	}
	//Count amount in the third nested 'if'
	for (Operation* op : *allOps) {
		if (op->getLoopContain() == 3 && op->getloopType().find("if")) {
			countLoop3 += 1;
		}
	}
	//for (Variable var : allVariables)
	while (j <= latency) {
		if (state1 == false) {
			oFile << "	   State" << j << ": begin" << endl;
			alreadyPrinted1 = false;
			alreadyPrinted1e = false;
			state1 = true;
		}
		for (Operation* op : *allOps) {
			if (op->getScheduledTime() == j) {
				if (op->getLoopContain() == 1) {
					//Some type of loop which type
					if (op->getloopType().find("if") != string::npos && alreadyPrinted1 == false) {
						oFile << "	      " << "if(" << op->getloopVar() << ") begin " << endl;
						alreadyPrinted1 = true;
					}
					else if (op->getloopType().find("else") != string::npos && alreadyPrinted1e == false) {
						if (countLoop1 > 1 && alreadyPrinted1 == true) {
							alreadyPrinted1 = false;
							oFile << "	      end" << endl;
						}
						else if (alreadyPrinted1 == false) {
							oFile << "	      " << "if(" << op->getloopVar() << ") begin " << endl;
							oFile << "	      end" << endl;
						}
						oFile << "	      " << "else begin " << endl;
						alreadyPrinted1e = true;
					}
				}
				if (op->getLoopContain() == 2 && alreadyPrinted == false && op->getloopType().find("if") != string::npos) {
					for (Operation* op2 : *allOps) {
						if (op2->getLoopContain() == 1) {
							std::string temp = op2->getloopVar();
							oFile << "          " << "if(" << temp << ") begin " << endl;
							break;
						}
					}
					oFile << "                 " << "if(" << op->getloopVar() << ") begin " << endl;
					alreadyPrinted = true;
				} //Nested else stuff
				if (op->getLoopContain() == 2 && op->getloopType().find("else") != string::npos) {
					oFile << "          " << "if(csa1) begin " << endl;
					oFile << "                 " << "else begin " << endl;
					alreadyPrinted = true;
				}
				if (op->getLoopContain() == 3) {//neseted if-else-if
					for (Operation* op2 : *allOps) {
						if (op2->getLoopContain() == 1) {
							std::string temp = op2->getloopVar();
							oFile << "          " << "if(" << temp << ") begin " << endl;
							break;
						}
					}
					oFile << "                 " << "else begin " << endl;
					oFile << "                           " << "if(csa3) begin " << endl;
					alreadyPrinted = true;
				}
				oFile << "	      " << op->getOperationOutput() << ";" << endl;

				//Need to check if next operation is in the loop other ;wise end.
				//For a 1 layer nested if statement
				if (op->getloopType().find("if") != string::npos && op->getLoopContain() == 1 && countLoop1 == 1) {
					oFile << "	      end" << endl;
					countLoop1 -= 1;
				}
				else if (op->getloopType().find("if") != string::npos && op->getLoopContain() == 1 && countLoop1 > 1)
					countLoop1 -= 1;
				//For a 1 layer nested else statement
				if (op->getloopType().find("else") != string::npos && op->getLoopContain() == 1 && countLoop1e == 1) {
					oFile << "	      end" << endl;
					alreadyPrinted1e = false;
					countLoop1e -= 1;
				}
				else if (op->getloopType().find("else") != string::npos && op->getLoopContain() == 1 && countLoop1e > 1)
					countLoop1e -= 1;
				//For 2nd layer nested if statement
				if (op->getLoopContain() == 2 && countLoop2 == 1 && op->getloopType().find("if") != string::npos) {
					oFile << "	      end" << endl;
					countLoop2 -= 1;
				}
				else if (op->getLoopContain() == 2 && countLoop2 > 1) {
					countLoop2 -= 1;
				}
				if (op->getLoopContain() == 2 && op->getloopType().find("else") != string::npos) {
					oFile << "	      	      end" << endl;
					oFile << "	      end" << endl;

				}
				if (op->getLoopContain() == 3) {
					oFile << "	      	      	      end" << endl;
					oFile << "	      	      end" << endl;
					oFile << "	      end" << endl;
				}
			}
		}
		if (alreadyPrinted1e == true && countLoop1e >= 1) {
			oFile << "	      end" << endl;
		}
		//add to end
		if (j < latency)
			oFile << "	      StateNext <= State" << j + 1 << ";" << endl;
		else if (j == latency)
			oFile << "	      StateNext <= S_CycleEnd;" << endl;
		oFile << "	   end" << endl;

		state1 = false;
		j = j + 1;
	}

	oFile << "	   S_CycleEnd: begin" << endl;
	oFile << "	      if(Rst == 1)" << endl;
	oFile << "	         StateNext <= S_Wait;" << endl;
	oFile << "	      else" << endl;
	oFile << "	         StateNext <= S_CycleEnd;" << endl;
	oFile << "	   end" << endl;
	oFile << "	endcase" << endl;
	oFile << "   end" << endl;
	oFile << "   always @(posedge Clk) begin" << endl;
	oFile << "	if (Rst == 1) begin" << endl;
	oFile << "	   State <= S_Wait;" << endl;
	oFile << "	end" << endl;
	oFile << "	else begin" << endl;
	oFile << "	   State <= StateNext;" << endl;
	oFile << "	end" << endl;
	oFile << "   end" << endl;
	oFile << "endmodule" << endl;

	oFile.close();
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
	while ((pos = varNames.find(delimiter)) != (int)string::npos) {
		tempVar.setVarType(currType);
		if (bitWidth.at(0) == 'U')
			tempVar.setUnSigned(true);
		tempVar.setBitWidth(stoi(bitWidth.substr(begin, bitWidth.length() - 1)));

		currName = varNames.substr(0, pos);
		varNames.erase(0, pos + delimiter.length());
		tempVar.setName(currName);

		for (unsigned int i = 0; i < tempVar.getName().length(); i++) {//new needs testing
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
		for (unsigned int i = 0; i < tempVar.getName().length(); i++) {//new needs testing
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
	vector<Variable> currOperand, string val, int count, bool validVar, int currLoop,
	int maxDatawidth, int operandCount, string loopType, string loopVar, bool flagIncDec) {
	istringstream opStream(line);
	istringstream tempStream(line);
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
			for (unsigned int i = 0; i < allVariables.size(); i++) {
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
				std::terminate();//TODO: Return a failure code
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
	tempOperation->setOperationOutput(line);
	//Add info regarding the Loop to the operation.
	if (currLoop > 0) {
		tempOperation->setLoopContain(currLoop);
		tempOperation->setLoopCondition(loopType);
		tempOperation->setLoopVar(loopVar);
	}

	(allOps).push_back(tempOperation);
	operandCount += 1;
	*allOperations = allOps;
	return;
}

//Function will set the currOperations predecessor nodes and then set it as a successor of any of these nodes
void dependentOperation(Operation *currOperation, vector<Operation*> *allOperations) {
	Operation currOp = *currOperation;
	bool alreadyIn = false, passedCurrOp = false, added = false;

	//Null Check
	if ((*allOperations).empty() == true) return;

	for (unsigned int i = 0; i < (*allOperations).size(); i++) {
		added = false;
		//Check all output vars against inputs
		//Other loops cannot be predecessors but loops above them are.
		for (unsigned int j = 0; j < currOp.getInputs().size(); j++) {
			if ((currOp.getInputs().at(j).getName().compare((allOperations)->at(i)->getOutput().getName()) == 0)) {
				alreadyIn = false;
				//Check that its not already in the pred node list
				for (Operation* pred : currOp.getPredecessors()) {
					if ((allOperations)->at(i)->getOperationOutput().find(pred->getOperationOutput()) != string::npos) {
						alreadyIn = true;
					}
				}
				//This node is a predecessor
				if (alreadyIn == false) {
					(allOperations)->at(i)->addSuccessor(currOperation);
					currOp.addPredecessor((*allOperations).at(i));
					added = true;
				}
			}
		}
		if (currOp.getLoopContain() > 0 && added == false) {
			if (currOp.getOperationOutput().find((allOperations)->at(i)->getOperationOutput()) != string::npos) {
				passedCurrOp = true;
			}
			//            else if (passedCurrOp == false && (allOperations)->at(i)->getLoopContain() == 0) {
			//                (allOperations)->at(i)->addSuccessor(currOperation);
			//                currOp.addPredecessor((*allOperations).at(i));
			//            }
			else if (passedCurrOp == true && (allOperations)->at(i)->getLoopContain() == 0) {
				(allOperations)->at(i)->addPredecessor(currOperation);
				currOp.addSuccessor((*allOperations).at(i));
			}
		}
	}

	*currOperation = currOp;
	//Return
}