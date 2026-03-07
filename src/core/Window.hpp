#pragma once

#include <string>

struct SDL_Window;

namespace core {

class Window {
public:
    Window(const std::string& title, int width, int height);
    ~Window();

    Window(const Window&) = delete;
    Window& operator=(const Window&) = delete;

    void swap_buffers();

    int get_width() const { return m_width; }
    int get_height() const { return m_height; }
    bool should_close() const { return m_should_close; }
    void set_should_close(bool v) { m_should_close = v; }

    SDL_Window* get_sdl_window() { return m_window; }

private:
    SDL_Window* m_window = nullptr;
    void* m_context = nullptr;
    int m_width = 0;
    int m_height = 0;
    bool m_should_close = false;
};

} // namespac core
