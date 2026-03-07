#include "Window.hpp"
#include "Logger.hpp"

#include <glad/glad.h>
#include <SDL3/SDL_error.h>
#include <SDL3/SDL_init.h>
#include <SDL3/SDL_video.h>

#include <stdexcept>
#include <string>

namespace core {

Window::Window(const std::string& title, int width, int height) : m_width(width), m_height(height) {

    if (!SDL_Init(SDL_INIT_VIDEO | SDL_INIT_AUDIO)) {
        throw std::runtime_error(std::string("SDL_Init failed: ") + SDL_GetError());
    }

    SDL_GL_SetAttribute(SDL_GL_CONTEXT_MAJOR_VERSION, 3);
    SDL_GL_SetAttribute(SDL_GL_CONTEXT_MINOR_VERSION, 3);
    SDL_GL_SetAttribute(SDL_GL_CONTEXT_PROFILE_MASK, SDL_GL_CONTEXT_PROFILE_CORE);
    SDL_GL_SetAttribute(SDL_GL_DOUBLEBUFFER, 1);
    SDL_GL_SetAttribute(SDL_GL_DEPTH_SIZE, 0);

    m_window = SDL_CreateWindow(title.c_str(), width, height, SDL_WINDOW_OPENGL | SDL_WINDOW_HIGH_PIXEL_DENSITY);
    if (!m_window) {
        SDL_Quit();
        throw new std::runtime_error(std::string("SDL_CreateWindow failed: ") + SDL_GetError());
    }

    SDL_GLContext context = SDL_GL_CreateContext(m_window);
    if (!context) {
        SDL_DestroyWindow(m_window);
        SDL_Quit();
        throw new std::runtime_error(std::string("SDL_GL_CreateContext failed: ") + SDL_GetError());
    }
    m_context = context;

    SDL_GL_MakeCurrent(m_window, context);
    SDL_GL_SetSwapInterval(1);

    if (!gladLoadGLLoader(reinterpret_cast<GLADloadproc>(SDL_GL_GetProcAddress))) {
        SDL_GL_DestroyContext(context);
        SDL_DestroyWindow(m_window);
        SDL_Quit();
        throw std::runtime_error("gladLoadGLLoader failed");
    }

    int fb_w, fb_h;
    SDL_GetWindowSizeInPixels(m_window, &fb_w, &fb_h);
    glViewport(0, 0, fb_w, fb_h);

    Log::info("Window created: {}x{}", width, height);
    Log::info("GL Version : {}", reinterpret_cast<const char*>(glGetString(GL_VERSION)));
    Log::info("GL Renderer : {}", reinterpret_cast<const char*>(glGetString(GL_RENDERER)));
    Log::info("GL GLSL : {}", reinterpret_cast<const char*>(glGetString(GL_SHADING_LANGUAGE_VERSION)));
}

Window::~Window() {
    if (m_context) SDL_GL_DestroyContext(reinterpret_cast<SDL_GLContext>(m_context));
    if (m_window) SDL_DestroyWindow(m_window);
    SDL_Quit();
}

void Window::swap_buffers() {
    SDL_GL_SwapWindow(m_window);
}

} // namespace core
