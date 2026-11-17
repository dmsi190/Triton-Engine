// animation.hpp

#pragma once

#include <optional>
#include "object.hpp"
#include "handle.hpp"
#include "animation_key.hpp"
#include "animation_frame.hpp"
#include "skeleton.hpp"

namespace triton
{
    class XAnimation : public iObject
    {
        TRITON_OBJECT(XAnimation)

        std::string _name = "";
        types::f32 _duration = 0.0f;
        types::f32 _ticks_per_second = 0.0f;
        std::vector<SAnimationKey> _keys = {};

    public:
        explicit XAnimation(
            const SMemoryAllocation& mem_alloc,
            cContext* context,
            const std::string& name,
            types::f32 duration,
            types::f32 ticks_per_second,
            const std::vector<SAnimationKey>& keys
        );
        
        ~XAnimation() override = default;

        std::optional<triton::SAnimationFrame> EvaluateFrame(
            const XSkeleton* skeleton,
            types::f32 time
        );
    };
}