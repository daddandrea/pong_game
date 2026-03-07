#include "Collider.hpp"

#include <algorithm>

namespace game {

bool intersects(const CircleCollider& c, const RectCollider& r) {
    const float cx = std::clamp(c.center.x, r.left(), r.right());
    const float cy = std::clamp(c.center.y, r.bottom(), r.top());

    const float dx = c.center.x - cx;
    const float dy = c.center.y - cy;

    return (dx * dx + dy * dy) < (c.radius * c.radius);
}

bool intersects(const RectCollider& a, const RectCollider& b) {
    return a.left()   < b.right()
        && a.right()  > b.left()
        && a.bottom() < b.top()
        && a.top()    > b.bottom();
}

}
