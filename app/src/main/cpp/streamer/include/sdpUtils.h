//
// Created by Tom on 2024/10/16.
//

#ifndef SOCKECTDEMO2_SDPUTILS_H
#define SOCKECTDEMO2_SDPUTILS_H

#include <cstdint>
#include <iostream>
#include <sstream>
#include "openssl/evp.h"

std::vector<uint8_t> stpStringBase64Decode(const std::string& encoded);
std::pair<std::string, std::string> subByDelimiter(const std::string& s, char delimiter);
std::string extractSpropParameterSets(const std::string& fmtpLine);

#endif //SOCKECTDEMO2_SDPUTILS_H