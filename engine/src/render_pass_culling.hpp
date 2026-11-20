// render_pass_culling.hpp

#pragma once

#include "render_pass.hpp"
#include "gpu_buffer_resource.hpp"
#include "gpu_shader_resource.hpp"
#include "gpu_pipeline_resource.hpp"
#include "gpu_binding_group_layout_resource.hpp"
#include "gpu_binding_group_resource.hpp"

namespace triton
{
    class XRenderPassCulling : public IRenderPass
    {
        TRITON_OBJECT(XRenderPassCulling)

        SGPUShaderResource                              _shader;
        SGPUBindingGroupLayoutResource                  _binding_group_layout;
        std::vector<SGPUBindingGroupResource>           _binding_groups;
        SGPUPipelineResource                            _pipeline;

    public:
        explicit XRenderPassCulling(const SMemAlloc& mem_alloc, cContext* context);

        ~XRenderPassCulling() override;

        void Render() override;

        inline const SGPUShaderResource& GetShader() const { return _shader; }
    };
}