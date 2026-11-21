// image_data_format_enum.hpp

#pragma once

#include "types.hpp"

namespace triton
{
	enum class EImageDataFormat : types::dword
	{
		Unknown,
		R8,
		RGBA8,
		RGBA8_SRGB
	};
}