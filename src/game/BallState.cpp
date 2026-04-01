#include "BallState.hpp"
#include "game/Math.hpp"
#include "game/DevSettings.hpp"

namespace game {

void BallState::update(float dt) {
    if (!in_play) return;

    pos.x += hor_dir * speed * dt;
    pos.y += ver_dir * speed * dt;
}

void BallState::launch(bool go_right, bool go_up) {
    pos     = BALL_INITIAL_POS;
    speed   = g_dev.ball_base_speed;
    hor_dir = go_right ? 1.0f : -1.0f;
    ver_dir = go_up    ? 1.0f : -1.0f;
    in_play = true;
}

void BallState::reset() {
    pos     = BALL_INITIAL_POS;
    speed   = BALL_INITIAL_SPEED;
    in_play = false;
}

void BallState::bounce_off_wall() {
    ver_dir *= -1;
}

void BallState::bounce_from_paddle(float ver_dir_new, bool parry) {
    hor_dir *= -1;
    ver_dir = ver_dir_new;
    incr_speed();
    if (parry) speed *= g_dev.ball_parry_mult;
}

void BallState::incr_speed() {
    speed += g_dev.ball_speed_step;
}

CircleCollider BallState::collider() const {
    return {pos, g_dev.ball_radius};
}

}
