// gpu_shader.hpp

#pragma once

#include <vector>
#include "gpu_resource.hpp"
#include "types.hpp"

namespace triton
{
    struct SGPUShaderResource : public SGPUResource
    {
        types::dword                stage_mask = 0;
        std::vector<types::qword>   modules;

        explicit SGPUShaderResource(
            types::qword instance,
            types::qword view_instance,
            types::dword stage_mask,
            const std::vector<types::qword>& shader_modules
        ) : SGPUResource(instance, view_instance), stage_mask(stage_mask), modules(shader_modules) {}

        ~SGPUShaderResource() override = default;

        static SGPUShaderResource Invalid()
        {
            return SGPUShaderResource(
                0, 0, 0, std::vector<types::qword>()
            );
        }
    };
}