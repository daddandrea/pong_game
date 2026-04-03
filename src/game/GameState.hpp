#pragma once

#include "game/GameConfig.hpp"

namespace game {

enum class GamePhase {
    Countdown,
    Playing,
    PointScored,
    GameOver,
};

enum class Winner { Left, Right };

struct GameState {
    int score_right         = 0;
    int score_left          = 0;
    int win_score           = STANDARD_WIN_SCORE;
    GamePhase phase = GamePhase::Countdown;
    Winner winner   = Winner::Left;
};

inline GameState g_state;
}

