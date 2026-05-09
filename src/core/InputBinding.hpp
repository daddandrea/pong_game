#pragma once

#include "PlayerInput.hpp"
#include "InputSettings.hpp"
#include <array>
#include <string>
#include <SDL3/SDL.h>
#include <variant>
#include <vector>

namespace core {

enum class AxisDir { Up, Down };

struct AxisBinding {
    SDL_GamepadAxis axis;
    AxisDir         dir;

    bool operator==(const AxisBinding&) const = default;
};

using ActionBinding = std::variant<SDL_Keycode, SDL_GamepadButton, AxisBinding>;

class InputBinding {
public:
    static InputBinding player1_defaults();
    static InputBinding player2_defaults();

    static InputBinding player1_kb_defaults();
    static InputBinding player2_kb_defaults();

    static InputBinding player1_gp_defaults();
    static InputBinding player2_gp_defaults();

    bool empty() const;
    void merge(const InputBinding& other);

    void bind(PlayerAction action, SDL_Keycode key);
    void bind(PlayerAction action, SDL_GamepadButton button);
    void bind(PlayerAction action, AxisBinding axis);

    void unbind(PlayerAction action, SDL_Keycode key);
    void unbind(PlayerAction action, SDL_GamepadButton button);
    void unbind(PlayerAction action, AxisBinding axis);

    void clear(PlayerAction action);

    PlayerInput resolve(SDL_Gamepad* gamepad, const InputSettings& settings);

    void save(const std::string& path, int player_index) const;
    static InputBinding load(const std::string& path, int player_index);

private:
    enum class ButtonState {
        Up,
        Down
    };

    float axis_float  (PlayerAction action, SDL_Gamepad* gamepad, float deadzone, float axis_scale) const;
    bool  held        (PlayerAction action, const bool* keys, SDL_Gamepad* gamepad, float deadzone, float axis_scale) const;
    bool  just_pressed(PlayerAction action, const bool* keys, SDL_Gamepad* gamepad, float deadzone, float axis_scale);

    std::array<std::vector<ActionBinding>, static_cast<size_t>(PlayerAction::COUNT)> m_bindings;
    std::array<ButtonState,                static_cast<size_t>(PlayerAction::COUNT)> m_state;
};

}
