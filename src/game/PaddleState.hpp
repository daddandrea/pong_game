#pragma once

#include "Math.hpp"
#include "game/Collider.hpp"
#include "game/GameSettings.hpp"
#include <glm/glm.hpp>

namespace game {

enum class PaddleSide { Left, Right };

struct PaddleState {
    glm::vec2  pos;
    PaddleSide side;
    float      parry_timer = 0.0f;

    explicit PaddleState(PaddleSide s);

    void update(float dt, bool move_up, bool move_down, const GameSettings& settings);

    bool parry_on_cooldown() const { return parry_timer > 0.0f; };

    void trigger_parry(const GameSettings& settings);

    RectCollider collider(const GameSettings& settings) const;
};

}
