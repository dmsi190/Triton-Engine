// render_image_layout_enum.hpp

#pragma once

#include "types.hpp"

namespace triton
{
	enum class EImageLayout : types::dword
	{
		Unknown,
		ShaderRead,
		TransferDst
	};
}