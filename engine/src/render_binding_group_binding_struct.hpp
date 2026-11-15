// render_binding_group_binding_struct.hpp

#pragma once

#include "gpu_resource.hpp"
#include "shader_stage_bit_enum.hpp"
#include "types.hpp"

namespace triton
{
	enum class EBindingGroupBindingType : types::dword
	{
		Unknown,
		StorageBuffer,
		TextureSampler
	};

	struct SBindingGroupBinding final
	{
		types::usize index;
		EBindingGroupBindingType type;
		EShaderStageBit shaderStageMask;
		CGPUResource* resource;

		SBindingGroupBinding(
			types::usize index,
			EBindingGroupBindingType type,
			EShaderStageBit shader_stage_mask,
			CGPUResource* resource
		) : index(index), type(type), shaderStageMask(shader_stage_mask), resource(resource) {}
	};
}