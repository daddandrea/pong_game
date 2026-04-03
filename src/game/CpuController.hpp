#pragma once

namespace game {

struct PaddleState;
struct BallState;
struct GameSettings;

class CpuController {
public:
    void update(PaddleState& paddle, const BallState& ball, float dt, const GameSettings& settings) const;
};

}
