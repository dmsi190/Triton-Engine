// render_image_aspect_enum.hpp

#pragma once

#include "types.hpp"

namespace triton
{
	enum class ERenderImageAspectBit : types::dword
	{
		Unknown = 0,
		Color = 1,
		Depth = 2
	};
}