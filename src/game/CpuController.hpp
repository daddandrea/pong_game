#pragma once

namespace game {

struct PaddleState;
struct BallState;

class CpuController {
public:
    void update(PaddleState& paddle, const BallState& ball, float dt) const;
};

}
