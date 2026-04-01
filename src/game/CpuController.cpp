#include "CpuController.hpp"
#include "game/BallState.hpp"
#include "game/Math.hpp"
#include "game/DevSettings.hpp"
#include "game/PaddleState.hpp"

namespace game {

void CpuController::update(PaddleState& paddle, const BallState& ball, float dt) const {
    const float dead_zone   = 0.1f;
    const float diff        = ball.pos.y - paddle.pos.y;

    const bool move_up      = diff > dead_zone;
    const bool move_down    = diff < -dead_zone;

    const float dy = (move_up ? 1.0f : 0.0f) - (move_down ? 1.0f : 0.0f);
    paddle.pos.y += dy * g_dev.cpu_speed * dt;

    const float max_y = ARENA_TOP    - g_dev.paddle_half_h - ARENA_WALL_THICKNESS;
    const float min_y = ARENA_BOTTOM + g_dev.paddle_half_h + ARENA_WALL_THICKNESS;

    if (paddle.pos.y > max_y) paddle.pos.y = max_y;
    if (paddle.pos.y < min_y) paddle.pos.y = min_y;

    if (paddle.parry_timer > 0.0f) paddle.parry_timer -= dt;
}

}
