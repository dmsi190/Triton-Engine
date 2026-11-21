// gpu_resource.hpp

#pragma once

#include "types.hpp"

namespace triton
{
    struct SGPUResource
    {
        types::qword instance = 0;
        types::qword view = 0;

        explicit SGPUResource(types::qword instance, types::qword view)
            : instance(instance), view(view) {}
        
        virtual ~SGPUResource() = default;

        inline types::boolean IsValid() const
        {
            return instance ? types::True : types::False;
        }

        inline void Invalidate()
        {
            instance = 0;
            view = 0;
        }
    };
}