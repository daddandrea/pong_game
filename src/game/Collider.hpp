#pragma once

#include <glm/glm.hpp>

namespace game {

struct RectCollider {
    glm::vec2 center;
    glm::vec2 half_extents;

    float left()   const { return center.x - half_extents.x; }
    float right()  const { return center.x + half_extents.x; }
    float top()    const { return center.y + half_extents.y; }
    float bottom() const { return center.y - half_extents.y; }
};

struct CircleCollider {
    glm::vec2 center;
    float radius;
};

bool intersects(const CircleCollider& c, const RectCollider& r);

bool intersects(const RectCollider& a, const RectCollider& b);

}
