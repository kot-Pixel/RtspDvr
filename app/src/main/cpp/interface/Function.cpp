//
// Created by Admin on 2024/11/16.
//

#include "Function.h"

Function::Function(std::string functionName, std::vector<Parameter> functionParam, std::string functionRet) {
    mFunctionName = std::move(functionName);
    mFunctionParam = std::move(functionParam);
    mFunctionRet = std::move(functionRet);
}