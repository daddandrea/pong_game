#include "Button.hpp"

namespace renderer {

bool Button::contains(glm::vec2 pt) const {
    return pt.x >= center.x - size.x / 2.0f
        && pt.x <= center.x + size.x / 2.0f
        && pt.y >= center.y - size.y / 2.0f
        && pt.y <= center.y + size.y / 2.0f;
}

void Button::handle_hover(glm::vec2 pt) {
    selected = contains(pt);
    hovered  = selected;
}

std::optional<int> Button::handle_click(glm::vec2 pt) {
    if (!contains(pt))
        return std::nullopt;

    selected = true;
    hovered  = true;
    return item;
}

std::optional<int> Button::handle_select() const {
    if (selected)
        return item;

    return std::nullopt;
}

}
