#include <stdio.h>
#include <vector>
#include <algorithm>
#include <map>
#include "functionsIO.hpp"
#include "Resource.hpp"

void schedule_ASAP(std::vector<Operation> &allOperations);
void schedule_ALAP(std::vector<Operation> &allOperations, int latency);
void computeProbabilities(std::vector<Operation*> &ops, int latency);
std::vector<Resource> computeTypeDistributions(std::vector<Operation*> &allOperations, int latency);
void mappingToResource(Operation &op, std::vector<Operation> &resDistr, int ts);
void computeSelfForce(std::vector<Resource> &resourceDist, Operation &op);
void computePredSuccForce(std::vector<Resource> &resourceDist, Operation &op);
void scheduleNodes(std::vector<Operation*> &allOps);

void schedule_ASAP(std::vector<Operation> &allOperations) {
    int maxAsap = 0;
    int associatedDelay = 0;
    int currMax = 0;
    for (unsigned int i = 0; i < allOperations.size(); ++i) {
        maxAsap = 0;
        currMax = 0;
        if (allOperations.at(i).getPredecessors().size() == 0)
            allOperations.at(i).setAsapTime(1);
        else {
            for (int j = 0; j < allOperations.at(i).getPredecessors().size(); j++) {
                if (allOperations.at(i).getPredecessors().at(j)->getAsapTime() >= maxAsap) {
                    maxAsap = allOperations.at(i).getPredecessors().at(j)->getAsapTime();
                    associatedDelay = allOperations.at(i).getPredecessors().at(j)->getDelay();
                    //Edge case for checking ALU then MUL
                    if (maxAsap + associatedDelay > currMax)
                        currMax = maxAsap + associatedDelay;
                }
            }
            allOperations.at(i).setAsapTime(currMax);
//            allOperations.at(i).setAsapTime(maxAsap + associatedDelay);
        }
    }
}

void schedule_ALAP(std::vector<Operation> &allOperations, int latency) {
    int currMin = 777; //arbitrarily large number
    int associatedDelay = 0;
    for (int i = (int)allOperations.size() - 1; i >= 0; --i) {
        currMin = 777;
        if (allOperations.at(i).getSuccessors().size() == 0)
            allOperations.at(i).setAlapTime(latency);
        else {
            for (int j = 0; j < allOperations.at(i).getSuccessors().size(); j++) {
                if (allOperations.at(i).getSuccessors().at(j)->getAlapTime() < currMin) {
                    currMin = allOperations.at(i).getSuccessors().at(j)->getAlapTime();
                    associatedDelay = allOperations.at(i).getSuccessors().at(j)->getDelay();
                }
            }
            if (allOperations.at(i).getDelay() > 1) //case for delay greater than 1
                allOperations.at(i).setAlapTime(currMin - allOperations.at(i).getDelay());
            else //generic case
                allOperations.at(i).setAlapTime(currMin - associatedDelay);
        }
    }
}

void computeProbabilities(std::vector<Operation*> &ops, int latency) {
	for (unsigned int i = 0; i < ops.size(); i++) {
		for (int j = 1; j <= latency; j++) {
			if (j >= ops.at(i)->getAsapTime() and j <= ops.at(i)->getAlapTime()) {
				ops.at(i)->addProbability(1/float(ops.at(i)->getAlapTime() - ops.at(i)->getAsapTime() + 1));
			} else {
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
    
    for (unsigned int ts = 0; ts < latency; ++ts) { //for every timestep
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

void computeSelfForce(std::vector<Resource> &resourceDist, std::vector<Operation*> &allOps) {
    int resourceIndex;

    for (auto &op : allOps) {
        if (op->getOperation() == "*") {
            resourceIndex = 1;
        } else if (op->getOperation() == "/" or op->getOperation() == "%") {
            resourceIndex = 2;
        } else {
            resourceIndex = 0;
        }
        
        for (int i = op->getAsapTime(); i <= op->getAlapTime(); i++) {
            float runningSum = 0;

            for (int j = op->getAsapTime(); j <= op->getAlapTime(); j++) {
                if (i == j) {
                    runningSum += resourceDist.at(resourceIndex).getProbabilities().at(j - 1) * (1 - op->getProbabilities().at(j - 1));
                } else {
                    runningSum += resourceDist.at(resourceIndex).getProbabilities().at(j - 1) * (0 - op->getProbabilities().at(j - 1));
                }
            }

            op->addToForceAt(i, runningSum);
        }
    }
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

        allOps.at(i)->scheduleAt(lowIndex);
    }
}