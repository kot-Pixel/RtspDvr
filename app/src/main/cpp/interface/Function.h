//
// Created by Admin on 2024/11/16.
//
#include <string>
#include <utility>
#include <vector>
#include <map>
#include <typeindex>
#include <any>
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

    // Getter Methods
    std::string getFunctionName() const { return mFunctionName; }
    std::vector<Parameter> getFunctionParam() const { return mFunctionParam; }
    std::string getFunctionRet() const { return mFunctionRet; }
private:
    std::string mFunctionName;
    std::vector<Parameter> mFunctionParam;
    std::string mFunctionRet;
};


class FunctionMapper {
public:

    void registerFunction(const std::string& name, std::function<std::any(std::vector<std::any>)> func) {
        functionMap[name] = std::move(func);
    }

    std::any invokeFunction(const std::string& name, const std::vector<std::any>& args) {
        auto it = functionMap.find(name);
        if (it != functionMap.end()) {
            return it->second(args);
        } else {
            throw std::runtime_error("Function not found.");
        }
    }

private:
    std::map<std::string, std::function<std::any(std::vector<std::any>)>> functionMap;
};




#endif //SOCKECTDEMO2_FUNCTION_H