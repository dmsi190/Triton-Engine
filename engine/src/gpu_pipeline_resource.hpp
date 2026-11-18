// gpu_pipeline_resource.hpp

#pragma once

#include <vector>
#include "gpu_resource.hpp"
#include "gpu_binding_group_resource.hpp"
#include "gpu_binding_group_layout_resource.hpp"
#include "render_pipeline_bind_point_enum.hpp"
#include "types.hpp"

namespace triton
{
    struct SGPUPipelineResource : public SGPUResource
    {
        EPipelineBindPoint                              bind_point = EPipelineBindPoint::Unknown;
        types::qword                                    layout = 0;
        std::vector<SGPUBindingGroupLayoutResource>     binding_group_layouts;

        explicit SGPUPipelineResource(
            types::qword instance,
            types::qword view_instance,
            EPipelineBindPoint bind_point,
            types::qword layout,
            const std::vector<SGPUBindingGroupLayoutResource>& binding_group_layouts
        ) :
            SGPUResource(instance, view_instance),
            bind_point(bind_point),
            layout(layout),
            binding_group_layouts(binding_group_layouts) {}

        ~SGPUPipelineResource() override = default;

        static SGPUPipelineResource Invalid()
        {
            return SGPUPipelineResource(
                0, 0, EPipelineBindPoint::Unknown, 0, {}
            );
        }
    };
}