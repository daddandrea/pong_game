#include "Physics.hpp"

#include "game/BallState.hpp"
#include "game/Collider.hpp"
#include "game/Math.hpp"
#include "game/PaddleState.hpp"

#include <algorithm>

namespace game {

bool handle_ball_paddle_collision(BallState &ball, PaddleState &paddle, bool parry_active) {
    if (!ball.in_play) return false;

    const CircleCollider bc = ball.collider();
    const RectCollider   pc = paddle.collider();

    if (!intersects(bc, pc)) return false;

    /// Collision position relative to paddle center [-1; +1].
    const float rel_pos = std::clamp(
        (ball.pos.y - paddle.pos.y) / PADDLE_HALF_H,
        -1.0f,
        1.0f
    );
    // Map [-1; +1] to the vertical direction
    float new_ver_dir = rel_pos;

    // Ensure a minimum vertical component
    if (std::abs(new_ver_dir) < 0.2f)
        new_ver_dir = (new_ver_dir > 0.0f)
                        ? 0.2f
                        : -0.2f;

    // Prevent the ball from sticking to the paddle
    if (ball.hor_dir < 0.0f)
        ball.pos.x = pc.right() + BALL_RADIUS + 0.0001f;
    else
        ball.pos.x = pc.left()  - BALL_RADIUS - 0.0001f;

    ball.bounce_from_paddle(new_ver_dir, parry_active && !paddle.parry_on_cooldown());

    if (parry_active && !paddle.parry_on_cooldown()) {
        paddle.trigger_parry();
    }

    return true;
}

int check_score(const BallState &ball) {
    if (!ball.in_play) return 0;

    if (ball.pos.x - BALL_RADIUS <= ARENA_LEFT)  return 1;   // Point for right 
    if (ball.pos.x + BALL_RADIUS >= ARENA_RIGHT) return -1; // Point for left 

    return 0;
}

bool is_game_over(int score_left, int score_right, int win_score) {
    return score_left >= win_score
        || score_right >= win_score;
}

bool handle_wall_bounce(BallState &ball) {
    if (!ball.in_play) return false;

    bool bounced = false;

    if (ball.pos.y + BALL_RADIUS >= ARENA_TOP) {

        ball.pos.y = ARENA_TOP - BALL_RADIUS - 0.001f;
        ball.bounce_off_wall();
        bounced = true;

    } else if (ball.pos.y - BALL_RADIUS <= ARENA_BOTTOM) {

        ball.pos.y = ARENA_BOTTOM + BALL_RADIUS + 0.001f;
        ball.bounce_off_wall();
        bounced = true;

    }

    return bounced;
}

}
