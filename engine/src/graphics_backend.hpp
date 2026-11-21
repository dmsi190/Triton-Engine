// graphics_backend.hpp

#pragma once

#include "backend.hpp"
#include "input_window_resource.hpp"
#include "math.hpp"
#include "gpu_resource.hpp"
#include "shader_define.hpp"
#include "gpu_texture_resource.hpp"
#include "gpu_shader_resource.hpp"
#include "gpu_buffer.hpp"
#include "gpu_input_layout_resource.hpp"
#include "gpu_render_pass_resource.hpp"
#include "gpu_render_target_resource.hpp"
#include "rasterizer_state.hpp"
#include "gpu_buffer_types.hpp"

namespace triton
{
    class cContext;

	class IGraphicsBackend : public IBackend
	{
        TRITON_OBJECT(IGraphicsBackend)

	public:
        explicit IGraphicsBackend(const SMemAlloc& mem_alloc, cContext* context)
            : IBackend(mem_alloc, context) {}

		~IGraphicsBackend() override = default;

		// Context
		virtual void CreateGraphicsContext(SWindowResource& window) = 0;

		virtual void MakeWindowGraphicsContextCurrent(const SWindowResource& window) = 0;

		virtual void SwapWindowBuffers(const SWindowResource& window) = 0;

		// Drawcall
		virtual void ClearColor(const cVector4& color) = 0;

		virtual void ClearDepth(types::f32 depth) = 0;

		virtual void ClearFramebufferColor(types::usize bufferIndex, const cVector4& color) = 0;

		virtual void ClearFramebufferDepth(types::f32 depth) = 0;

		virtual void Draw(
			types::usize indexCount,
			types::usize vertexElementOffset,
			types::usize indexElementOffset,
			types::usize instanceCount
		) = 0;

		virtual void DrawQuad() = 0;

		virtual void DrawQuads(types::usize count) = 0;

		// Pipeline
        virtual SGPUShaderResource CreateShader(
            const char* vertexStr,
            const char* fragmentStr,
            const char* vertexCustomFuncStr,
            const char* fragmentCustomFuncStr,
            types::usize defineCount,
            const SShaderDefine* defines,
            types::usize vertexIncludePathCount,
            const char** vertexIncludePaths,
            types::usize fragmentIncludePathCount,
            const char** fragmentIncludePaths
        ) = 0;

        virtual void BindShader(const SGPUShaderResource& shader) = 0;

        virtual void UnbindShader() = 0;

        virtual void DestroyShader(const SGPUShaderResource& shader) = 0;

        virtual void SetShaderUniform(
            const SGPUShaderResource& shader,
            const std::string& name,
            const glm::mat4& matrix
        ) = 0;

        virtual void SetShaderUniform(
            const SGPUShaderResource& shader,
            const std::string& name,
            types::usize count,
            const types::f32* values
        ) = 0;

        virtual void SetShaderUniform(
            const SGPUShaderResource& shader,
            const std::string& name,
            types::u32 value
        ) = 0;

        virtual void SetShaderUniform(
            const SGPUShaderResource& shader,
            const std::string& name,
            types::s32 value
        ) = 0;

        virtual void BindTextureNamed(
            const SGPUShaderResource& shader,
            const SGPUTextureResource& texture,
            const std::string& textureName,
            types::s32 slot
        ) = 0;

        virtual SGPUInputLayoutResource CreateInputLayout() = 0;

        virtual void BindInputLayout(const SGPUInputLayoutResource& vertexArray) = 0;

        virtual void BindRigidInputLayout() = 0;

        virtual void BindSkinnedInputLayout() = 0;

        virtual void UnbindInputLayout() = 0;

        virtual void DestroyInputLayout(const SGPUInputLayoutResource& vertexArray) = 0;

        virtual SGPURenderPassResource CreateRenderPass() = 0;

        virtual void BindRenderPass(const SGPURenderPassResource& renderPass) = 0;

        virtual void UnbindRenderPass(const SGPURenderPassResource& renderPass) = 0;

        virtual void BindDepthState(const SDepthState& blendMode) = 0;

        virtual void BindBlendState(const SBlendState& blendMode) = 0;

        virtual void SetViewport(const SViewport& viewport) = 0;

        virtual SGPURenderTargetResource CreateRenderTarget(
            const std::vector<SGPUTextureResource>& colorAttachments,
            const SGPUTextureResource& depthAttachment
        ) = 0;

        virtual void ResizeRenderTargetColors(
            SGPURenderTargetResource& renderTarget,
            const glm::vec2& size
        ) = 0;

        virtual void ResizeRenderTargetDepth(
            SGPURenderTargetResource& renderTarget,
            const glm::vec2& size
        ) = 0;

        virtual void UpdateRenderTargetBuffers(SGPURenderTargetResource& renderTarget) = 0;

        virtual void BindRenderTarget(const SGPURenderTargetResource& renderTarget) = 0;

        virtual void UnbindRenderTarget() = 0;

        virtual void DestroyRenderTarget(const SGPURenderTargetResource& renderTarget) = 0;

        // Resource
        virtual SGPUBufferResource CreateBuffer(
            EGPUBufferType type,
            const types::u8* data,
            types::usize byteSize,
            types::s32 slot
        ) = 0;

        virtual void BindBuffer(const SGPUBufferResource& buffer) = 0;

        virtual void BindBufferNotVAO(const SGPUBufferResource& buffer) = 0;

        virtual void UnbindBuffer(const SGPUBufferResource& buffer) = 0;

        virtual void WriteBuffer(
            const SGPUBufferResource& buffer,
            types::usize offset,
            types::usize byteSize,
            const types::u8* data
        ) = 0;

        virtual void DestroyBuffer(const SGPUBufferResource& buffer) = 0;

        virtual SGPUTextureResource CreateTexture(
            const cVector3& size,
            ETextureDimension dimension,
            ETextureFormat format,
            const types::u8* data,
            types::s32 slot
        ) = 0;

        virtual SGPUTextureResource ResizeTexture(const SGPUTextureResource& texture, const cVector2& size) = 0;

        virtual void BindTexture(const SGPUTextureResource& texture) = 0;

        virtual void UnbindTexture(const SGPUTextureResource& texture) = 0;

        virtual void WriteTexture(
            const SGPUTextureResource& texture,
            const cVector3& offset,
            const cVector2& size,
            const types::u8* data
        ) = 0;

        virtual void WriteTextureToFile(const SGPUTextureResource& texture, const std::string& filename) = 0;

        virtual void GenerateTextureMips(const SGPUTextureResource& texture) = 0;

        virtual void DestroyTexture(const SGPUTextureResource& texture) = 0;
	};
}