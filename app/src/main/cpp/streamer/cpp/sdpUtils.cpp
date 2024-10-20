//
// Created by Tom on 2024/10/16.


#include "../include/sdpUtils.h"


std::pair<std::string, std::string> subByDelimiter(const std::string& s, char delimiter) {
    // 找到分隔符的位置
    size_t pos = s.find(delimiter);

    // 如果找不到分隔符，返回整个字符串作为第一部分，第二部分为空
    if (pos == std::string::npos) {
        return {s, ""};
    }

    // 分割字符串：前面的部分和后面的部分
    std::string beforeDelimiter = s.substr(0, pos);
    std::string afterDelimiter = s.substr(pos + 1);

    // 返回结果
    return {beforeDelimiter, afterDelimiter};
}

std::string extractSpropParameterSets(const std::string& fmtpLine) {
    std::string searchStr = "sprop-parameter-sets=";
    size_t start = fmtpLine.find(searchStr);
    if (start != std::string::npos) {
        start += searchStr.length();
        size_t end = fmtpLine.find(";", start);  // 查找下一个分号
        if (end == std::string::npos) {
            end = fmtpLine.length();
        }
        return fmtpLine.substr(start, end - start);
    }
    return "";
}

/**
 * 将对应Base64的字符串编码
 * @param encoded
 * @return
 */
std::vector<uint8_t> stpStringBase64Decode(const std::string& encoded) {
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