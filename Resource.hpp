#ifndef Resource_hpp
#define Resource_hpp

#include <stdio.h>
#include <string>
#include <iostream>
#include <vector>

class Resource {
private:
    std::string operation;
    std::vector<float> sumOfProbsForTimeStep;
    
public:
    Resource() {
        this->operation = "undefined";
    }
    Resource(std::string operation)  {
        this->operation = operation;
    }
    
    //getters
    std::string getOperation() const {
        return this->operation;
    }

    std::vector<float> getProbabilities() {
        return this->sumOfProbsForTimeStep;
    }
    
    //setters
    void setOperation(std::string operation) {
        this->operation = operation;
    }
    void addProbability(float probability) {
        this->sumOfProbsForTimeStep.push_back(probability);
    }
    void addToProbabilityAtTimeStamp(float probability, int timeStamp) {
        this->sumOfProbsForTimeStep.at(timeStamp) = probability + this->sumOfProbsForTimeStep.at(timeStamp);
    }
};

#endif /* Resource_hpp */
