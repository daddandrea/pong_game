#pragma once

#include "glm/ext/vector_float2.hpp"
#include <string>

namespace renderer {

struct Button {
    static constexpr float DEFAULT_W = 5.0f;
    static constexpr float DEFAULT_H = 0.9f;
    static constexpr float PAUSE_W   = 4.0f;
    static constexpr float PAUSE_H   = 0.85f;

    int         item = 0;
    std::string label;
    glm::vec2   center;
    glm::vec2   size;
    bool        hovered  = false;
    bool        selected = false;

    bool contains(glm::vec2 pt) const;
};

}
