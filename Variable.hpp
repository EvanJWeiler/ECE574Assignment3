#ifndef VARIABLE_HPP
#define VARIABLE_HPP

#include <string>
#include <iostream>

class Variable {
private:
	std::string name;
	std::string vartype;
	int bitwidth;
	bool unSigned;
	std::vector<Variable> dependencies;

public:
	Variable() { //default constructor
		this->name = "undefined";
		this->vartype = "undefined";
		this->bitwidth = 0;
		this->unSigned = false;
	}

	Variable(std::string name, std::string vartype, int bitwidth, bool unSigned) {
		this->name = name;
		this->vartype = vartype;
		this->bitwidth = bitwidth;
		this->unSigned = unSigned;
	}

	//getters
	std::string getName() const {
		return this->name;
	}
	std::string getVarType() const {
		return this->vartype;
	}
	int getBitWidth() const {
		return this->bitwidth;
	}
	bool getUnSigned() const {
		return this->unSigned;
	}
	std::vector<Variable> getDependecies() const {
		return this->dependencies;
	}

	//setters
	void setName(std::string name) {
		this->name = name;
	}
	void setVarType(std::string varType) {
		this->vartype = varType;
	}
	void setBitWidth(int bitwidth) {
		this->bitwidth = bitwidth;
	}
	void setUnSigned(bool unSigned) {
		this->unSigned = unSigned;
	}
	void addToDependencies(Variable var) {
		this->dependencies.push_back(var);
	}

	void toString() {
		std::cout << "Var Name: " << name << " Var type: " << vartype << " Bit Width: " << bitwidth << std::endl;
	}
};

#endif
