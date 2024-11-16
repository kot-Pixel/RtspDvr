//
// Created by Admin on 2024/11/16.
//
#include "StreamInterface.h"


StreamInterface::StreamInterface() {
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
//    startRequestLooper();
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

                function;
                std::string reply = "ok";
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
