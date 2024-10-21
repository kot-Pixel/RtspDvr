//
// Created by Admin on 2024/10/20.
//

#ifndef SOCKECTDEMO2_KTRTSPRENDER_H
#define SOCKECTDEMO2_KTRTSPRENDER_H

#include <GLES2/gl2.h>
#include <EGL/egl.h>
#include <EGL/eglext.h>

class KtRtspRender {
public:
    static KtRtspRender *createNew(ANativeWindow* window);
protected:
    explicit KtRtspRender(ANativeWindow *window);
    virtual ~KtRtspRender();
public:

    void initOpenGL();

    const char* vertexShaderSource = R"(
        attribute vec4 position;
        attribute vec2 texCoord;
        varying vec2 vTexCoord;
        void main() {
            gl_Position = position;
            vTexCoord = texCoord;
        }
    )";

    const char *fragmentShaderSource = R"(
        precision mediump float;
        varying vec2 vTexCoord;
        uniform sampler2D yTexture;
        uniform sampler2D uTexture;
        uniform sampler2D vTexture;
        void main() {
            // 获取 YUV 组件
            float y = texture2D(yTexture, vTexCoord).r;
            float u = texture2D(uTexture, vTexCoord).r - 0.5;
            float v = texture2D(vTexture, vTexCoord).r - 0.5;
            float r = y + 1.402 * v; // Red
            float g = y - 0.344136 * u - 0.714136 * v; // Green
            float b = y + 1.772 * u; // Blue
            gl_FragColor = vec4(r, g, b, 1.0);
        }
    )";
    const EGLint eglLintArray[5] = {
            EGL_RENDERABLE_TYPE,
            EGL_OPENGL_ES2_BIT,
            EGL_SURFACE_TYPE,
            EGL_WINDOW_BIT,
            EGL_NONE
    };
    const EGLint contextAttribution[3] = {
            EGL_CONTEXT_CLIENT_VERSION,
            2,
            EGL_NONE
    };
private:
    EGLDisplay display;
    EGLSurface surface;
    EGLContext context;
    GLuint program;
    ANativeWindow* mANativeWindow = nullptr;

    int ret = -1;
    void initEGLContext();
    void createProgram();

    static int checkOpenGLError();
    static GLuint  compileShader(GLenum type, const char* source);
};


#endif //SOCKECTDEMO2_KTRTSPRENDER_H
