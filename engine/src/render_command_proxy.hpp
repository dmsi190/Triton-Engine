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
#include "rasterizer_state.hpp"
#include "render_primitive_topology_enum.hpp"
#include "vertex_buffer_format.hpp"
#include "object.hpp"

namespace triton
{
	class cContext;
	
	class CRenderCommandProxy final
	{
		TRITON_CLASS_NAME(CRenderCommandProxy)

		inline static cContext* _context;

	public:
		static void SetContext(cContext* context);

		static CGPUBufferResource CreateBuffer(EGPUBufferType type, types::usize byte_size);

		static void WriteBuffer(
			const CGPUBufferResource& buffer,
			types::usize offset,
			const types::u8* data,
			types::usize byte_size
		);

		static void DestroyBuffer(CGPUBufferResource& buffer);

		static CGPUShaderResource CreateShader(EShaderStageBit stage_mask, const SShaderBytecodeFiles& files);

		static void DestroyShader(CGPUShaderResource& shader);

		static CGPUBindingGroupLayoutResource CreateBindingGroupLayout(
			const std::vector<SBindingGroupBinding>& bindings
		);

		static void DestroyBindingGroupLayout(CGPUBindingGroupLayoutResource& binding_group_layout);

		static CGPUBindingGroupResource CreateBindingGroup(
			types::u32 index,
			const CGPUBindingGroupLayoutResource& layout,
			const std::vector<SBindingGroupBinding>& buffers_to_bind,
			const std::vector<SBindingGroupBinding>& textures_to_bind
		);

		static void DestroyBindingGroup(CGPUBindingGroupResource& binding_group);

		static CGPUPipelineResource CreatePipeline(
			const CGPUShaderResource& shader,
			const std::vector<CGPUBindingGroupLayoutResource>& bindingGroupLayouts,
			types::boolean bUsePushConstants,
			const std::optional<std::reference_wrapper<SViewport>> viewport = std::nullopt,
			std::optional<std::reference_wrapper<CGPURenderTargetResource>> renderTarget = std::nullopt,
			const std::optional<std::reference_wrapper<CGPURenderPassResource>> renderPass = std::nullopt,
			EPrimitiveTopology primitiveTopology = EPrimitiveTopology::Unknown,
			EVertexBufferFormat vertexBufferFormat = EVertexBufferFormat::Unknown
		);

		static void DestroyPipeline(CGPUPipelineResource& pipeline);

		static void Dispatch(
			const CGPUPipelineResource& pipeline,
			const std::vector<CGPUBindingGroupResource>& binding_groups,
			const cVector3& work_group_count
		);

		static void FinalizeSwapchain(const SGPUTextureResource& present_texture);

		static void ReleaseSwapchainResources();
	};
}