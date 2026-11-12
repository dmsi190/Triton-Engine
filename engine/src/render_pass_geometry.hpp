// render_pass_geometry.hpp

#pragma once

#include <vector>
#include <optional>
#include "render_pass.hpp"
#include "math.hpp"
#include "render_target.hpp"
#include "shader_texture_binding.hpp"
#include "render_shading_model_enum.hpp"
#include "render_native_draw_command_info_struct.hpp"
#include "render_primitive_topology_enum.hpp"
#include "render_resource_usage_enum.hpp"
#include "render_pass_push_constants_struct.hpp"
#include "gpu_pipeline_resource.hpp"
#include "input_layout.hpp"
#include "shader.hpp"
#include "camera.hpp"
#include "types.hpp"

namespace triton
{
    class XRenderPassGeometry : public IRenderPass
    {
        TRITON_CLASS_NAME(XRenderPassGeometry)

        SHandle                                         _renderDomain;
        std::optional<SClearState>                      _clearState = std::nullopt;
        SHandle                                         _inputLayout;
        std::vector<CGPUBufferResource>                 _inputBuffers = {};
        std::vector<SShaderTextureBinding>              _inputTextures = {};
        SDepthState                                     _depthState;
        SBlendState                                     _blendState;
        EPrimitiveTopology                              _primitiveTopology = EPrimitiveTopology::Unknown;
        SViewport                                       _viewport = {};
        types::boolean                                  _bClearRenderTarget = types::False;
        SHandle                                         _renderTarget;
        SHandle                                         _shader;
        CGPUPipelineResource                            _gpuPipeline = CGPUPipelineResource::Invalid();
        SNativeCommandDrawInfo                          _nativeCommandDrawInfoArrays[2];
        SRenderPassGPUPushConstantsLayout               _pushConstantArrays[2];
        std::vector<CGPUBindingGroupLayoutResource>     _gpuBindingGroupLayouts;
        std::vector<CGPUBindingGroupResource>           _gpuBindingGroupBuffersArrays[2];

    public:
        explicit XRenderPassGeometry(
            cContext* context,
            types::s32 poolIndex,
            const SHandle& renderDomain,
            const SHandle& renderTargetHandle,
            types::boolean bClearRenderTarget,
            const cVector4& clearColor,
            types::f32 clearDepth,
            const std::vector<EResourceUsage>& srcAttachmentsUsage,
            const std::vector<EResourceUsage>& dstAttachmentsUsage,
            const SHandle& shaderHandle,
            EPrimitiveTopology primitiveTopology,
            const SViewport& viewport,
            const std::vector<CGPUBufferResource>& inputBuffers,
            const std::vector<SShaderTextureBinding>& inputTextures
        );
        ~XRenderPassGeometry() override;

        void Render() override;

        inline types::boolean IsRenderTargetClearRequired() const { return _bClearRenderTarget; }

        inline const SHandle& GetRenderDomain() { return _renderDomain; }

        inline const SHandle& GetShader() { return _shader; }

        struct TGPULayout {};

    private:
        void Bind();

        void Draw();

        void Unbind();
    };
}