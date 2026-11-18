// gpu_render_pass_resource.hpp

#pragma once

#include "gpu_resource.hpp"
#include "math.hpp"
#include "types.hpp"

namespace triton
{
    struct SGPURenderPassResource : public SGPUResource
    {
        types::qword    framebuffer = 0;
        types::boolean  clear_render_target = types::K_FALSE;
        cVector4        clear_color = cVector4(0.0f);
        types::f32      clear_depth = 0.0f;

        explicit SGPURenderPassResource(
            types::qword instance,
            types::qword view_instance,
            types::qword framebuffer,
            types::boolean clear_render_target,
            const cVector4& clear_color,
            types::f32 clear_depth
        ) :
            SGPUResource(instance, view_instance),
            framebuffer(framebuffer),
            clear_render_target(clear_render_target),
            clear_color(clear_color),
            clear_depth(clear_depth) {}

        ~SGPURenderPassResource() override = default;

        static SGPURenderPassResource Invalid()
        {
            return SGPURenderPassResource(
                0, 0, 0, types::K_FALSE, cVector4(0.0f), 0.0f
            );
        }
    };
}