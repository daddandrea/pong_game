#pragma once

#include "core/AudioManager.hpp"
#include "core/InputState.hpp"
#include "core/Window.hpp"
#include "scenes/SceneManager.hpp"

#include <memory>

namespace scenes   { class SceneManager; }
namespace renderer { class Renderer; }

namespace core {

class Application {

public:
    Application();
    ~Application();

    void run();
    /**
     * @brief Initialize the Application
     * @return false if there are any problems with the initialization
     * @return true otherwise
     */
    static bool init();
    static void quit();

private:
    void clear_frame_state();
    void process_events();
    void dismiss_overlay_if_game_over();

    Window m_window;
    InputState m_input;
    std::string m_font_path;

    AudioManager m_audio;
    std::unique_ptr<scenes::SceneManager> m_scene_manager;
    std::unique_ptr<renderer::Renderer2D> m_renderer;
};

} // namespace core
