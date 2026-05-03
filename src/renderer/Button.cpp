#include "Button.hpp"

namespace renderer {

bool Button::contains(glm::vec2 pt) const {
    return pt.x >= center.x - size.x / 2.0f
        && pt.x <= center.x + size.x / 2.0f
        && pt.y >= center.y - size.y / 2.0f
        && pt.y <= center.y + size.y / 2.0f;
}

}
