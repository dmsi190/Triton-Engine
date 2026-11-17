// gpu_input_layout.hpp

#pragma once

#include "gpu_resource.hpp"
#include "types.hpp"

namespace triton
{
    struct SGPUInputLayoutResource : public SGPUResource
    {
        explicit SGPUInputLayoutResource(
            types::qword instance,
            types::qword view_instance
        ) : SGPUResource(instance, view_instance) {}

        ~SGPUInputLayoutResource() override = default;

        static SGPUInputLayoutResource Invalid()
        {
            return SGPUInputLayoutResource(
                0, 0
            );
        }
    };
}