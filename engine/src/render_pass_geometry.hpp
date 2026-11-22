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
#include "shader.hpp"
#include "camera.hpp"
#include "rasterizer_state.hpp"
#include "types.hpp"

namespace triton
{
    class XRenderDomain;

    class XRenderPassGeometry : public IRenderPass
    {
        TRITON_OBJECT(XRenderPassGeometry)

        XRenderDomain*                                  _renderDomain;
        std::optional<SClearState>                      _clearState = std::nullopt;
        std::vector<SGPUBufferResource>                 _inputBuffers = {};
        std::vector<SShaderTextureBinding>              _inputTextures = {};
        SDepthState                                     _depthState;
        SBlendState                                     _blendState;
        ERenderPrimitiveTopology                        _primitiveTopology = ERenderPrimitiveTopology::Unknown;
        SViewport                                       _viewport = {};
        types::boolean                                  _bClearRenderTarget = types::False;
        XRenderTarget*                                  _renderTarget;
        XShader*                                        _shader;
        SGPUPipelineResource                            _gpuPipeline = SGPUPipelineResource::Invalid();
        SNativeCommandDrawInfo                          _nativeCommandDrawInfoArrays[2];
        SRenderPassPushConstantsLayout                  _pushConstantArrays[2];
        std::vector<SGPUBindingGroupLayoutResource>     _gpuBindingGroupLayouts;
        std::vector<SGPUBindingGroupResource>           _gpuBindingGroupBuffersArrays[2];

    public:
        explicit XRenderPassGeometry(
            const SMemAlloc& mem_alloc,
            cContext* context,
            XRenderDomain* renderDomain,
            XRenderTarget* render_target,
            types::boolean bClearRenderTarget,
            const cVector4& clearColor,
            types::f32 clearDepth,
            const std::vector<ERenderResourceUsage>& srcAttachmentsUsage,
            const std::vector<ERenderResourceUsage>& dstAttachmentsUsage,
            XShader* shader,
            ERenderPrimitiveTopology primitiveTopology,
            const SViewport& viewport,
            const std::vector<SGPUBufferResource>& inputBuffers,
            const std::vector<SShaderTextureBinding>& inputTextures
        );
        ~XRenderPassGeometry() override;

        void Render() override;

        inline types::boolean IsRenderTargetClearRequired() const { return _bClearRenderTarget; }

        inline XRenderDomain* GetRenderDomain() { return _renderDomain; }

        inline XShader* GetShader() { return _shader; }

    private:
        void Bind();

        void Draw();

        void Unbind();
    };
}