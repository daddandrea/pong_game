#pragma once

#include "Math.hpp"
#include "game/Collider.hpp"
#include <glm/glm.hpp>

namespace game {

enum class PaddleSide { Left, Right };

struct PaddleState {
    glm::vec2  pos;
    PaddleSide side;
    float      parry_timer = 0.0f;

    explicit PaddleState(PaddleSide s);

    void update(float dt, bool move_up, bool move_down);

    bool parry_on_cooldown() const { return parry_timer > 0.0f; };

    void trigger_parry() { parry_timer = PADDLE_PARRY_CD; };

    RectCollider collider() const;
};

}
