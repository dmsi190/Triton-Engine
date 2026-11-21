// graphics_image_layout.hpp

#pragma once

#include "types.hpp"

namespace triton
{
	enum class EGraphicsImageLayout : types::dword
	{
		Undefined,
		ColorAttachment,
		DepthStencilAttachment,
		ShaderRead,
		Present
	};
}