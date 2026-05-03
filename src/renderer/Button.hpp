#pragma once

#include "glm/ext/vector_float2.hpp"
#include <string>

namespace renderer {

struct Button {
    int         item = 0;
    std::string label;
    glm::vec2   center;
    glm::vec2   size;
    bool        hovered = false;

    bool contains(glm::vec2 pt) const;
};

}
