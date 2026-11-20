// texture_dimensions.hpp

#pragma once

#include "types.hpp"

namespace triton
{
	enum class ETextureDimension : types::dword
	{
		Unknown,
		Texture2D,
		Texture2DArray
	};
}