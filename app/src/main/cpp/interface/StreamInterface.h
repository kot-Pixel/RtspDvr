//
// Created by Admin on 2024/11/16.
//

#include <document.h>
#include "Function.h"
#include "zmq.h"
#include <thread>
#include <queue>
#include "../streamer/include/log_utils.h"

using namespace rapidjson;

#ifndef SOCKECTDEMO2_STREAMINTERFACE_H
#define SOCKECTDEMO2_STREAMINTERFACE_H

class StreamInterface {
public:
    StreamInterface();
    virtual ~StreamInterface() = default;
    void reqLooperInner();
    FunctionMapper* mapper;

private:
    //parse string to function invoke
    Document mainJsonParse;

    Function invokeStringParse(const char* jsonStr);
    void startRequestLooper();
    void* mInterfaceZmqContext = nullptr;
    void* mInterfaceZmqSocket = nullptr;
    bool mInterfaceBindState = false;
};


#endif //SOCKECTDEMO2_STREAMINTERFACE_H
