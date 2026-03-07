#pragma once

#include "game/Math.hpp"
#include "game/Collider.hpp"

#include <glm/glm.hpp>

namespace game {


struct BallState {
    glm::vec2 pos     = BALL_INITIAL_POS;
    float     speed   = BALL_INITIAL_SPEED;
    float     hor_dir = BALL_INITIAL_HOR_DIR; 
    float     ver_dir = BALL_INITIAL_VER_DIR;
    bool      in_play = false;

    void update(float dt);

    void launch(bool go_right, bool go_up);

    void reset();

    void bounce_off_wall();

    void bounce_from_paddle(float ver_dir_new, bool parry = false);

    void incr_speed();

    CircleCollider collider() const;
};

}
