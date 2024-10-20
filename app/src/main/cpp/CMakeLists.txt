# For more information about using CMake with Android Studio, read the
# documentation: https://d.android.com/studio/projects/add-native-code.html.
# For more examples on how to use CMake, see https://github.com/android/ndk-samples.

# Sets the minimum CMake version required for this project.
cmake_minimum_required(VERSION 3.22.1)

# Declares the project name. The project name can be accessed via ${ PROJECT_NAME},
# Since this is the top level CMakeLists.txt, the project name is also accessible
# with ${CMAKE_PROJECT_NAME} (both CMake variables are in-sync within the top level
# build script scope).
project("SocketClientDemo22")

set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=c++11 -fno-rtti -Wall -fexceptions -DXLOCALE_NOT_USED=1 -DNULL=0 -DNO_SSTREAM=1 -DSOCKLEN_T=socklen_t -UIP_ADD_SOURCE_MEMBERSHIP")

# OPEN SSL 的相关编译路径
set(OPENSSL_ROOT_DIR ${CMAKE_SOURCE_DIR}/openssl)
set(OPENSSL_INCLUDE_DIR ${OPENSSL_ROOT_DIR}/include)

set(OPENSSL_CRYPTO_LIB ${OPENSSL_ROOT_DIR}/lib/libcrypto.a)
set(OPENSSL_SSL_LIB ${OPENSSL_ROOT_DIR}/lib/libssl.a)

# 包含 OpenSSL 的头文件
include_directories(${OPENSSL_INCLUDE_DIR})


set(OPENSSL_LIVE555_DIR ${CMAKE_SOURCE_DIR}/live555/include)
# 添加 Live555 头文件的包含目录
include_directories("${OPENSSL_LIVE555_DIR}/UsageEnvironment/")
include_directories("${OPENSSL_LIVE555_DIR}/groupsock/")
include_directories("${OPENSSL_LIVE555_DIR}/liveMedia/")
include_directories("${OPENSSL_LIVE555_DIR}/BasicUsageEnvironment/")

set(LIVE555_BASIC_USAGE_ENVIRONMENT_LIB ${CMAKE_SOURCE_DIR}/live555/lib/libBasicUsageEnvironment.a)
set(LIVE555_GROUP_SOCK_LIB ${CMAKE_SOURCE_DIR}/live555/lib/libgroupsock.a)
set(LIVE555_LIVE_MEDIA_LIB ${CMAKE_SOURCE_DIR}/live555/lib/libliveMedia.a)
set(LIVE555_USAGE_ENVIRONMENT_LIB ${CMAKE_SOURCE_DIR}/live555/lib/libUsageEnvironment.a)


# OPEN SSL 的相关编译路径
set(zeroMq_ROOT_DIR ${CMAKE_SOURCE_DIR}/zeroMq)
set(zeroMq_INCLUDE_DIR ${zeroMq_ROOT_DIR}/include)

set(zeroMq_ZMQ_LIB ${zeroMq_ROOT_DIR}/lib/libzmq.a)

# 包含 OpenSSL 的头文件
include_directories(${zeroMq_INCLUDE_DIR})


# Creates and names a library, sets it as either STATIC
# or SHARED, and provides the relative paths to its source code.
# You can define multiple libraries, and CMake builds them for you.
# Gradle automatically packages shared libraries with your APK.
#
# In this top level CMakeLists.txt, ${CMAKE_PROJECT_NAME} is used to define
# the target library name; in the sub-module's CMakeLists.txt, ${PROJECT_NAME}
# is preferred for the same purpose.
#
# In order to load a library into your app from Java/Kotlin, you must call
# System.loadLibrary() and pass the name of the library defined here;
# for GameActivity/NativeActivity derived applications, the same library name must be
# used in the AndroidManifest.xml file.
add_executable(${CMAKE_PROJECT_NAME}
        # List C/C++ source files with relative paths to this CMakeLists.txt.
        streamer/cpp/DumpSink.cpp
        streamer/cpp/KtStreamClientState.cpp
        streamer/cpp/KtRtspClient.cpp
        streamer/cpp/sdpUtils.cpp
        streamer/cpp/NjdaStream.cpp
)

# Specifies libraries CMake should link to your target library. You
# can link libraries from various origins, such as libraries defined in this
# build script, prebuilt third-party libraries, or Android system libraries.
target_link_libraries(${CMAKE_PROJECT_NAME}
        # List libraries link to the target library
        android
        mediandk

        #Live555 lib
        ${LIVE555_USAGE_ENVIRONMENT_LIB}
        ${LIVE555_GROUP_SOCK_LIB}
        ${LIVE555_LIVE_MEDIA_LIB}
        ${LIVE555_BASIC_USAGE_ENVIRONMENT_LIB}

        #openSSL lib
        ${OPENSSL_CRYPTO_LIB}
        ${OPENSSL_SSL_LIB}

        ${zeroMq_ZMQ_LIB}
        log)