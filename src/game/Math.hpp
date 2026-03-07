#pragma once

#include <glm/glm.hpp>

namespace game {

inline constexpr float     ARENA_HALF_W         = 10.0f;
inline constexpr float     ARENA_HALF_H         = 6.0f;
inline constexpr float     ARENA_LEFT           = -ARENA_HALF_W;
inline constexpr float     ARENA_RIGHT          = ARENA_HALF_W;
inline constexpr float     ARENA_TOP            = ARENA_HALF_H;
inline constexpr float     ARENA_BOTTOM         = -ARENA_HALF_H;

inline constexpr float     BALL_RADIUS          = 0.22f;
inline constexpr glm::vec2 BALL_INITIAL_POS     = {0.0f, 0.0f};
static constexpr float     BALL_INITIAL_SPEED   = 0.0f;
inline constexpr float     BALL_BASE_SPEED      = 7.0f;
static constexpr float     BALL_INITIAL_HOR_DIR = 1.0f;  // -1 left | +1 right
static constexpr float     BALL_INITIAL_VER_DIR = 1.0f;  // -1 down | +1 up
inline constexpr float     BALL_SPEED_STEP      = 0.35f; // Added each paddle bounce
inline constexpr float     BALL_PARRY_MULT      = 1.8f;  // Speed multiplier on parry hit

inline constexpr float     ARENA_WALL_THICKNESS = 0.08f;

inline constexpr float     PADDLE_HALF_W        = 0.2f;
inline constexpr float     PADDLE_HALF_H        = 1.0f;
inline constexpr float     PADDLE_MOVE_SPEED    = 9.0f;
inline constexpr float     PADDLE_X             = 9.0f;  // Distance from center
inline constexpr float     PADDLE_PARRY_CD      = 0.8f;  // Second between parries (Parry countdown)

inline constexpr float     CPU_SPEED            = 5.5f;

inline constexpr float     WIN_SCORE            = 7;

using vec2 = glm::vec2;
using vec4 = glm::vec4;

}
