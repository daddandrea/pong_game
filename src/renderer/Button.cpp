#include "Button.hpp"
#include <algorithm>

namespace renderer {

int select_hovered(std::vector<Button>& buttons, int& sel, glm::vec2 pt) {
    auto it = std::ranges::find_if(
        buttons,
        [pt](const Button& b) { return b.contains(pt); }
    );

    if (it == buttons.end()) return -1;
    if (sel >= 0) buttons[sel].selected = false;

    sel = static_cast<int>(it - buttons.begin());
    it->selected = true;
    return sel;
}

bool Button::contains(glm::vec2 pt) const {
    return pt.x >= center.x - size.x / 2.0f
        && pt.x <= center.x + size.x / 2.0f
        && pt.y >= center.y - size.y / 2.0f
        && pt.y <= center.y + size.y / 2.0f;
}

}
