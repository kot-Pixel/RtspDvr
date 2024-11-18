//
// Created by Admin on 2024/11/16.
//
#include "StreamInterface.h"

StreamInterface::StreamInterface() {
    mapper = new FunctionMapper();
    mInterfaceZmqContext = zmq_ctx_new();
    if (mInterfaceZmqContext == nullptr) return;
    LOGI("interface success create zmq context");
    mInterfaceZmqSocket = zmq_socket(mInterfaceZmqContext, ZMQ_REP);
    if (mInterfaceZmqSocket == nullptr) return;
    if (zmq_bind(mInterfaceZmqSocket, "ipc:///sdcard/interface.sock") != 0) {
        mInterfaceBindState = false;
    } else {
        mInterfaceBindState = true;
    }
    LOGI("interface udx bind state %d", mInterfaceBindState);
}

Function StreamInterface::invokeStringParse(const char *jsonStr) {
    mainJsonParse.Parse(jsonStr);
    // 从 JSON 反序列化
    if (mainJsonParse.IsObject()) {
        Function func = Function::from_json(mainJsonParse);
        return func;
    }
    return Function::error();
}

void StreamInterface::reqLooperInner() {
    if (mInterfaceBindState) {
        LOGI("interface reqLooper start looper");
        zmq_msg_t message;
        zmq_msg_init(&message);
        while (true) {
            int rc = zmq_recvmsg(mInterfaceZmqSocket, &message, 0);
            if (rc == -1) {
                LOGE("zmq rec msg produce error");
            } else {
                // 获取消息数据
                size_t message_size = zmq_msg_size(&message);
                LOGI("zmq rec interface msg, msg size is %lu", message_size);
                auto *message_data = (char *) zmq_msg_data(&message);
                Function function = invokeStringParse(message_data);
                std::vector<std::any> args;
                std:: string returnRet = function.getFunctionRet();
                for (const auto& param : function.getFunctionParam()) {
                    if (param.getParameterType() == "int") {
                        args.emplace_back(std::stoi(param.getParameterValue()));
                    } else if (param.getParameterType() == "double") {
                        args.emplace_back(std::stod(param.getParameterValue()));
                    } else if (param.getParameterType() == "bool") {
                        args.emplace_back(std::stod(param.getParameterValue()));
                    } else if (param.getParameterType() == "string") {
                        args.emplace_back(param.getParameterValue().data());
                    } else if (param.getParameterType() == "float") {
                        args.emplace_back(std::stof(param.getParameterValue()));
                    } else if (param.getParameterType() == "u_char") {
                        args.emplace_back(std::stoi(param.getParameterValue()));
                    } else {
                        LOGE("unsupported parameter type");
                    }
                }
                std::string reply;
                if (returnRet == "int") {
                    int result = std::any_cast<int>(
                            mapper->invokeFunction(function.getFunctionName(), args));
                    reply = std::to_string(result);
                } else if (returnRet == "double") {
                    auto result = std::any_cast<double>(
                            mapper->invokeFunction(function.getFunctionName(), args));
                    reply = std::to_string(result);
                } else if (returnRet == "bool") {
                    bool result = std::any_cast<bool>(
                            mapper->invokeFunction(function.getFunctionName(), args));
                    reply = std::to_string(result);
                } else if (returnRet== "string") {
                    reply = std::any_cast<std::string>(
                            mapper->invokeFunction(function.getFunctionName(), args));
                } else if (returnRet== "float") {
                    auto result = std::any_cast<float>(
                            mapper->invokeFunction(function.getFunctionName(), args));
                    reply = std::to_string(result);
                } else if (returnRet == "u_char") {
                    auto result = std::any_cast<u_char>(
                            mapper->invokeFunction(function.getFunctionName(), args));
                    reply = std::to_string(result);
                } else if (returnRet == "void") {
                    auto result = std::any_cast<std::string>(
                            mapper->invokeFunction(function.getFunctionName(), args));
                } else {
                    LOGE("unsupported return type");
                }

                zmq_msg_init_data(&message, (void *) reply.data(), reply.size(), nullptr, nullptr);
                int ret = zmq_msg_send(&message, mInterfaceZmqSocket, 0);
                if (rc < 0) {
                    LOGE("zmq reply msg produce failure");
                } else {
                    LOGE("zmq reply msg produce success");
                }
            }
        }
    }
}

void StreamInterface::startRequestLooper() {
    std::thread looperThread(&StreamInterface::reqLooperInner, this);
    looperThread.detach();
}
