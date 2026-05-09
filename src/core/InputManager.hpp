#pragma once

#include "SDL3/SDL_events.h"
#include "SDL3/SDL_joystick.h"
#include "core/FrameInput.hpp"
#include "core/InputBinding.hpp"
#include "core/InputSettings.hpp"
#include <array>

namespace core {

class InputManager {

public:
    InputManager();
    ~InputManager();

    void set_window_size(float w, float h);
    void clear_frame_state();
    void handle_event(const SDL_Event& event);
    FrameInput frame_input();

    InputBinding& binding(int player);
    void save_bindings() const;

private:
    void on_gamepad_added  (SDL_JoystickID id);
    void on_gamepad_removed(SDL_JoystickID id);

    std::array<PlayerInput, 2> poll();

    std::array<InputBinding, 2>    m_bindings;
    std::array<SDL_Gamepad*, 2>    m_gamepads    = {nullptr, nullptr};
    std::array<SDL_JoystickID, 2>  m_gamepad_ids = {0, 0};
    InputSettings                  m_input_settings;
    MouseState                     m_mouse;
    float                          m_win_w = 1.0f;
    float                          m_win_h = 1.0f;
};
}
