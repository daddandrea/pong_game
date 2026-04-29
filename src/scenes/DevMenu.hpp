#pragma once

#include "game/DevSettings.hpp"
#include "game/GameSettings.hpp"
#include "game/GameState.hpp"

namespace scenes {

#ifdef PONG_DEV
void on_win_score_changed(game::GameState* state);
void render_dev_menu(game::GameState* state, game::GameSettings& settings, const game::DevSettings& dev);
#endif

} // namespace scenes
