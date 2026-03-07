#include "CpuController.hpp"
#include "game/BallState.hpp"
#include "game/Math.hpp"
#include "game/PaddleState.hpp"

namespace game {

void CpuController::update(PaddleState& paddle, const BallState& ball, float dt) const {
    const float dead_zone   = 0.1f;
    const float diff        = ball.pos.y - paddle.pos.y;

    const bool move_up      = diff > dead_zone;
    const bool move_down    = diff < -dead_zone;

    const float saved_speed = PADDLE_MOVE_SPEED;
    (void)saved_speed;

    const float dy = (move_up ? 1.0f : 0.0f) - (move_down ? 1.0f : 0.0f);
    paddle.pos.y += dy * CPU_SPEED * dt;

    const float max_y = ARENA_TOP    - PADDLE_HALF_H - ARENA_WALL_THICKNESS;
    const float min_y = ARENA_BOTTOM + PADDLE_HALF_H + ARENA_WALL_THICKNESS;

    if (paddle.pos.y > max_y) paddle.pos.y = max_y;
    if (paddle.pos.y < min_y) paddle.pos.y = min_y;

    if (paddle.parry_timer > 0.0f) paddle.parry_timer -= dt;
}

}
