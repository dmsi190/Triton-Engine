// gpu_binding_group_resource.hpp

#pragma once

#include "gpu_resource.hpp"
#include "types.hpp"

namespace triton
{
    class SGPUBindingGroupResource : public SGPUResource
    {
        types::qword    pool = 0;
        types::s32      index = 0;

        explicit SGPUBindingGroupResource(
            types::qword instance,
            types::qword view,
            types::qword pool,
            types::s32 index
        ) : SGPUResource(instance, view), pool(pool), index(index) {}

        ~SGPUBindingGroupResource() override = default;

        static SGPUBindingGroupResource Invalid()
        {
            return SGPUBindingGroupResource(
                0, 0, 0, -1
            );
        }
    };
}