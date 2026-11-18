// graphics_backend_ogl.hpp

#pragma once

#include "graphics_backend.hpp"

namespace triton
{
	class XGraphicsBackendOGL : public IGraphicsBackend
	{
	public:
		explicit XGraphicsBackendOGL(cContext* context) : IGraphicsBackend(context) {}
		~XGraphicsBackendOGL() override = default;

		// Context
		virtual void CreateGraphicsContext(SWindowBackend& window) override final;

		virtual void MakeWindowGraphicsContextCurrent(const SWindowBackend& window) override final;

		virtual void SwapWindowBuffers(const SWindowBackend& window) override final;

		// Drawcall
		virtual void ClearColor(const cVector4& color) override final;

		virtual void ClearDepth(types::f32 depth) override final;

		virtual void ClearFramebufferColor(types::usize bufferIndex, const cVector4& color) override final;

		virtual void ClearFramebufferDepth(types::f32 depth) override final;

		virtual void Draw(
			types::usize indexCount,
			types::usize vertexElementOffset,
			types::usize indexElementOffset,
			types::usize instanceCount
		) override final;

		virtual void DrawQuad() override final;

		virtual void DrawQuads(types::usize count) override final;

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
        ) override final;

        virtual void BindShader(const SGPUShaderResource& shader) override final;

        virtual void UnbindShader() override final;

        virtual void DestroyShader(const SGPUShaderResource& shader) override final;

        virtual void SetShaderUniform(
            const SGPUShaderResource& shader,
            const std::string& name,
            const glm::mat4& matrix
        ) override final;

        virtual void SetShaderUniform(
            const SGPUShaderResource& shader,
            const std::string& name,
            types::usize count,
            const types::f32* values
        ) override final;

        virtual void SetShaderUniform(
            const SGPUShaderResource& shader,
            const std::string& name,
            types::u32 value
        ) override final;

        virtual void SetShaderUniform(
            const SGPUShaderResource& shader,
            const std::string& name,
            types::s32 value
        ) override final;

        virtual void BindTextureNamed(
            const SGPUShaderResource& shader,
            const SGPUTextureResource& texture,
            const std::string& textureName,
            types::s32 slot
        ) override final;

        virtual SGPUInputLayoutResource CreateInputLayout() override final;

        virtual void BindInputLayout(const SGPUInputLayoutResource& inputLayout) override final;

        virtual void BindRigidInputLayout() override final;

        virtual void BindSkinnedInputLayout() override final;

        virtual void UnbindInputLayout() override final;

        virtual void DestroyInputLayout(const SGPUInputLayoutResource& vertexArray) override final;

        virtual SGPURenderPassResource CreateRenderPass() override final;

        virtual void BindRenderPass(const SGPURenderPassResource& renderPass) override final;

        virtual void UnbindRenderPass(const SGPURenderPassResource& renderPass) override final;

        virtual void BindDepthState(const SDepthState& blendMode) override final;

        virtual void BindBlendState(const SBlendState& blendMode) override final;

        virtual void SetViewport(const SViewport& viewport) override final;

        virtual SGPURenderTargetResource CreateRenderTarget(
            const std::vector<SGPUTextureResource>& colorAttachments,
            const SGPUTextureResource& depthAttachment
        ) override final;

        virtual void ResizeRenderTargetColors(
            SGPURenderTargetResource& renderTarget,
            const glm::vec2& size
        ) override final;

        virtual void ResizeRenderTargetDepth(
            SGPURenderTargetResource& renderTarget,
            const glm::vec2& size
        ) override final;

        virtual void UpdateRenderTargetBuffers(SGPURenderTargetResource& renderTarget) override final;

        virtual void BindRenderTarget(const SGPURenderTargetResource& renderTarget) override final;

        virtual void UnbindRenderTarget() override final;

        virtual void DestroyRenderTarget(const SGPURenderTargetResource& renderTarget) override final;

        // Resource
        virtual SGPUBufferResource CreateBuffer(
            EGPUBufferType type,
            const types::u8* data,
            types::usize byteSize,
            types::s32 slot
        ) override final;

        virtual void BindBuffer(const SGPUBufferResource& buffer) override final;

        virtual void BindBufferNotVAO(const SGPUBufferResource& buffer) override final;

        virtual void UnbindBuffer(const SGPUBufferResource& buffer) override final;

        virtual void WriteBuffer(
            const SGPUBufferResource& buffer,
            types::usize offset,
            types::usize byteSize,
            const types::u8* data
        ) override final;

        virtual void DestroyBuffer(const SGPUBufferResource& buffer) override final;

        virtual SGPUTextureResource CreateTexture(
            const cVector3& size,
            ETextureDimension dimension,
            ETextureFormat format,
            const types::u8* data,
            types::s32 slot
        ) override final;

        virtual SGPUTextureResource ResizeTexture(const SGPUTextureResource& texture, const cVector2& size) override final;

        virtual void BindTexture(const SGPUTextureResource& texture) override final;

        virtual void UnbindTexture(const SGPUTextureResource& texture) override final;

        virtual void WriteTexture(
            const SGPUTextureResource& texture,
            const cVector3& offset,
            const cVector2& size,
            const types::u8* data
        ) override final;

        virtual void WriteTextureToFile(const SGPUTextureResource& texture, const std::string& filename) override final;

        virtual void GenerateTextureMips(const SGPUTextureResource& texture) override final;

        virtual void DestroyTexture(const SGPUTextureResource& texture) override final;
	};
}