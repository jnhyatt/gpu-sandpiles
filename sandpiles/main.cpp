#include "gl-link.h"
#include "log.h"
#include "windows-util.h"

#include <fstream>
#include <string>
#include <thread>
#include <vector>
#include <Windows.h>
#include <gl/GL.h>

std::string loadFile(const std::string& fileName) {
    std::ifstream ifs(fileName.c_str(),
                      std::ios::in | std::ios::binary | std::ios::ate);
    std::ifstream::pos_type fileSize = ifs.tellg();
    ifs.seekg(0, std::ios::beg);
    std::vector<char> bytes(fileSize);
    ifs.read(&bytes[0], fileSize);
    return std::string(&bytes[0], fileSize);
}

class CompileError: public std::runtime_error {
public:
    CompileError(const GLuint shader) : std::runtime_error(errorLog(shader)) {}

private:
    static std::string errorLog(const GLuint shader) {
        GLint logLength;
        glGetShaderiv(shader, GL_INFO_LOG_LENGTH, &logLength);
        std::vector<GLchar> messageBuffer(logLength);
        glGetShaderInfoLog(shader, logLength, nullptr, messageBuffer.data());
        return std::string(messageBuffer.begin(), messageBuffer.end());
    }
};

GLuint compile(const char* source, GLenum type) {
    GLuint shader = glCreateShader(type);
    glShaderSource(shader, 1, &source, nullptr);
    glCompileShader(shader);
    GLint ok;
    glGetShaderiv(shader, GL_COMPILE_STATUS, &ok);
    if (!ok) {
        throw CompileError(shader);
    }
    return shader;
}

LRESULT CALLBACK windowsMessageCallback(HWND hWindow, UINT message,
                                        WPARAM wParam, LPARAM lParam) {
    switch (message) {
    case WM_CLOSE:
        PostQuitMessage(0);
        return 0;
    default:
        return DefWindowProc(hWindow, message, wParam, lParam);
    }
}

int main() {
    using namespace sandbox;
    WindowsConsole console;
    Logger log(console, "Main");

    HINSTANCE hInstance = GetModuleHandle(NULL);

    log.verbose() << "Registering window class... ";
    WNDCLASSEX windowClass;
    ZeroMemory(&windowClass, sizeof(WNDCLASSEX));
    windowClass.cbSize = sizeof(WNDCLASSEX);
    windowClass.style = CS_HREDRAW | CS_VREDRAW | CS_OWNDC;
    windowClass.lpfnWndProc = windowsMessageCallback;
    windowClass.hInstance = hInstance;
    windowClass.hCursor = LoadCursor(NULL, IDC_ARROW);
    windowClass.lpszClassName = "sandbox";
    if (!RegisterClassEx(&windowClass)) {
        log.fatal() << WindowsError::last();
        return 0;
    }

    log.verbose() << "Done. ";

    log.verbose() << "Creating dummy window... ";
    HWND hTmpWindow = CreateWindowEx(WS_EX_OVERLAPPEDWINDOW, "sandbox",
                                     "Sandbox", WS_OVERLAPPEDWINDOW, 80, 80,
                                     1440, 900, NULL, NULL, hInstance, NULL);

    log.verbose() << "Acquiring dummy device context... ";
    HDC hTmpDeviceContext = GetDC(hTmpWindow);

    log.verbose() << "Selecting pixel format... ";
    PIXELFORMATDESCRIPTOR tmpFormatDescriptor;
    ZeroMemory(&tmpFormatDescriptor, sizeof(PIXELFORMATDESCRIPTOR));
    tmpFormatDescriptor.nSize = sizeof(PIXELFORMATDESCRIPTOR);
    tmpFormatDescriptor.nVersion = 1;
    tmpFormatDescriptor.dwFlags = PFD_DRAW_TO_WINDOW;
    tmpFormatDescriptor.iPixelType = PFD_TYPE_RGBA;
    tmpFormatDescriptor.cColorBits = 32;
    tmpFormatDescriptor.cAlphaBits = 8;
    tmpFormatDescriptor.cDepthBits = 24;

    int tmpFormat = ChoosePixelFormat(hTmpDeviceContext, &tmpFormatDescriptor);
    if (tmpFormat == 0) {
        log.fatal() << WindowsError::last();
        return NULL;
    }
    log.verbose() << "Setting dummy pixel format... ";
    if (!SetPixelFormat(hTmpDeviceContext, tmpFormat, &tmpFormatDescriptor)) {
        log.fatal() << WindowsError::last();
        return NULL;
    }
    log.verbose() << "Done. ";

    log.verbose() << "Creating legacy OpenGL rendering context... ";
    HGLRC hTmpRenderingContext = wglCreateContext(hTmpDeviceContext);
    if (!hTmpRenderingContext) {
        log.fatal() << "Error creating OpenGL legacy context. ";
        return NULL;
    }
    if (!wglMakeCurrent(hTmpDeviceContext, hTmpRenderingContext)) {
        log.fatal() << "Error making OpenGL legacy context current. ";
        return NULL;
    }
    log.info() << "OpenGL legacy context is current. ";
    log.verbose() << "OpenGL functions are now available. ";

    log.verbose() << "Linking WGL functions... ";
    linkWgl();

    log.info() << "Creating window... ";
    RECT windowSize;
    windowSize.top = 80;
    windowSize.left = 80;
    windowSize.bottom = 980;
    windowSize.right = 1520;
    AdjustWindowRect(&windowSize, WS_OVERLAPPEDWINDOW, FALSE);
    HWND hWindow = CreateWindowEx(
        WS_EX_OVERLAPPEDWINDOW, "sandbox", "Sandbox", WS_OVERLAPPEDWINDOW,
        windowSize.left, windowSize.top, windowSize.right, windowSize.bottom,
        NULL, NULL, hInstance, NULL);
    if (!hWindow) {
        log.fatal() << WindowsError::last();
        return 0;
    }
    log.verbose() << "Acquiring device context... ";
    HDC hDeviceContext = GetDC(hWindow);
    if (!hDeviceContext) {
        log.fatal() << WindowsError::last();
        return 0;
    }

    const int pixelFormatAttributes[]{
        WGL_DRAW_TO_WINDOW_ARB,
        GL_TRUE,
        WGL_SUPPORT_OPENGL_ARB,
        GL_TRUE,
        WGL_DOUBLE_BUFFER_ARB,
        GL_TRUE,
        WGL_PIXEL_TYPE_ARB,
        WGL_TYPE_RGBA_ARB,
        WGL_ACCELERATION_ARB,
        WGL_FULL_ACCELERATION_ARB,
        WGL_COLOR_BITS_ARB,
        32,
        WGL_ALPHA_BITS_ARB,
        8,
        WGL_DEPTH_BITS_ARB,
        24,
        WGL_STENCIL_BITS_ARB,
        8,
        WGL_SAMPLE_BUFFERS_ARB,
        GL_TRUE,
        WGL_SAMPLES_ARB,
        4,
        0,
    };

    int format;
    unsigned int formatCount;
    log.verbose() << "Selecting pixel format... ";
    if (!wglChoosePixelFormatARB(hDeviceContext, pixelFormatAttributes, nullptr,
                                 1, &format, &formatCount)) {
        log.fatal() << "Failed to select a compatible pixel format. ";
        return NULL;
    }
    if (formatCount == 0) {
        log.fatal() << "No compatible pixel formats found. ";
        return NULL;
    }
    PIXELFORMATDESCRIPTOR formatDescriptor;
    DescribePixelFormat(hDeviceContext, format, sizeof(PIXELFORMATDESCRIPTOR),
                        &formatDescriptor);
    SetPixelFormat(hDeviceContext, format, &formatDescriptor);
    log.info() << "Done. ";

    const int contextAttribs[]{
        WGL_CONTEXT_MAJOR_VERSION_ARB,
        4,
        WGL_CONTEXT_MINOR_VERSION_ARB,
        3,
        WGL_CONTEXT_PROFILE_MASK_ARB,
        WGL_CONTEXT_CORE_PROFILE_BIT_ARB,
        0,
    };

    log.info() << "Creating modern OpenGL context... ";
    HGLRC hRenderingContext =
        wglCreateContextAttribsARB(hDeviceContext, NULL, contextAttribs);
    if (!hRenderingContext) {
        log.fatal() << "Failed to create rendering context with the requested "
                       "attributes. ";
        return NULL;
    }
    wglMakeCurrent(NULL, NULL);
    wglDeleteContext(hTmpRenderingContext);
    ReleaseDC(hTmpWindow, hTmpDeviceContext);
    DestroyWindow(hTmpWindow);
    if (!wglMakeCurrent(hDeviceContext, hRenderingContext)) {
        log.fatal() << "Error making OpenGL modern context current. ";
        return NULL;
    }
    log.info() << "OpenGL modern context is current. ";

    log.verbose() << "Linking OpenGL functions... ";
    linkGl();
    log.verbose() << "Done. ";

    // Initialize
    GLuint fbos[2];
    GLuint color[2];
    glGenFramebuffers(2, fbos);
    glGenTextures(2, color);

    for (size_t i = 0; i < 2; i++) {
        GLuint texData[256];
        for (size_t j = 0; j < 256; j++) {
            texData[j] = j % 5;
        }

        glBindTexture(GL_TEXTURE_2D, color[i]);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
        glTexImage2D(GL_TEXTURE_2D, 0, GL_R32UI, 16, 16, 0, GL_RED_INTEGER,
                     GL_UNSIGNED_INT, texData);

        glBindFramebuffer(GL_FRAMEBUFFER, fbos[i]);
        glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0,
                               GL_TEXTURE_2D, color[i], 0);
    }

    GLuint vbo;
    GLuint vao;

    float screenQuad[12]{0.0f, 0.0f, 1.0f, 0.0f, 1.0f, 1.0f,
                         0.0f, 0.0f, 1.0f, 1.0f, 0.0f, 1.0f};

    glGenBuffers(1, &vbo);
    glBindBuffer(GL_ARRAY_BUFFER, vbo);
    glBufferData(GL_ARRAY_BUFFER, sizeof(screenQuad), screenQuad,
                 GL_STATIC_DRAW);

    glGenVertexArrays(1, &vao);
    glBindVertexArray(vao);
    glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, 0, nullptr);
    glEnableVertexAttribArray(0);

    GLuint sandpileProgram = glCreateProgram();
    try {
        log.verbose() << "Compiling sandpile.vs... ";
        GLuint vShader =
            compile(loadFile("sandpile.vs").c_str(), GL_VERTEX_SHADER);
        log.verbose() << "Compiling sandpile.fs... ";
        GLuint fShader =
            compile(loadFile("sandpile.fs").c_str(), GL_FRAGMENT_SHADER);
        glAttachShader(sandpileProgram, vShader);
        glAttachShader(sandpileProgram, fShader);
        glLinkProgram(sandpileProgram);
        log.verbose() << "Done. ";
    } catch (const CompileError& e) {
        log.fatal() << "Error compiling sandpile shader: " << e.what();
        return 0;
    }
    glUseProgram(sandpileProgram);
    GLint dim = glGetUniformLocation(sandpileProgram, "dim");
    glUniform2f(dim, 16, 16); // texture dimensions

    GLuint displayProgram = glCreateProgram();
    try {
        log.verbose() << "Compiling display.vs... ";
        GLuint vShader =
            compile(loadFile("display.vs").c_str(), GL_VERTEX_SHADER);
        log.verbose() << "Compiling display.fs... ";
        GLuint fShader =
            compile(loadFile("display.fs").c_str(), GL_FRAGMENT_SHADER);
        glAttachShader(displayProgram, vShader);
        glAttachShader(displayProgram, fShader);
        glLinkProgram(displayProgram);
        log.verbose() << "Done. ";
    } catch (const CompileError& e) {
        log.fatal() << "Error compiling display shader: " << e.what();
        return 0;
    }

    ShowWindow(hWindow, SW_SHOWDEFAULT);

    size_t pingPongIndex = 0;

    bool running = true;
    MSG message;
    while (running) {
        if (PeekMessage(&message, NULL, 0, 0, PM_REMOVE)) {
            if (message.message == WM_QUIT) {
                running = false;
            } else {
                TranslateMessage(&message);
                DispatchMessage(&message);
            }
        } else {
            // Render loop

            // Ping-pong render one fbo to the other using the sandpile shader
            glUseProgram(sandpileProgram);
            glBindFramebuffer(GL_FRAMEBUFFER, fbos[pingPongIndex]);
            glBindTexture(GL_TEXTURE_2D, color[1 - pingPongIndex]);
            glBindVertexArray(vao);
            glDrawArrays(GL_TRIANGLES, 0, 6);

            // Render using the visualization shader
            glUseProgram(displayProgram);
            glBindFramebuffer(GL_FRAMEBUFFER, 0);
            glBindTexture(GL_TEXTURE_2D, color[pingPongIndex]);
            glBindVertexArray(vao);
            glDrawArrays(GL_TRIANGLES, 0, 6);

            SwapBuffers(hDeviceContext);
            pingPongIndex = (pingPongIndex + 1) % 2;
        }
    }
}