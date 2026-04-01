#pragma once

#include "game/DevSettings.hpp"
#include <imgui.h>

namespace scenes {

inline void render_dev_menu() {
    if (!game::g_dev.show_dev) return;

    ImGui::Begin("Dev Settings");
    ImGui::SliderFloat("Ball base speed",   &game::g_dev.ball_base_speed,   1.0f, 30.0f);
    ImGui::SliderFloat("Ball speed step",   &game::g_dev.ball_speed_step,   0.0f, 2.0f);
    ImGui::SliderFloat("Ball parry mult",   &game::g_dev.ball_parry_mult,   1.0f, 5.0f);
    ImGui::SliderFloat("Ball radius",       &game::g_dev.ball_radius,       0.05f, 1.0f);
    ImGui::Separator();
    ImGui::SliderFloat("Paddle speed",      &game::g_dev.paddle_move_speed, 1.0f, 20.0f);
    ImGui::SliderFloat("Paddle height",     &game::g_dev.paddle_half_h,     0.3f, 4.0f);
    ImGui::SliderFloat("Parry cooldown",    &game::g_dev.paddle_parry_cd,   0.1f, 3.0f);
    ImGui::Separator();
    ImGui::SliderFloat("CPU speed",         &game::g_dev.cpu_speed,         1.0f, 20.0f);
    ImGui::Separator();
    ImGui::SliderInt("Win score",           &game::g_dev.win_score,         1, 21);
    ImGui::Separator();
    if (ImGui::Button("Reset defaults"))    game::g_dev = game::DevSettings{};
    ImGui::End();
}

} // namespace scenes
