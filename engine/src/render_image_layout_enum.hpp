// render_image_layout_enum.hpp

#pragma once

#include "types.hpp"

namespace triton
{
	enum class ERenderImageLayout : types::dword
	{
		Undefined,
		ShaderRead,
		TransferDst
	};
}