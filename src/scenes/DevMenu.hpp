#pragma once

#include "game/GameState.hpp"

namespace scenes {

void on_win_score_changed(game::GameState* state);
void render_dev_menu(game::GameState* state);

} // namespace scenes
