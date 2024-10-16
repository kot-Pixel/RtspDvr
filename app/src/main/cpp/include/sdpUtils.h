//
// Created by Tom on 2024/10/16.
//

#ifndef SOCKECTDEMO2_SDPUTILS_H
#define SOCKECTDEMO2_SDPUTILS_H

#include <stdint.h>
#include <iostream>
#include "openssl/evp.h"

std::vector<uint8_t> base64Decode(const std::string& encoded);

#endif //SOCKECTDEMO2_SDPUTILS_H