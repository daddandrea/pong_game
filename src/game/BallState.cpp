#include "BallState.hpp"
#include "game/Math.hpp"

namespace game {

void BallState::update(float dt) {
    if (!in_play) return;

    pos.x += hor_dir * speed * dt;
    pos.y += ver_dir * speed * dt;
}

void BallState::launch(bool go_right, bool go_up, const GameSettings& settings) {
    pos     = BALL_INITIAL_POS;
    speed   = settings.ball_base_speed;
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

void BallState::bounce_from_paddle(float ver_dir_new, const GameSettings& settings, bool parry) {
    hor_dir *= -1;
    ver_dir = ver_dir_new;
    incr_speed(settings);
    if (parry) speed *= settings.ball_parry_mult;
}

void BallState::incr_speed(const GameSettings& settings) {
    speed += settings.ball_speed_step;
}

CircleCollider BallState::collider(const GameSettings& settings) const {
    return {pos, settings.ball_radius};
}

}
