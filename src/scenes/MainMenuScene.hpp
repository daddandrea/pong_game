#pragma once

#include "scenes/IScene.hpp"

#include <glm/glm.hpp>

#include <string>
#include <vector>

namespace scenes {

struct Button {
    int item = 0;
    std::string label;
    glm::vec2 center;
    glm::vec2 size;
    bool hovered = false;

    bool contains(glm::vec2 pt) const;
};

enum class MenuItem {
    PlayerVsCpu,
    PlayerVsPlayer,
    CpuVsCpu,
    Credits,
    Quit
};

class MainMenuScene : public IScene {

public:
    MainMenuScene();

    std::string update(const core::InputState &input, float dt) override;
    void render(renderer::Renderer2D &r) const override;

private:
    std::vector<Button> m_buttons;
    int m_hovered_idx = -1;

    static constexpr float BTN_W = 5.0f;
    static constexpr float BTN_H = 0.9f;

};

}
