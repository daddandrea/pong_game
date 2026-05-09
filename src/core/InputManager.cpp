#include "InputManager.hpp"
#include "SDL3/SDL_events.h"
#include "SDL3/SDL_filesystem.h"
#include "SDL3/SDL_gamepad.h"
#include "SDL3/SDL_mouse.h"
#include "core/InputBinding.hpp"
#include "core/Logger.hpp"
#include "game/Math.hpp"
#include <format>
#include <string>

namespace core {

static std::string pref_path(int player) {
    char* raw = SDL_GetPrefPath("pong", "pong");
    if (!raw) return std::format("p{}_bindings.cfg", player + 1);
    std::string path = std::format("{}p{}_bindings.cfg", raw, player + 1);
    SDL_free(raw);

    return path;
}

static float px_to_world_x(float px, float win_w) {
    return (px / win_w) * game::ARENA_W - (game::ARENA_W * 0.5f);
}

static float px_to_world_y(float py, float win_h) {
    return (game::ARENA_H * 0.5f) - (py / win_h) * game::ARENA_H;
}

InputManager::InputManager() {
    m_bindings[0] = InputBinding::load(pref_path(0), 1);

    if (m_bindings[0].empty()) {
        m_bindings[0] = InputBinding::player1_defaults();
    }

    m_bindings[1] = InputBinding::load(pref_path(1), 2);

    if (m_bindings[1].empty()) {
        m_bindings[1] = InputBinding::player2_defaults();
    }
}

InputManager::~InputManager() {
    for (auto* gp : m_gamepads) {
        if (gp) SDL_CloseGamepad(gp);
    }
}

void InputManager::set_window_size(float w, float h) {
    m_win_w = w;
    m_win_h = h;
}

void InputManager::clear_frame_state() {
    m_mouse.left_pressed  = false;
    m_mouse.left_released = false;
    m_mouse.moved         = false;
}

void InputManager::on_gamepad_added(SDL_JoystickID id) {
    for (size_t i = 0; i < 2; ++i) {
        if (!m_gamepads[i]) {
            SDL_Gamepad* gp = SDL_OpenGamepad(id);
            if (!gp) return;
            m_gamepads[i]    = gp;
            m_gamepad_ids[i] = id;
            Log::info("Gamepad connected: {} (player {})", SDL_GetGamepadName(gp), i + 1);
            return;
        }
    }
}

void InputManager::on_gamepad_removed(SDL_JoystickID id) {
    for (size_t i = 0; i < 2; ++i) {
        if (m_gamepad_ids[i] == id) {
            Log::info("Gamepad disconnected (player {})", i + 1);
            SDL_CloseGamepad(m_gamepads[i]);
            m_gamepads[i]    = nullptr;
            m_gamepad_ids[i] = 0;
            return;
        }
    }
}

void InputManager::handle_event(const SDL_Event& event) {
    switch (event.type) {
        case SDL_EVENT_GAMEPAD_ADDED:
            on_gamepad_added(event.gdevice.which);
            break;

        case SDL_EVENT_GAMEPAD_REMOVED:
            on_gamepad_removed(event.gdevice.which);
            break;

        case SDL_EVENT_MOUSE_MOTION:
            m_mouse.x     = px_to_world_x(event.motion.x, m_win_w);
            m_mouse.y     = px_to_world_y(event.motion.y, m_win_h);
            m_mouse.moved = true;
            break;

        case SDL_EVENT_MOUSE_BUTTON_DOWN:
            if (event.button.button == SDL_BUTTON_LEFT) {
                m_mouse.x            = px_to_world_x(event.button.x, m_win_w);
                m_mouse.y            = px_to_world_y(event.button.y, m_win_h);
                m_mouse.left_held    = true;
                m_mouse.left_pressed = true;
            }
            break;

        case SDL_EVENT_MOUSE_BUTTON_UP:
            if (event.button.button == SDL_BUTTON_LEFT) {
                m_mouse.left_held     = false;
                m_mouse.left_released = true;
            }
            break;

        default:
            break;
    }
}

std::array<PlayerInput, 2> InputManager::poll() {
    return {
        m_bindings[0].resolve(m_gamepads[0], m_input_settings),
        m_bindings[1].resolve(m_gamepads[1], m_input_settings)
    };
}

FrameInput InputManager::frame_input() {
    return { poll(), m_mouse };
}

InputBinding& InputManager::binding(int player) {
    return m_bindings[player];
}

void InputManager::save_bindings() const {
    m_bindings[0].save(pref_path(0), 1);
    m_bindings[1].save(pref_path(1), 2);
}

}
