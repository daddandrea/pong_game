#pragma once

#include "core/AudioManager.hpp"
#include "core/InputState.hpp"
#include "core/Updater.hpp"
#include "core/Window.hpp"
#include "game/DevSettings.hpp"
#include "game/GameConfig.hpp"
#include "game/GameSettings.hpp"
#include "scenes/SceneManager.hpp"

#include <memory>

namespace scenes   { class SceneManager; }
namespace renderer { class Renderer; }

namespace core {

static constexpr float WORLD_W = 20.0f;
static constexpr float WORLD_H = 12.0f;

static constexpr int WIN_W = 1280;
static constexpr int WIN_H = 720;

static constexpr const char* FONT_PATH = "assets/fonts/LiberationMono-Regular.ttf";

static constexpr float MAX_DT = 0.05f;

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

    Window m_window = Window("Pong", WIN_W, WIN_H);
    InputState m_input;
    std::string m_font_path;

    AudioManager m_audio;
    Updater      m_updater;
    std::unique_ptr<scenes::SceneManager> m_scene_manager;
    std::unique_ptr<renderer::Renderer2D> m_renderer;

    game::GameSettings m_settings;
    game::GameConfig   m_config;
    game::DevSettings  m_dev;
};

} // namespace core
