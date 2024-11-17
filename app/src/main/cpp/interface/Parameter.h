//
// Created by Admin on 2024/11/16.
//
#include <string>
#ifndef SOCKECTDEMO2_FUNCTIONKV_H
#define SOCKECTDEMO2_FUNCTIONKV_H

class Parameter {
public:
    Parameter(std::string parameterName, std::string parameterType, std::string parameterValue);
    virtual ~Parameter() = default;
    // Getter Methods
    std::string getParameterName() const { return mParameterName; }
    std::string getParameterType() const { return mParameterType; }
    std::string getParameterValue() const { return mParameterValue; }
private:
    std::string mParameterName;
    std::string mParameterType;
    std::string mParameterValue;
};

#endif //SOCKECTDEMO2_FUNCTIONKV_H
