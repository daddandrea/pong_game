#pragma once
#include "game/Math.hpp"

namespace game {
struct DevSettings {
    bool  show_dev          = false;
    int   win_score         = 7;
    float ball_base_speed   = BALL_BASE_SPEED;
    float ball_speed_step   = BALL_SPEED_STEP;
    float ball_parry_mult   = BALL_PARRY_MULT;
    float ball_radius       = BALL_RADIUS;
    float paddle_move_speed = PADDLE_MOVE_SPEED;
    float paddle_half_h     = PADDLE_HALF_H;
    float paddle_parry_cd   = PADDLE_PARRY_CD;
    float cpu_speed         = CPU_SPEED;
};
inline DevSettings g_dev;
} // namespace game
