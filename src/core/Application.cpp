#include "Application.hpp"

#include "core/Exceptions.hpp"
#include "core/Logger.hpp"
#include "scenes/CreditsScene.hpp"
#include "scenes/DevMenu.hpp"
#include "scenes/GameScene.hpp"
#include "scenes/IScene.hpp"
#include "scenes/MainMenuScene.hpp"
#include "scenes/MultiPlayerMenuScene.hpp"
#include "scenes/PauseScene.hpp"
#include "renderer/Renderer2D.hpp"
#include "scenes/SinglePlayerMenuScene.hpp"

#include <glad/glad.h>
#include <imgui.h>
#include <imgui_impl_opengl3.h>
#include <imgui_impl_sdl3.h>
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

namespace core {

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
    using enum core::Key;

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
    case SDLK_F1:
        return static_cast<int>(Key::F1);
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

    throw core::FontNotFoundException(
        std::format("{}\n{}",
                    "No usable font found",
                    "Place LiberationMono-Regular.ttf in assets/fonts/ next to the executable.")
    );
}

#ifndef PONG_VERSION
#define PONG_VERSION "0.0.0"
#endif

#ifdef PONG_DEV
static constexpr std::string_view BUILD_MODE = "Debug";
#else
static constexpr std::string_view BUILD_MODE = "Release";
#endif

Application::Application() {
    Log::info("Pong v{}", PONG_VERSION);

    m_renderer = std::make_unique<renderer::Renderer2D>();

    m_font_path = find_font();
    Log::info("Loading font: {}", m_font_path);

    int fb_w;
    int fb_h;

    SDL_GetWindowSizeInPixels(m_window.get_sdl_window(), &fb_w, &fb_h);
    m_renderer->load_font(m_font_path, 3.0f * static_cast<float>(fb_h) / WORLD_H);

    m_scene_manager = std::make_unique<scenes::SceneManager>();

    m_scene_manager->register_scene(Transition::MainMenu,
                                    [&c = m_config] { return std::make_unique<scenes::MainMenuScene>(c); }
    );

    m_scene_manager->register_scene(Transition::SinglePlayer,
                                    [&c = m_config] { return std::make_unique<scenes::SinglePlayerMenuScene>(c); }
    );

    m_scene_manager->register_scene(Transition::MultiPlayer,
                                    [&c = m_config] { return std::make_unique<scenes::MultiPlayerMenuScene>(c); }
    );

    m_scene_manager->register_scene(Transition::Game,
                                    [&s = m_settings, &c = m_config, &d = m_dev ]
                                        { return std::make_unique<scenes::GameScene>(s, c, d); }
    );

    m_scene_manager->register_scene(Transition::Pause,
                                    [&d = m_dev] { return std::make_unique<scenes::PauseScene>(d); }
    );

    m_scene_manager->register_scene(Transition::Credits,
                                    [] { return std::make_unique<scenes::CreditsScene>(); }
    );

    m_scene_manager->push(Transition::MainMenu);

    m_updater.check_async();

    IMGUI_CHECKVERSION();
    ImGui::CreateContext();
    ImGui::StyleColorsDark();
    ImGui_ImplSDL3_InitForOpenGL(m_window.get_sdl_window(), m_window.get_gl_context());
    ImGui_ImplOpenGL3_Init("#version 330 core");
}

bool Application::init() {
    if (!SDL_Init(SDL_INIT_VIDEO | SDL_INIT_AUDIO)) {
        Log::error("SDL_Init failed: {}", SDL_GetError());
        return false;
    }
    Log::init(PONG_VERSION, BUILD_MODE);
    if (!TTF_Init()) {
        Log::error("TTF_Init failed: {}\n", SDL_GetError());
        return false;
    }
    return true;
}

Application::~Application() {
    ImGui_ImplOpenGL3_Shutdown();
    ImGui_ImplSDL3_Shutdown();
    ImGui::DestroyContext();
}

static void draw_update_status(Updater& updater) {
    const auto update_status = updater.status();
    ImGui::Spacing();

    if (update_status == Updater::Status::Downloading) {
        ImGui::Text("Downloading version %s...", updater.latest_version().c_str());
    } else if (update_status == Updater::Status::InstallFailed) {
        ImGui::TextColored({ 1.f, 0.3f, 0.3f, 1.f }, "Update failed. Please try again later.");
        ImGui::Spacing();
        if (ImGui::Button("Close", { 120.f, 0.f })) {
            updater.dismiss();
            ImGui::CloseCurrentPopup();
        }
    } else {
        ImGui::Text("Version %s is available. Update now?", updater.latest_version().c_str());
        ImGui::Spacing();
        if (ImGui::Button("Yes",   { 120.f, 0.f })) updater.download_and_install();
        ImGui::SameLine();
        if (ImGui::Button("Later", { 120.f, 0.f })) {
            updater.dismiss();
            ImGui::CloseCurrentPopup();
        }
    }

    ImGui::Spacing();
    ImGui::EndPopup();
}

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
            return;

        ImGui_ImplOpenGL3_NewFrame();
        ImGui_ImplSDL3_NewFrame();
        ImGui::NewFrame();

        const auto updater_status = m_updater.status();
        const bool block_input = (updater_status == Updater::Status::Checking ||
                                  updater_status == Updater::Status::UpdateAvailable);
        const core::InputState scene_input = block_input ? core::InputState{} : m_input;

        if (const std::string next = m_scene_manager->update(scene_input, dt); next == Transition::Quit) {
            m_window.set_should_close(true);
            ImGui::EndFrame();
            return;
        }

        glClearColor(0.04f, 0.04f, 0.04f, 1.0f);
        glClear(GL_COLOR_BUFFER_BIT);

        m_scene_manager->render(*m_renderer);

        if (const auto update_status = m_updater.status();
            update_status == Updater::Status::UpdateAvailable  ||
            update_status == Updater::Status::Downloading      ||
            update_status == Updater::Status::InstallFailed) {

            ImGui::OpenPopup("Update Available");
        }

        ImGui::SetNextWindowPos(ImGui::GetMainViewport()->GetCenter(), ImGuiCond_Always, { 0.5f, 0.5f });
        ImGui::SetNextWindowSize({ 480.f, 0.f }, ImGuiCond_Always);

        if (ImGui::BeginPopupModal("Update Available", nullptr,
                ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoMove | ImGuiWindowFlags_NoCollapse)) {
            draw_update_status(m_updater);
        }

        #ifdef PONG_DEV
        scenes::render_dev_menu(m_scene_manager->find_game_state(), m_settings, m_dev);
        #endif

        dismiss_overlay_if_game_over();

        ImGui::Render();
        ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());

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
    const auto win_w = static_cast<float>(m_window.get_width());
    const auto win_h = static_cast<float>(m_window.get_height());

    SDL_Event event;
    while (SDL_PollEvent(&event)) {
        ImGui_ImplSDL3_ProcessEvent(&event);
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
                if (const int idx = sdl_key_to_index(event.key.key); idx != -1) {
                    m_input.held[idx] = true;
                    m_input.pressed[idx] = true;
                    m_input.released[idx] = false;
                }
                break;
            }

            case SDL_EVENT_KEY_UP: {
                if (const int idx = sdl_key_to_index(event.key.key); idx != -1) {
                    m_input.held[idx] = false;
                    m_input.pressed[idx] = false;
                    m_input.released[idx] = true;
                }
                break;
            }

            case SDL_EVENT_MOUSE_MOTION:
                if (!ImGui::GetIO().WantCaptureMouse) {
                    m_input.mouse.x = px_to_world_x(event.motion.x, win_w);
                    m_input.mouse.y = py_to_world_y(event.motion.y, win_h);
                }
                break;

            case SDL_EVENT_MOUSE_BUTTON_DOWN:
                if (event.button.button == SDL_BUTTON_LEFT
                    && !ImGui::GetIO().WantCaptureMouse) {
                    m_input.mouse.x = px_to_world_x(event.motion.x, win_w);
                    m_input.mouse.y = py_to_world_y(event.motion.y, win_h);
                    m_input.mouse.left_held = true;
                    m_input.mouse.left_pressed = true;
                    m_input.mouse.left_released = false;
                }
                break;

            case SDL_EVENT_MOUSE_BUTTON_UP:
                if (event.button.button == SDL_BUTTON_LEFT
                    && !ImGui::GetIO().WantCaptureMouse) {
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

void Application::dismiss_overlay_if_game_over() {
    const game::GameState* state = m_scene_manager->find_game_state();
    if (state && state->phase == game::GamePhase::GameOver
        && m_scene_manager->top()->get_game_state() == nullptr)
        m_scene_manager->pop();
}

void Application::quit() { TTF_Quit(); SDL_Quit(); }

} // namespace core
