#include "PaddleState.hpp"
#include "game/Math.hpp"

#include <algorithm>

namespace game {

PaddleState::PaddleState(PaddleSide s) 
    : pos(s == PaddleSide::Left ? glm::vec2{-PADDLE_X, 0.0f} : glm::vec2{ PADDLE_X, 0.0f}),
      side(s) 
{}

void PaddleState::update(float dt, bool move_up, bool move_down) {
    if (parry_timer > 0.0f) parry_timer -= dt;

    float dy = 0.0f;
    if (move_up)   dy += PADDLE_MOVE_SPEED * dt;
    if (move_down) dy -= PADDLE_MOVE_SPEED * dt;

    pos.y += dy;

    const float max_y = ARENA_TOP    - PADDLE_HALF_H - ARENA_WALL_THICKNESS;
    const float min_y = ARENA_BOTTOM + PADDLE_HALF_H + ARENA_WALL_THICKNESS;
    pos.y = std::clamp(pos.y, min_y, max_y);
}

RectCollider PaddleState::collider() const {
    return {pos, {PADDLE_HALF_W, PADDLE_HALF_H}};
}


}
