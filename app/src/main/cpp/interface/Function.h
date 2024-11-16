//
// Created by Admin on 2024/11/16.
//
#include <string>
#include <vector>
#include "document.h"
#include "Parameter.h"

#ifndef SOCKECTDEMO2_FUNCTION_H
#define SOCKECTDEMO2_FUNCTION_H
using namespace rapidjson;

class Function {
public:
    Function(std::string functionName, std::vector<Parameter> functionParam, std::string functionRet);
    virtual ~Function() = default;

    static Function from_json(const Value& value) {
        std::string functionName = value["functionName"].GetString();

        // 反序列化 functionParam 数组
        std::vector<Parameter> functionParam;
        const Value& params = value["functionParam"];
        for (SizeType i = 0; i < params.Size(); ++i) {
            std::string paramName = params[i]["parameterName"].GetString();
            std::string paramType = params[i]["parameterType"].GetString();
            std::string paramValue = params[i]["parameterValue"].GetString();
            functionParam.emplace_back(paramName, paramType, paramValue);
        }
        std::string functionRet = value["functionRet"].GetString();
        return {functionName, functionParam, functionRet};
    }

    // 返回一个错误对象
    static Function error() {
        return {"Error", {}, "Error"};
    }
private:
    std::string mFunctionName;
    std::vector<Parameter> mFunctionParam;
    std::string mFunctionRet;
};


#endif //SOCKECTDEMO2_FUNCTION_H