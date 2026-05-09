#pragma once

#include "core/InputState.hpp"
#include "core/PlayerInput.hpp"
#include <array>

namespace core {

struct FrameInput {
    std::array<PlayerInput, 2> players;
    MouseState mouse;
};

}
