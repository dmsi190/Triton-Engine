// render_command_proxy.hpp

#pragma once

#include "gpu_buffer_resource.hpp"
#include "gpu_shader_resource.hpp"
#include "shader_stage_bit_enum.hpp"
#include "shader_bytecode_files_struct.hpp"
#include "gpu_binding_group_layout_resource.hpp"
#include "gpu_binding_group_resource.hpp"
#include "object.hpp"

namespace triton
{
	class cContext;
	
	class CRenderCommandProxy final
	{
		TRITON_CLASS_NAME(CRenderCommandProxy)

		static cContext* _context;

	public:
		static void SetContext(cContext* context);

		static CGPUBufferResource CreateBuffer(EGPUBufferType type, types::usize byte_size);

		static CGPUShaderResource CreateShader(EShaderStageBit stage_mask, const SShaderBytecodeFiles& files);

		static CGPUBindingGroupLayoutResource CreateBindingGroupLayout(
			const std::vector<SBindingGroupBinding>& bindings
		);

		static CGPUBindingGroupResource CreateBindingGroup(
			types::u32 index,
			const CGPUBindingGroupLayoutResource& layout,
			const std::vector<SBindingGroupBinding>& buffers_to_bind,
			const std::vector<SBindingGroupBinding>& textures_to_bind
		);
	};
}