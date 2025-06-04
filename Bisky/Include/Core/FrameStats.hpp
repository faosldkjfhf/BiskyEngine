#pragma once

#include <cmath>

namespace bisky::core
{

struct FrameStats
{
    float    frameTime;
    uint32_t triangleCount;
    uint32_t drawCount;
    float    sceneUpdateTime;
    float    meshDrawTime;
};

} // namespace bisky::core