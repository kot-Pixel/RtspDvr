//
// Created by Tom on 2024/10/16.


#include "../include/sdpUtils.h"


// Base64解码函数
std::vector<uint8_t> base64Decode(const std::string& encoded) {
    BIO* bio = BIO_new_mem_buf(encoded.data(), -1);
    BIO* b64 = BIO_new(BIO_f_base64());
    BIO_set_flags(b64, BIO_FLAGS_BASE64_NO_NL); // 禁用换行符
    bio = BIO_push(b64, bio);

    std::vector<uint8_t> decoded(encoded.size());
    int decodedLength = BIO_read(bio, decoded.data(), encoded.size());
    decoded.resize(decodedLength);
    BIO_free_all(bio);

    return decoded;
}