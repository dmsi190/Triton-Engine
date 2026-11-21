// gpu_binding_group_layout_resource.hpp

#pragma once

#include <vector>
#include "gpu_resource.hpp"
#include "render_binding_group_binding_struct.hpp"
#include "types.hpp"

namespace triton
{
    struct SGPUBindingGroupLayoutResource : public SGPUResource
    {
        std::vector<SRenderBindingGroupBinding> bindings;

        explicit SGPUBindingGroupLayoutResource(
            types::qword instance,
            types::qword view,
            const std::vector<SRenderBindingGroupBinding>& bindings
        ) : SGPUResource(instance, view), bindings(bindings) {}

        ~SGPUBindingGroupLayoutResource() override = default;

        static SGPUBindingGroupLayoutResource Invalid()
        {
            return SGPUBindingGroupLayoutResource(0, 0, {});
        }
    };
}