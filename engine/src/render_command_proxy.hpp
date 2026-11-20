// render_command_proxy.hpp

#pragma once

#include <optional>
#include <functional>
#include "gpu_buffer_resource.hpp"
#include "gpu_shader_resource.hpp"
#include "shader_stage_bit_enum.hpp"
#include "shader_bytecode_files_struct.hpp"
#include "gpu_binding_group_layout_resource.hpp"
#include "gpu_binding_group_resource.hpp"
#include "gpu_pipeline_resource.hpp"
#include "gpu_render_target_resource.hpp"
#include "gpu_render_pass_resource.hpp"
#include "graphics_texture_usage_bit_enum.hpp"
#include "rasterizer_state.hpp"
#include "render_primitive_topology_enum.hpp"
#include "render_resource_usage_enum.hpp"
#include "vertex_buffer_format.hpp"
#include "object.hpp"

namespace triton
{
	class cContext;
	
	class CRenderCommandProxy final
	{
		inline static cContext* _context;

	public:
		static void SetContext(cContext* context);

		static SGPUBufferResource CreateBuffer(EGPUBufferType type, types::usize byte_size);

		static void WriteBuffer(
			const SGPUBufferResource& buffer,
			types::usize offset,
			const types::u8* data,
			types::usize byte_size
		);

		static void DestroyBuffer(SGPUBufferResource& buffer);

		static SGPUShaderResource CreateShader(EShaderStageBit stage_mask, const SShaderBytecodeFiles& files);

		static void DestroyShader(SGPUShaderResource& shader);

		static SGPUBindingGroupLayoutResource CreateBindingGroupLayout(
			const std::vector<SRenderBindingGroupBinding>& bindings
		);

		static void DestroyBindingGroupLayout(SGPUBindingGroupLayoutResource& binding_group_layout);

		static SGPUBindingGroupResource CreateBindingGroup(
			types::u32 index,
			const SGPUBindingGroupLayoutResource& layout,
			const std::vector<SRenderBindingGroupBinding>& buffers_to_bind,
			const std::vector<SRenderBindingGroupBinding>& textures_to_bind
		);

		static void DestroyBindingGroup(SGPUBindingGroupResource& binding_group);

		static SGPUPipelineResource CreatePipeline(
			const SGPUShaderResource& shader,
			const std::vector<SGPUBindingGroupLayoutResource>& bindingGroupLayouts,
			types::boolean bUsePushConstants,
			const std::optional<std::reference_wrapper<SViewport>> viewport = std::nullopt,
			std::optional<std::reference_wrapper<SGPURenderTargetResource>> renderTarget = std::nullopt,
			const std::optional<std::reference_wrapper<SGPURenderPassResource>> renderPass = std::nullopt,
			ERenderPrimitiveTopology primitiveTopology = ERenderPrimitiveTopology::Unknown,
			EVertexBufferFormat vertexBufferFormat = EVertexBufferFormat::Unknown
		);

		static void DestroyPipeline(SGPUPipelineResource& pipeline);

		static SGPURenderPassResource CreateRenderPass(
			const SGPURenderTargetResource& render_target_resource,
			boolean clear_render_target,
			const cVector4 clear_color,
			const f32 clear_depth,
			const std::vector<ERenderResourceUsage>& src_render_target_attachments_usage,
			const std::vector<ERenderResourceUsage>& dst_render_target_attachments_usage
		);

		static void DestroyRenderPass(const SGPURenderPassResource& render_pass);

		static SGPURenderTargetResource CreateRenderTarget(
			const std::vector<std::reference_wrapper<SGPUTextureResource>>& color_attachments,
			const SGPUTextureResource& depth_attachment
		);

		static void DestroyRenderTarget(const SGPURenderTargetResource& render_target);

		static SGPUTextureResource CreateTexture(
			types::boolean create_sampler,
			ETextureFormat format,
			ETextureUsageBit usage_mask,
			ETextureDimension dimension,
			const cVector3& size
		);

		static void WriteTexture(const SGPUTextureResource& texture, const void* data);

		static void GenerateTextureMips(const SGPUTextureResource& texture);

		static void DestroyTexture(const SGPUTextureResource& texture);

		static void Dispatch(
			const SGPUPipelineResource& pipeline,
			const std::vector<SGPUBindingGroupResource>& binding_groups,
			const cVector3& work_group_count
		);

		static void FinalizeSwapchain(const SGPUTextureResource& present_texture);

		static void ReleaseSwapchainResources();
	};
}