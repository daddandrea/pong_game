#include "CpuController.hpp"
#include "game/BallState.hpp"
#include "game/GameSettings.hpp"
#include "game/Math.hpp"
#include "game/PaddleState.hpp"

#include <algorithm>

namespace game {

void CpuController::update(PaddleState& paddle, const BallState& ball, float dt, const GameSettings& settings) const {
    const float diff = ball.pos.y - paddle.pos.y;
    const float step = settings.cpu_speed * dt;
    paddle.pos.y += std::clamp(diff, -step, step);

    const float max_y = ARENA_TOP    - settings.paddle_half_h - ARENA_WALL_THICKNESS;
    const float min_y = ARENA_BOTTOM + settings.paddle_half_h + ARENA_WALL_THICKNESS;

    if (paddle.pos.y > max_y) paddle.pos.y = max_y;
    if (paddle.pos.y < min_y) paddle.pos.y = min_y;

    if (paddle.parry_timer > 0.0f) paddle.parry_timer -= dt;
}

}
