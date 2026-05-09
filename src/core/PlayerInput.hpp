#pragma once

#include <optional>
#include <string_view>

namespace core {

enum class PlayerAction {
    Up,
    Down,
    Parry,
    Confirm,
    Back,
    COUNT
};

constexpr std::string_view UP       = "Up";
constexpr std::string_view DOWN     = "Down";
constexpr std::string_view PARRY    = "Parry";
constexpr std::string_view CONFIRM  = "Confirm";
constexpr std::string_view BACK     = "Back";

std::optional<std::string_view>  action_to_string(PlayerAction action);
std::optional<PlayerAction>      string_to_action(std::string_view name);

struct PlayerInput {
    bool  up;
    bool  down;
    bool  nav_up;
    bool  nav_down;
    bool  parry;
    bool  confirm;
    bool  back;
    float move; // -1.0 to 1.0
};

}
