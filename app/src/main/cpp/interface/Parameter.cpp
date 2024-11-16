//
// Created by Admin on 2024/11/16.
//
#include "Parameter.h"

Parameter::Parameter(std::string parameterName, std::string parameterType,std::string parameterValue) {
    mParameterName = std::move(parameterName);
    mParameterType =  std::move(parameterType);
    mParameterValue =  std::move(parameterValue);
}