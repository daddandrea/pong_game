#include "Application.hpp"
#include "core/Logger.hpp"
#include "scenes/CreditsScene.hpp"
#include "scenes/GameScene.hpp"
#include "scenes/IScene.hpp"
#include "scenes/MainMenuScene.hpp"
#include "scenes/PauseScene.hpp"
#include "renderer/Renderer2D.hpp"

#include <glad/glad.h>
#include <SDL3/SDL.h>
#include <SDL3/SDL_events.h>
#include <SDL3/SDL_filesystem.h>
#include <SDL3/SDL_keycode.h>
#include <SDL3/SDL_mouse.h>
#include <SDL3_ttf/SDL_ttf.h>

#include <filesystem>
#include <memory>
#include <scenes/SceneManager.hpp>

#include <chrono>
#include <stdexcept>

namespace core {

static constexpr float WORLD_W = 20.0f;
static constexpr float WORLD_H = 12.0f;

static constexpr int WIN_W = 1280;
static constexpr int WIN_H = 768;

static constexpr const char* FONT_PATH = "assets/fonts/LiberationMono-Regular.ttf";

static constexpr float MAX_DT = 0.05f;

static float px_to_world_x(float px, float win_w) {
    return (px / win_w) * WORLD_W - (WORLD_W * 0.5f);
}

static float py_to_world_y(float py, float win_h) {
    return (WORLD_H * 0.5f) - (py / win_h) * WORLD_H;
}

/**
 * @brief Map an SDL key symbol to Key enum index
 * @param key SDL keycode
 * @return matching Key enum index
 * @return -1 if unmapped
 */
static int sdl_key_to_index(SDL_Keycode key) {
    switch (key) {
    case SDLK_W:
        return static_cast<int>(Key::W);
    case SDLK_S:
        return static_cast<int>(Key::S);
    case SDLK_UP:
        return static_cast<int>(Key::Up);
    case SDLK_DOWN:
        return static_cast<int>(Key::Down);
    case SDLK_SPACE:
        return static_cast<int>(Key::Space);
    case SDLK_RETURN:
        return static_cast<int>(Key::Enter);
    case SDLK_ESCAPE:
        return static_cast<int>(Key::Escape);
    default:
        return -1;
    }
}

static std::string find_font() {
    if (const char* base = SDL_GetBasePath()) {
        std::string bundled = std::format("{}{}", base, FONT_PATH);
        if (std::filesystem::exists(bundled))
            return bundled;
    }

    throw std::runtime_error(
        std::format("{}\n{}", "No usable font found",
                    "Place LiberationMono-Regular.ttf in assets/fonts/ next to the executable."));
}

Application::Application() : m_window("Pong", WIN_W, WIN_H) {

    m_renderer = std::make_unique<renderer::Renderer2D>();

    m_font_path = find_font();
    Log::info("Loading font: {}", m_font_path);
    int fb_w, fb_h;
    SDL_GetWindowSizeInPixels(m_window.get_sdl_window(), &fb_w, &fb_h);
    m_renderer->load_font(m_font_path, 3.0f * static_cast<float>(fb_h) / WORLD_H);

    m_scene_manager = std::make_unique<scenes::SceneManager>();
    m_scene_manager->register_scene(Transition::MainMenu,
                                    [] { return std::make_unique<scenes::MainMenuScene>(); });
    m_scene_manager->register_scene(Transition::Game,
                                    [] { return std::make_unique<scenes::GameScene>(); });
    m_scene_manager->register_scene(Transition::Pause,
                                    [] { return std::make_unique<scenes::PauseScene>(); });
    m_scene_manager->register_scene(Transition::Credits,
                                    [] { return std::make_unique<scenes::CreditsScene>(); });

    m_scene_manager->push(Transition::MainMenu);
}

bool Application::init() {
    if (!TTF_Init()) {
        Log::error("TTF_Init failed: {}\n", SDL_GetError());
        return false;
    }
    return true;
}

Application::~Application() = default;

void Application::run() {
    using clock = std::chrono::steady_clock;
    auto last_time = clock::now();

    while (!m_window.should_close()) {
        const auto now = clock::now();
        float dt = std::chrono::duration<float>(now - last_time).count();
        last_time = now;
        if (dt > MAX_DT)
            dt = MAX_DT;

        clear_frame_state();
        process_events();

        if (m_window.should_close())
            break;

        const std::string next = m_scene_manager->update(m_input, dt);
        if (next == Transition::Quit) {
            m_window.set_should_close(true);
            break;
        }

        glClearColor(0.04f, 0.04f, 0.04f, 1.0f);
        glClear(GL_COLOR_BUFFER_BIT);

        m_scene_manager->render(*m_renderer);

        m_window.swap_buffers();
    }
}

void Application::clear_frame_state() {
    m_input.pressed.fill(false);
    m_input.released.fill(false);
    m_input.mouse.left_pressed = false;
    m_input.mouse.left_released = false;
}

void Application::process_events() {
    const float win_w = static_cast<float>(m_window.get_width());
    const float win_h = static_cast<float>(m_window.get_height());

    SDL_Event event;
    while (SDL_PollEvent(&event)) {
        switch (event.type) {
            case SDL_EVENT_QUIT:
                m_window.set_should_close(true);
                break;

            case SDL_EVENT_WINDOW_PIXEL_SIZE_CHANGED: {
                const int w = event.window.data1;
                const int h = event.window.data2;
                glViewport(0, 0, w, h);
                m_renderer->load_font(m_font_path, 3.0f * static_cast<float>(h) / WORLD_H);
                break;
            }

            case SDL_EVENT_KEY_DOWN: {
                const int idx = sdl_key_to_index(event.key.key);
                if (idx != -1) {
                    m_input.held[idx] = true;
                    m_input.pressed[idx] = true;
                    m_input.released[idx] = false;
                }
                break;
            }

            case SDL_EVENT_KEY_UP: {
                const int idx = sdl_key_to_index(event.key.key);
                if (idx != -1) {
                    m_input.held[idx] = false;
                    m_input.pressed[idx] = false;
                    m_input.released[idx] = true;
                }
                break;
            }

            case SDL_EVENT_MOUSE_MOTION:
                m_input.mouse.x = px_to_world_x(event.motion.x, win_w);
                m_input.mouse.y = py_to_world_y(event.motion.y, win_h);
                break;

            case SDL_EVENT_MOUSE_BUTTON_DOWN:
                if (event.button.button == SDL_BUTTON_LEFT) {
                    m_input.mouse.x = px_to_world_x(event.motion.x, win_w);
                    m_input.mouse.y = py_to_world_y(event.motion.y, win_h);
                    m_input.mouse.left_held = true;
                    m_input.mouse.left_pressed = true;
                    m_input.mouse.left_released = false;
                }
                break;

            case SDL_EVENT_MOUSE_BUTTON_UP:
                if (event.button.button == SDL_BUTTON_LEFT) {
                    m_input.mouse.left_held = true;
                    m_input.mouse.left_pressed = false;
                    m_input.mouse.left_released = true;
                }
                break;

            default:
                break;
        }
    }
}

void Application::quit() { TTF_Quit(); }

} // namespace core
