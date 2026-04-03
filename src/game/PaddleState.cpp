#include "PaddleState.hpp"
#include "game/Math.hpp"

#include <algorithm>

namespace game {

PaddleState::PaddleState(PaddleSide s)
    : pos(s == PaddleSide::Left ? glm::vec2{-PADDLE_X, 0.0f} : glm::vec2{ PADDLE_X, 0.0f}),
      side(s)
{}

void PaddleState::update(float dt, bool move_up, bool move_down, const GameSettings& settings) {
    if (parry_timer > 0.0f) parry_timer -= dt;

    float dy = 0.0f;
    if (move_up)   dy += settings.paddle_move_speed * dt;
    if (move_down) dy -= settings.paddle_move_speed * dt;

    pos.y += dy;

    const float max_y = ARENA_TOP    - settings.paddle_half_h - ARENA_WALL_THICKNESS;
    const float min_y = ARENA_BOTTOM + settings.paddle_half_h + ARENA_WALL_THICKNESS;
    pos.y = std::clamp(pos.y, min_y, max_y);
}

RectCollider PaddleState::collider(const GameSettings& settings) const {
    return {pos, {PADDLE_HALF_W, settings.paddle_half_h}};
}

void PaddleState::trigger_parry(const GameSettings& settings) { parry_timer = settings.paddle_parry_cd; }

}
