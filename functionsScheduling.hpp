#include <iostream>
#include <vector>
#include <algorithm>
#include <map>
#include "functionsIO.hpp"
#include "Resource.hpp"

void schedule_ASAP(std::vector<Operation*> &allOperations);
void schedule_ALAP(std::vector<Operation*> &allOperations, int latency);
void computeProbabilities(std::vector<Operation*> &ops, int latency);
std::vector<Resource> computeTypeDistributions(std::vector<Operation*> &allOperations, int latency);
//void computeSelfForce(std::vector<Resource> &resourceDist, std::vector<Operation*> &allOps);
//void computePredSuccForces(std::vector<Resource> &resourceDist, std::vector<Operation*> &allOps);
void computeForces(std::vector<Resource> &resourceDist, std::vector<Operation*> &allOps);
void scheduleNodes(std::vector<Operation*> &allOps);

void schedule_ASAP(std::vector<Operation*> &allOperations) {
	int maxAsap = 0;
	int associatedDelay = 0;
	int currMax = 0;
	for (unsigned int i = 0; i < allOperations.size(); ++i) {
		maxAsap = 0;
		currMax = 0;
		if (allOperations.at(i)->getPredecessors().size() == 0)
			allOperations.at(i)->setAsapTime(1);
		else {
			for (unsigned int j = 0; j < allOperations.at(i)->getPredecessors().size(); j++) {
				if (allOperations.at(i)->getPredecessors().at(j)->getAsapTime() >= maxAsap) {
					maxAsap = allOperations.at(i)->getPredecessors().at(j)->getAsapTime();
					associatedDelay = allOperations.at(i)->getPredecessors().at(j)->getDelay();
					//Edge case for checking ALU then MUL
					if (maxAsap + associatedDelay > currMax)
						currMax = maxAsap + associatedDelay;
				}
			}
			allOperations.at(i)->setAsapTime(currMax);
			//            allOperations.at(i).setAsapTime(maxAsap + associatedDelay);
		}
	}
}

void schedule_ALAP(std::vector<Operation*> &allOperations, int latency) {
	int currMin = 777; //arbitrarily large number
	int associatedDelay = 0;
	for (int i = (int)allOperations.size() - 1; i >= 0; --i) {
		currMin = 777;
		if (allOperations.at(i)->getSuccessors().size() == 0)
			allOperations.at(i)->setAlapTime(latency - allOperations.at(i)->getDelay() + 1);
		else {
			for (unsigned int j = 0; j < allOperations.at(i)->getSuccessors().size(); j++) {
				if (allOperations.at(i)->getSuccessors().at(j)->getAlapTime() < currMin) {
					currMin = allOperations.at(i)->getSuccessors().at(j)->getAlapTime();
					associatedDelay = allOperations.at(i)->getSuccessors().at(j)->getDelay();
				}
			}
			//            if (currMin - associatedDelay < 0) std::terminate();
			if (allOperations.at(i)->getDelay() > 1) //case for delay greater than 1
				allOperations.at(i)->setAlapTime(currMin - allOperations.at(i)->getDelay());
			else //generic case
				allOperations.at(i)->setAlapTime(currMin - associatedDelay);
			if (allOperations.at(i)->getAlapTime() <= 0) allOperations.at(i)->setAlapTime(1);
		}
	}
}

void computeProbabilities(std::vector<Operation*> &ops, int latency) {
	for (unsigned int i = 0; i < ops.size(); i++) {
		for (int j = 1; j <= latency; j++) {
			if (j >= ops.at(i)->getAsapTime() and j <= ops.at(i)->getAlapTime()) {
				ops.at(i)->addProbability(1 / float(ops.at(i)->getAlapTime() - ops.at(i)->getAsapTime() + 1));
			}
			else {
				ops.at(i)->addProbability(0);
			}
		}
	}
}



std::vector<Resource> computeTypeDistributions(std::vector<Operation*> &allOperations, int latency) {
	std::vector<Resource> resDistr;
	Resource aluRes;
	Resource mulRes;
	Resource divRes;
	aluRes.setOperation("ALU");
	resDistr.push_back(aluRes);//0 index in resDistr will be for ALU
	mulRes.setOperation("MUL");
	resDistr.push_back(mulRes); //1 will be for MUL
	divRes.setOperation("DIV");
	resDistr.push_back(divRes);//2 will be for DIV and MOD

	for (unsigned int i = 0; i < resDistr.size(); i++) { //initialize vector of timestamps to zero to avoid badAccess later
		for (int j = 0; j < latency; j++) { //do <= or equal to to have a index for one to latency (i.e each time stamp), we'll just ignore the zero index
			resDistr.at(i).addProbability(0.0);
		}
	}

	for (int ts = 0; ts < latency; ++ts) { //for every timestep
		for (unsigned int i = 0; i < allOperations.size(); ++i) { //for every operation
			if (allOperations.at(i)->getOperation() == "*") //following else ifs map to resources declared above
				resDistr.at(1).addToProbabilityAtTimeStamp(allOperations.at(i)->getProbabilities().at(ts), ts);
			else if (allOperations.at(i)->getOperation() == "/" || allOperations.at(i)->getOperation() == "%")
				resDistr.at(2).addToProbabilityAtTimeStamp(allOperations.at(i)->getProbabilities().at(ts), ts);
			else //anything not MOD, DIV, or MUL goes to ALU
				resDistr.at(0).addToProbabilityAtTimeStamp(allOperations.at(i)->getProbabilities().at(ts), ts);
		}
	}

	return resDistr;
}

void computeForces(std::vector<Resource> &resourceDist, std::vector<Operation*> &allOps) {
	int resourceIndex;

	for (auto &op : allOps) { // self forces
		if (op->getOperation() == "*") {
			resourceIndex = 1;
		}
		else if (op->getOperation() == "/" or op->getOperation() == "%") {
			resourceIndex = 2;
		}
		else {
			resourceIndex = 0;
		}

		for (int i = op->getAsapTime(); i <= op->getAlapTime(); i++) {
			float runningSum = 0;

			for (int j = op->getAsapTime(); j <= op->getAlapTime(); j++) {
				if (i == j) {
					runningSum += resourceDist.at(resourceIndex).getProbabilities().at(j - 1) * (1 - op->getProbabilities().at(j - 1));
				}
				else {
					runningSum += resourceDist.at(resourceIndex).getProbabilities().at(j - 1) * (0 - op->getProbabilities().at(j - 1));
				}
			}

			op->addToForceAt(i, runningSum);
		}
	}

	// BEHOLD THE QUADRUPLE NESTED FOR LOOP
	// inefficiency at its finest, but it works so w/e
	for (auto &op : allOps) { // successor forces
		if (op->getOperation() == "*") {
			resourceIndex = 1;
		}
		else if (op->getOperation() == "/" or op->getOperation() == "%") {
			resourceIndex = 2;
		}
		else {
			resourceIndex = 0;
		}

		// successor force
		for (int i = op->getAsapTime(); i <= op->getAlapTime(); i++) {
			// i is now equal to the time frame we are currently looking at
			// iterate through all successor nodes, check if they overlap, calculate self force for all time frames past where the overlap ends (i.e. node 1 has time frame [1, 3] and node 2 has time frame [3, 4] calculate self force for node 3 at time slot 4, then add that to the successor force)
			for (unsigned int j = 0; j < op->getSuccessors().size(); j++) {
				if (i >= op->getSuccessors().at(j)->getAsapTime()) {
					float runningSum = 0;
					// calculate force for said successor from beginning of overlap +1 to alap time of successor
					for (int k = i + 1; k <= op->getSuccessors().at(j)->getAlapTime(); k++) {
						runningSum += op->getSuccessors().at(j)->getForceAt(k);
					}
					op->addToSuccForceAt(i, runningSum);
				}
			}
		}
	}

	// more quad loops
	for (auto &op : allOps) { // predecessor forces
		if (op->getOperation() == "*") {
			resourceIndex = 1;
		}
		else if (op->getOperation() == "/" or op->getOperation() == "%") {
			resourceIndex = 2;
		}
		else {
			resourceIndex = 0;
		}

		for (int i = op->getAsapTime(); i <= op->getAlapTime(); i++) {
			for (unsigned int j = 0; j < op->getPredecessors().size(); j++) {
				if (i <= op->getPredecessors().at(j)->getAlapTime()) {
					float runningSum = 0;

					for (int k = op->getPredecessors().at(j)->getAsapTime(); k < i; k++) {
						runningSum += op->getPredecessors().at(j)->getForceAt(k);
					}
					op->addToPredForceAt(i, runningSum);
				}
			}
		}
	}

	//combining forces
	for (auto &op : allOps) {
		for (int i = op->getAsapTime(); i < op->getAlapTime(); i++) {
			op->addToForceAt(i, op->getSuccForceAt(i) + op->getPredForceAt(i));
		}
	}

	// predecessor forces

	// if alap == asap, go to next node
	// iterate through all operations
	// for successor force: check if current op range overlaps with successor op range
	// if no overlap, successor force for that node is zero
	// for predecessor force: check if current op range overlaps with predecessor op range
}

void scheduleNodes(std::vector<Operation*> &allOps) {
	for (unsigned int i = 0; i < allOps.size(); i++) {
		int lowIndex = allOps.at(i)->getAsapTime();
		float currLow = allOps.at(i)->getForceAt(lowIndex);
		for (int j = allOps.at(i)->getAsapTime(); j <= allOps.at(i)->getAlapTime(); j++) {
			if (allOps.at(i)->getForceAt(j) < currLow) {
				lowIndex = j;
				currLow = allOps.at(i)->getForceAt(lowIndex);
			}
		}

		/*for (auto &pred : allOps.at(i)->getPredecessors()) {
			if (lowIndex < pred->getScheduledTime() + pred->getDelay()) {
				lowIndex += pred->getDelay();
			}
		}*/

		allOps.at(i)->scheduleAt(lowIndex);
	}
}
