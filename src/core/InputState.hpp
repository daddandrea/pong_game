#pragma once

#include <array>

namespace core {

enum class Key {
    W,
    S,
    Up,
    Down,
    Space,
    Enter,
    Escape,
    COUNT
};

struct MouseState {
    float x = 0.0f;
    float y = 0.0f;
    bool left_pressed = false;
    bool left_held = false;
    bool left_released = false;
};

struct InputState {
    static constexpr int KEY_COUNT = static_cast<int>(Key::COUNT);

    std::array<bool, KEY_COUNT> held{};
    std::array<bool, KEY_COUNT> pressed{};
    std::array<bool, KEY_COUNT> released{};
    MouseState mouse;

    bool is_held    (Key k) const { return held    [static_cast<int>(k)]; }
    bool is_pressed (Key k) const { return pressed [static_cast<int>(k)]; }
    bool is_released(Key k) const { return released[static_cast<int>(k)]; }
};

}
