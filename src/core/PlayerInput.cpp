#include "PlayerInput.hpp"

namespace core {

std::optional<std::string_view> action_to_string(PlayerAction action) {
    using enum PlayerAction;

    switch (action) {
        case Up:      return UP;
        case Down:    return DOWN;
        case Parry:   return PARRY;
        case Confirm: return CONFIRM;
        case Back:    return BACK;
        default:      return std::nullopt;
    }
}

std::optional<PlayerAction> string_to_action(std::string_view name) {
    using enum PlayerAction;

    if      (name == UP     ) return Up;
    else if (name == DOWN   ) return Down;
    else if (name == PARRY  ) return Parry;
    else if (name == CONFIRM) return Confirm;
    else if (name == BACK   ) return Back;

    return std::nullopt;
}

}
