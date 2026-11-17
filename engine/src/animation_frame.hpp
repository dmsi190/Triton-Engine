// animation_frame.hpp

#pragma once

#include <vector>
#include "math.hpp"

namespace triton
{
    struct SAnimationFrameBone
    {
        cMatrix4 transform = cMatrix4();
    };

    struct SAnimationFrame
    {
        std::vector<SAnimationFrameBone> bones = {};
    };
}