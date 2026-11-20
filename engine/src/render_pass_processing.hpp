// render_pass_processing.hpp

#pragma once

#include <vector>
#include "render_pass.hpp"
#include "shader_texture_binding.hpp"
#include "shader.hpp"
#include "rasterizer_state.hpp"
#include "render_target.hpp"
#include "gpu_buffer_resource.hpp"
#include "types.hpp"

namespace triton
{
    class XRenderPassProcessing : public IRenderPass
    {
        TRITON_OBJECT(XRenderPassProcessing)

        std::vector<SGPUBufferResource>             _inputBuffers = {};
        std::vector<SShaderTextureBinding>          _inputTextures = {};
        SDepthState                                 _depthState = SDepthState(0, 0);
        SBlendState                                 _blendState = {};
        SViewport                                   _viewport = {};
        XRenderTarget*                              _renderTarget;
        XShader*                                    _shader;

    public:
        explicit XRenderPassProcessing(
            const SMemAlloc& mem_alloc,
            cContext* context,
            const std::vector<SGPUBufferResource>& inputBuffers,
            const std::vector<SShaderTextureBinding>& inputTextures,
            const SViewport& viewport,
            const SBlendState& blendState,
            const SDepthState& depthState,
            XRenderTarget* renderTarget,
            XShader* shader
        );
        ~XRenderPassProcessing() override = default;

        void Render() override;

        inline const std::vector<SGPUBufferResource>& GetInputBuffers() const { return _inputBuffers; }

        inline const std::vector<SShaderTextureBinding>& GetInputTextures() const { return _inputTextures; }

        inline XShader* GetShader() const { return _shader; }
    };
}