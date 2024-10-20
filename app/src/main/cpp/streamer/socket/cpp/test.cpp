//
// Created by Admin on 2024/10/20.
//

#include "../../../zeroMq/include/zmq.h"
#include "../../include/log_utils.h"

int main2() {
    // 创建 ZMQ 上下文
    void* context = zmq_ctx_new();

    // 创建 ZMQ 套接字 (ZMQ_REP 用于响应请求)
    void* responder = zmq_socket(context, ZMQ_REP);

    // 绑定到 Unix Domain Socket 地址
    int rc = zmq_bind(responder, "ipc:///tmp/zeromq.sock");
    if (rc != 0) {
        LOGE("Error binding to socket:");
        return -1;
    }

    while (true) {
        // 接收消息
        char buffer[10];
        zmq_recv(responder, buffer, 10, 0);
        LOGI("Received");

        // 发送回应
        zmq_send(responder, "World", 5, 0);
    }

    // 清理资源
    zmq_close(responder);
    zmq_ctx_destroy(context);

    return 0;
}