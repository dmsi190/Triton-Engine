// render_image_aspect_enum.hpp

#pragma once

#include "types.hpp"

namespace triton
{
	enum class EImageAspectBit : types::dword
	{
		Unknown,
		Color = 1,
		Depth = 2
	};
}