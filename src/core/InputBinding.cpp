#include "InputBinding.hpp"
#include "SDL3/SDL_gamepad.h"
#include "SDL3/SDL_joystick.h"
#include "SDL3/SDL_keyboard.h"
#include "SDL3/SDL_keycode.h"
#include "core/PlayerInput.hpp"
#include <algorithm>
#include <cstddef>
#include <fstream>
#include <sstream>
#include <string>
#include <format>

namespace core {

static size_t idx(PlayerAction a) {
    return static_cast<size_t>(a);
}

InputBinding InputBinding::player1_defaults() {
    InputBinding b = player1_kb_defaults();
    b.merge(player1_gp_defaults());
    return b;
}

InputBinding InputBinding::player2_defaults() {
    InputBinding b = player2_kb_defaults();
    b.merge(player2_gp_defaults());
    return b;
}

InputBinding InputBinding::player1_kb_defaults() {
    using enum PlayerAction;

    InputBinding b;
    b.bind(Up,      SDLK_W);
    b.bind(Down,    SDLK_S);
    b.bind(Parry,   SDLK_SPACE);
    b.bind(Confirm, SDLK_SPACE);
    b.bind(Back,    SDLK_ESCAPE);
    return b;
}

InputBinding InputBinding::player2_kb_defaults() {
    using enum PlayerAction;

    InputBinding b;
    b.bind(Up,      SDLK_UP);
    b.bind(Down,    SDLK_DOWN);
    b.bind(Parry,   SDLK_RETURN);
    b.bind(Confirm, SDLK_RETURN);
    b.bind(Back,    SDLK_ESCAPE);
    return b;
}

InputBinding InputBinding::player1_gp_defaults() {
    using enum PlayerAction;

    InputBinding b;
    b.bind(Up,      SDL_GAMEPAD_BUTTON_DPAD_UP);
    b.bind(Up,      AxisBinding{SDL_GAMEPAD_AXIS_LEFTY, AxisDir::Up});
    b.bind(Down,    SDL_GAMEPAD_BUTTON_DPAD_DOWN);
    b.bind(Down,    AxisBinding{SDL_GAMEPAD_AXIS_LEFTY, AxisDir::Down});
    b.bind(Parry,   SDL_GAMEPAD_BUTTON_SOUTH);
    b.bind(Confirm, SDL_GAMEPAD_BUTTON_SOUTH);
    b.bind(Back,    SDL_GAMEPAD_BUTTON_EAST);
    return b;
}

InputBinding InputBinding::player2_gp_defaults() {
    using enum PlayerAction;

    InputBinding b;
    b.bind(Up,      SDL_GAMEPAD_BUTTON_DPAD_UP);
    b.bind(Up,      AxisBinding{SDL_GAMEPAD_AXIS_LEFTY, AxisDir::Up});
    b.bind(Down,    SDL_GAMEPAD_BUTTON_DPAD_DOWN);
    b.bind(Down,    AxisBinding{SDL_GAMEPAD_AXIS_LEFTY, AxisDir::Down});
    b.bind(Parry,   SDL_GAMEPAD_BUTTON_SOUTH);
    b.bind(Confirm, SDL_GAMEPAD_BUTTON_SOUTH);
    b.bind(Back,    SDL_GAMEPAD_BUTTON_EAST);
    return b;
}

void InputBinding::merge(const InputBinding& other) {
    for (size_t i = 0; i < m_bindings.size(); ++i) {
        for (const auto& b : other.m_bindings[i]) {
            if (std::ranges::find(m_bindings[i], b) == m_bindings[i].end()) {
                m_bindings[i].push_back(b);
            }
        }
    }
}

bool InputBinding::empty() const {
    return std::ranges::all_of(
        m_bindings,
        [](const auto& slot) {
            return slot.empty();
        }
    );
}

void InputBinding::bind(PlayerAction action, SDL_Keycode key) {
    auto& bindings = m_bindings[idx(action)];
    ActionBinding entry = key;

    if (std::ranges::find(bindings, entry) == bindings.end()) {
        bindings.push_back(entry);
    }
}

void InputBinding::bind(PlayerAction action, SDL_GamepadButton button) {
    auto& bindings = m_bindings[idx(action)];
    ActionBinding entry = button;

    if (std::ranges::find(bindings, entry) == bindings.end()) {
        bindings.push_back(entry);
    }
}

void InputBinding::bind(PlayerAction action, AxisBinding axis) {
    auto& bindings = m_bindings[idx(action)];
    ActionBinding entry = axis;

    if (std::ranges::find(bindings, entry) == bindings.end()) {
        bindings.push_back(entry);
    }
}

void InputBinding::unbind(PlayerAction action, SDL_Keycode key) {
    std::erase_if(
        m_bindings[idx(action)],

        [key](const ActionBinding& b) {
            const auto* k = std::get_if<SDL_Keycode>(&b);
            return k && *k == key;
        }
    );
}

void InputBinding::unbind(PlayerAction action, SDL_GamepadButton button) {
    std::erase_if(
        m_bindings[idx(action)],

        [button](const ActionBinding& b) {
            const auto* btn = std::get_if<SDL_GamepadButton>(&b);
            return btn && *btn == button;
        }
    );
}

void InputBinding::unbind(PlayerAction action, AxisBinding axis) {
    std::erase_if(
        m_bindings[idx(action)],
        [axis](const ActionBinding& b) {
            const auto* a = std::get_if<AxisBinding>(&b);
            return a && *a == axis;
        }
    );
}

void InputBinding::clear(PlayerAction action) {
    m_bindings[idx(action)].clear();
}

float InputBinding::axis_float(PlayerAction action, SDL_Gamepad* gamepad, float deadzone, float axis_scale) const {
    for (const auto& b : m_bindings[idx(action)]) {
        if (const auto* a = std::get_if<AxisBinding>(&b); a && gamepad) {
            const float v = SDL_GetGamepadAxis(gamepad, a->axis) * axis_scale;
            if (a->dir == AxisDir::Down && v >  deadzone) return  v;
            if (a->dir == AxisDir::Up   && v < -deadzone) return -v;
        }
    }
    return 0.0f;
}

bool InputBinding::held(PlayerAction action, const bool* keys, SDL_Gamepad* gamepad, float deadzone, float axis_scale) const {
    for (const auto& b : m_bindings[idx(action)]) {

        if (const auto* key = std::get_if<SDL_Keycode>(&b);
            key && keys[SDL_GetScancodeFromKey(*key, nullptr)]) {
                return true;
        }

        if (const auto* btn = std::get_if<SDL_GamepadButton>(&b);
            btn && gamepad && SDL_GetGamepadButton(gamepad, *btn)) {
                return true;
        }

        if (const auto* a = std::get_if<AxisBinding>(&b); a && gamepad) {
            const float v = SDL_GetGamepadAxis(gamepad, a->axis) * axis_scale;
            if (a->dir == AxisDir::Down && v >  deadzone) return true;
            if (a->dir == AxisDir::Up   && v < -deadzone) return true;
        }
    }
    return false;
}

bool InputBinding::just_pressed(PlayerAction action, const bool* keys, SDL_Gamepad* gamepad, float deadzone, float axis_scale) {
    using enum ButtonState;

    const bool active    = held(action, keys, gamepad, deadzone, axis_scale);
    const bool fired     = active && m_state[idx(action)] == Up;

    m_state[idx(action)] = active
                               ? Down
                               : Up;

    return fired;
}

PlayerInput InputBinding::resolve(SDL_Gamepad* gamepad, const InputSettings& settings) {
    using enum PlayerAction;

    const bool* keys       = SDL_GetKeyboardState(nullptr);
    const float deadzone   = settings.axis_deadzone;
    const float axis_scale = 1.0f / static_cast<float>(SDL_JOYSTICK_AXIS_MAX);

    const bool  up   = held(Up, keys, gamepad, deadzone, axis_scale);
    const bool  down = held(Down,keys, gamepad, deadzone, axis_scale);
    const float up_f = axis_float(Up, gamepad, deadzone, axis_scale);
    const float dn_f = axis_float(Down, gamepad, deadzone, axis_scale);

    float move = dn_f - up_f;
    if (move == 0.0f) {
        if (up)   move = -1.0f;
        if (down) move =  1.0f;
    }

    return {
        .up       = up,
        .down     = down,
        .nav_up   = just_pressed(PlayerAction::Up,      keys, gamepad, deadzone, axis_scale),
        .nav_down = just_pressed(PlayerAction::Down,    keys, gamepad, deadzone, axis_scale),
        .parry    = just_pressed(PlayerAction::Parry,   keys, gamepad, deadzone, axis_scale),
        .confirm  = just_pressed(PlayerAction::Confirm, keys, gamepad, deadzone, axis_scale),
        .back     = just_pressed(PlayerAction::Back,    keys, gamepad, deadzone, axis_scale),
        .move     = move,
    };
}

void InputBinding::save(const std::string& path, int player_index) const {
    std::ofstream file(path);

    for (size_t i = 0; i < static_cast<size_t>(PlayerAction::COUNT); ++i) {
        const auto action = static_cast<PlayerAction>(i);
        const auto name   = action_to_string(action);

        if (!name) continue;

        file << "p" << player_index << "." << name.value() << "=";

        bool first = true;

        for (const auto& b : m_bindings[i]) {

            if (!first) file << ",";

            std::visit(
                [&file](const auto& binding) {
                    using T = std::decay_t<decltype(binding)>;
                    if constexpr (std::is_same_v<T, SDL_Keycode>) {
                        file << "key:" << SDL_GetKeyName(binding);
                    }
                    else if constexpr (std::is_same_v<T, SDL_GamepadButton>) {
                        file << "button:" << SDL_GetGamepadStringForButton(binding);
                    }
                    else if constexpr (std::is_same_v<T, AxisBinding>) {
                        file << "axis:" << SDL_GetGamepadStringForAxis(binding.axis) << (binding.dir == AxisDir::Down ? "+" : "-");
                    }
                },
                b
            );

            first = false;

        }

        file << "\n";

    }
}

static void bind_action_from_file(InputBinding& b, std::string_view type, PlayerAction action, std::string_view value) {
    if (type == "key") {
        const SDL_Keycode key = SDL_GetKeyFromName(std::string(value).c_str());
        if (key != SDLK_UNKNOWN) 
            b.bind(action, key);

    } else if (type == "button") {
        const SDL_GamepadButton btn = SDL_GetGamepadButtonFromString(std::string(value).c_str());
        if (btn != SDL_GAMEPAD_BUTTON_INVALID)
            b.bind(action, btn);

    } else if (type == "axis") {
        if (value.empty()) return;

        const AxisDir dir        = (value.back() == '+') ? AxisDir::Down : AxisDir::Up;
        const auto axis_name     = std::string(value.substr(0, value.size() - 1));
        const SDL_GamepadAxis ax = SDL_GetGamepadAxisFromString(axis_name.c_str());

        if (ax != SDL_GAMEPAD_AXIS_INVALID)
            b.bind(action, AxisBinding{ax, dir});
    }
}

InputBinding InputBinding::load(const std::string& path, int player_index) {
    std::ifstream file(path);
    if (!file.is_open()) return {};

    InputBinding result;
    std::string line;

    while (std::getline(file, line)) {
        const auto eq = line.find('=');
        if (eq == std::string::npos) continue;

        const std::string left  = line.substr(0, eq);
        const std::string right = line.substr(eq + 1);

        const auto dot = left.find('.');
        if (dot == std::string::npos) continue;

        if (left.substr(0, dot) != std::format("p{}", player_index)) continue;

        const auto action = string_to_action(std::string_view(left).substr(dot + 1));

        if (!action) continue;

        std::stringstream ss(right);
        std::string token;
        while (std::getline(ss, token, ',')) {
            const auto colon = token.find(':');
            if (colon == std::string::npos) continue;

            const std::string type  = token.substr(0, colon);
            const std::string value = token.substr(colon + 1);

            bind_action_from_file(result, std::string_view(type), *action, std::string_view(value));
        }
    }

    return result;
}

}
