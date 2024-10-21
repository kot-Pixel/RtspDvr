//
// Created by Admin on 2024/10/20.
//

#include "../include/KtRtspRender.h"
#include "../../include/log_utils.h"


KtRtspRender *KtRtspRender::createNew(ANativeWindow *window) {
    return new KtRtspRender(window);
}

KtRtspRender::KtRtspRender(ANativeWindow *window) {
    mANativeWindow = window;
}

void KtRtspRender::initEGLContext() {
    EGLint numConfigs;
    EGLConfig config;
    eglChooseConfig(display, eglLintArray, &config, 1, &numConfigs);
    ret = checkOpenGLError();
    if (ret < 0)
        return;
    surface = eglCreateWindowSurface(display, config, mANativeWindow, nullptr);
    ret = checkOpenGLError();
    if (ret < 0)
        return;
    context = eglCreateContext(display, config, EGL_NO_CONTEXT, contextAttribution);
    ret = checkOpenGLError();
    if (ret < 0)
        return;
    eglMakeCurrent(display, surface, surface, context);
}

int KtRtspRender::checkOpenGLError() {
    GLenum error = glGetError();
    if (error != GL_NO_ERROR) {
        switch (error) {
            case GL_INVALID_ENUM:
                LOGE("OpenGL Error: GL_INVALID_ENUM");
                return -1;
            case GL_INVALID_VALUE:
                LOGE("OpenGL Error: GL_INVALID_VALUE");
                return -2;
            case GL_INVALID_OPERATION:
                LOGE("OpenGL Error: GL_INVALID_OPERATION");
                return -3;
            case GL_OUT_OF_MEMORY:
                LOGE("OpenGL Error: GL_OUT_OF_MEMORY");
                return -4;
            default:
                LOGE("OpenGL Error: Unknown error");
                return -5;
        }
    }
    return 0;
}

GLuint KtRtspRender::compileShader(GLenum type, const char *source) {
    GLuint shader = glCreateShader(type);
    glShaderSource(shader, 1, &source, nullptr);
    glCompileShader(shader);
    return shader;
}

void KtRtspRender::createProgram() {
    GLuint vertexShader = compileShader(GL_VERTEX_SHADER, vertexShaderSource);
    checkOpenGLError();
    GLuint fragmentShader = compileShader(GL_FRAGMENT_SHADER, fragmentShaderSource);
    checkOpenGLError();
    program = glCreateProgram();
    checkOpenGLError();
    glAttachShader(program, vertexShader);
    glAttachShader(program, fragmentShader);
    glLinkProgram(program);
    checkOpenGLError();
    glDeleteShader(vertexShader);
    glDeleteShader(fragmentShader);
}

void KtRtspRender::initOpenGL() {
    initEGLContext();
    createProgram();
    glViewport(0, 0, 1920, 1080);
    glClearColor(0.0f, 0.0f, 0.0f, 1.0f);
}

KtRtspRender::~KtRtspRender() = default;