// graphics_backend2.hpp

#pragma once

#include <filesystem>
#include <optional>
#include <vector>
#include "backend.hpp"
#include "math.hpp"
#include "input_window_resource.hpp"
#include "gpu_buffer_types.hpp"
#include "graphics_adapter_type_enum.hpp"
#include "graphics_texture_usage_bit_enum.hpp"
#include "texture_dimensions.hpp"
#include "gpu_texture_resource.hpp"
#include "gpu_buffer_resource.hpp"
#include "gpu_render_target_resource.hpp"
#include "gpu_render_pass_resource.hpp"
#include "gpu_shader_resource.hpp"
#include "gpu_pipeline_resource.hpp"
#include "gpu_binding_group_resource.hpp"
#include "gpu_binding_group_layout_resource.hpp"
#include "shader_stage_bit_enum.hpp"
#include "shader_bytecode_files_struct.hpp"
#include "render_native_command_enum.hpp"
#include "render_primitive_topology_enum.hpp"
#include "render_resource_usage_enum.hpp"
#include "render_pass_push_constants_struct.hpp"
#include "render_binding_group_binding_struct.hpp"
#include "render_command_buffer_type_enum.hpp"
#include "render_command_buffer_reset_option_enum.hpp"
#include "rasterizer_state.hpp"
#include "vertex_buffer_format.hpp"

namespace triton
{
	class cContext;

	class IGraphicsBackend2 : public IBackend
	{
		TRITON_OBJECT(IGraphicsBackend2)

	public:
        explicit IGraphicsBackend2(const SMemAlloc& mem_alloc, cContext* context) : IBackend(mem_alloc, context) {}
		~IGraphicsBackend2() override = default;

		// Initialization/Shutdown
		virtual void Initialize(
			SWindowResource& window,
			types::boolean bEnableDebugging,
			const std::vector<const char*> extensions,
			EGraphicsAdapterType deviceType,
			types::usize staging_buffer_byte_size,
			const cVector2& swapchainSize,
			types::usize framesInFlight
		) = 0;

		virtual void Shutdown() = 0;

		virtual void FinalizeSwapchain(const SGPUTextureResource& presentTexture) = 0;

		virtual void ReleaseSwapchainResources() = 0;

		virtual SGPUTextureResource CreateTexture(
			types::boolean bCreateSampler,
			ETextureFormat format,
			types::dword usageMask,
			ETextureDimension dimension,
			const cVector3& size,
			SGPUTextureResource* resultTexture
		) = 0;

		virtual void WriteTexture(
			const SGPUTextureResource& texture,
			const types::u8* data
		) = 0;

		virtual void DestroyTexture(SGPUTextureResource& texture) = 0;

		virtual SGPUBufferResource CreateBuffer(
			EGPUBufferType type,
			const types::usize byteSize
		) = 0;

		virtual void WriteBuffer(
			const SGPUBufferResource& buffer,
			types::usize offset,
			const types::u8* data,
			types::usize data_byte_size
		) = 0;
		
		virtual void DestroyBuffer(SGPUBufferResource& buffer) = 0;

		virtual SGPURenderTargetResource CreateRenderTarget(
			const std::vector<SGPUTextureResource>& colorAttachments,
			const SGPUTextureResource& depthAttachment
		) = 0;

		virtual void DestroyRenderTarget(SGPURenderTargetResource& renderTarget) = 0;

		virtual SGPUPipelineResource CreatePipeline(
			const SGPUShaderResource& shader,
			const std::vector<SGPUBindingGroupLayoutResource>& bindingGroupLayouts,
			types::boolean bUsePushConstants,
			const std::optional<std::reference_wrapper<SViewport>> viewport = std::nullopt,
			std::optional<std::reference_wrapper<SGPURenderTargetResource>> renderTarget = std::nullopt,
			const std::optional<std::reference_wrapper<SGPURenderPassResource>> renderPass = std::nullopt,
			ERenderPrimitiveTopology primitiveTopology = ERenderPrimitiveTopology::Unknown,
			EVertexBufferFormat vertexBufferFormat = EVertexBufferFormat::Unknown
		) = 0;

		virtual void DestroyPipeline(SGPUPipelineResource& pipeline) = 0;

		virtual SGPUBindingGroupLayoutResource CreateBindingGroupLayout(
			const std::vector<SRenderBindingGroupBinding>& bindings
		) = 0;

		virtual void DestroyBindingGroupLayout(SGPUBindingGroupLayoutResource& bindingGroupLayout) = 0;

		virtual SGPUBindingGroupResource CreateBindingGroup(
			types::s32 index,
			const SGPUBindingGroupLayoutResource& bindingGroupLayout,
			const std::vector<SRenderBindingGroupBinding>& buffersToBind,
			const std::vector<SRenderBindingGroupBinding>& texturesToBind
		) = 0;

		virtual void DestroyBindingGroup(SGPUBindingGroupResource& bindingGroup) = 0;

		virtual SGPURenderPassResource CreateRenderPass(
			SGPURenderTargetResource& renderTarget,
			types::boolean bClearRenderTarget,
			const cVector4& clearColor,
			types::f32 clearDepth,
			const std::vector<ERenderResourceUsage>& srcAttachmentsUsage,
			const std::vector<ERenderResourceUsage>& dstAttachmentsUsage
		) = 0;

		virtual void DestroyRenderPass(SGPURenderPassResource& renderPass) = 0;

		virtual SGPUShaderResource CreateShader(
			types::dword stageMask,
			const SShaderBytecodeFiles& bytecodeFiles
		) = 0;

		virtual void DestroyShader(SGPUShaderResource& shader) = 0;

		virtual void Dispatch(
			const SGPUPipelineResource& pipeline,
			const std::vector<SGPUBindingGroupResource>& binding_groups,
			const cVector3& work_group_count
		) = 0;

		virtual void ResetCommandBuffer(
			ERenderCommandBufferType command_buffer,
			ERenderCommandBufferResetOption reset_option
		) = 0;

		virtual void AddCommandToBuffer(
			ENativeRenderCommand command,
			const void* commandArgA,
			const void* commandArgB
		) = 0;

		virtual void BeginFrame() = 0;

		virtual void EndFrame() = 0;
	};
}