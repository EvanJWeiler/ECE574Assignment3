#include <stdio.h>
#include <vector>
#include <algorithm>
#include "Operation.hpp"
#include "Resource.hpp"

void schedule_ASAP(std::vector<Operation> &allOperations);
void schedule_ALAP(std::vector<Operation> &allOperations, int latency);
void computeProbabilities(std::vector<Operation> &allOperations, int latency);
std::vector<Resource> computeTypeDistributions(std::vector<Operation> &allOperations, int latency);
void mappingToResource(Operation &op, std::vector<Operation> &resDistr, int ts);

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

void computeProbabilities(std::vector<Operation> &allOperations, int latency) {
    for (unsigned int i = 0; i < allOperations.size(); i++) {
        for (unsigned int j = 0; j <= latency; j++) { //do <= or equal to to have a index for one to latency (i.e each time stamp), we'll just ignore the zero index
            if (j >= allOperations.at(i).getAsapTime() && j <= allOperations.at(i).getAlapTime())
                allOperations.at(i).addProbability(1/float(allOperations.at(i).getAlapTime() - allOperations.at(i).getAsapTime() + 1));
            else
                allOperations.at(i).addProbability(0.0);
        }
    }
}



std::vector<Resource> computeTypeDistributions(std::vector<Operation> &allOperations, int latency) {
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
        for (unsigned int j = 0; j <= latency; j++) { //do <= or equal to to have a index for one to latency (i.e each time stamp), we'll just ignore the zero index
            resDistr.at(i).addProbability(0.0);
        }
    }
    
    for (unsigned int ts = 1; ts <= latency; ++ts) { //for every timestep
        for (unsigned int i = 0; i < allOperations.size(); ++i) { //for every operation
            if (allOperations.at(i).getOperation() == "*") //following else ifs map to resources declared above
                resDistr.at(1).addToProbabilityAtTimeStamp(allOperations.at(i).getProbabilities().at(ts), ts);
            else if (allOperations.at(i).getOperation() == "/" || allOperations.at(i).getOperation() == "%")
                resDistr.at(2).addToProbabilityAtTimeStamp(allOperations.at(i).getProbabilities().at(ts), ts);
            else //anything not MOD, DIV, or MUL goes to ALU
                resDistr.at(0).addToProbabilityAtTimeStamp(allOperations.at(i).getProbabilities().at(ts), ts);
        }
    }
    
    return resDistr;
}
