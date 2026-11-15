// render_pass_culling.hpp

#pragma once

#include "render_pass.hpp"
#include "handle.hpp"
#include "gpu_buffer_resource.hpp"
#include "gpu_shader_resource.hpp"
#include "gpu_pipeline_resource.hpp"
#include "gpu_binding_group_layout_resource.hpp"
#include "gpu_binding_group_resource.hpp"

namespace triton
{
    class XRenderPassCulling : public IRenderPass
    {
        TRITON_CLASS_NAME(XRenderPassCulling)

        CGPUBufferResource                              _indirect_draw_command_counter_buffer;
        CGPUBufferResource                              _indirect_draw_commands_buffer;
        CGPUBufferResource                              _motion_type_per_command_buffer;
        CGPUShaderResource                              _shader;
        CGPUBindingGroupLayoutResource                  _binding_group_layout;
        std::vector<CGPUBindingGroupResource>           _binding_groups;
        CGPUPipelineResource                            _pipeline;

    public:
        explicit XRenderPassCulling(cContext* context, types::s32 poolIndex);

        ~XRenderPassCulling() override;

        void Render() override;

        inline const CGPUShaderResource& GetShader() const { return _shader; }
    };
}