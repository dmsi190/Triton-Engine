// render_binding_group_binding_struct.hpp

#pragma once

#include "gpu_resource.hpp"
#include "shader_stage_bit_enum.hpp"
#include "types.hpp"

namespace triton
{
	enum class ERenderBindingGroupBindingType : types::dword
	{
		Unknown,
		StorageBuffer,
		TextureSampler
	};

	struct SRenderBindingGroupBinding final
	{
		types::usize index;
		ERenderBindingGroupBindingType type;
		EShaderStageBit shaderStageMask;
		SGPUResource* resource;

		SRenderBindingGroupBinding(
			types::usize index,
			ERenderBindingGroupBindingType type,
			EShaderStageBit shader_stage_mask,
			SGPUResource* resource
		) : index(index), type(type), shaderStageMask(shader_stage_mask), resource(resource) {}
	};
}