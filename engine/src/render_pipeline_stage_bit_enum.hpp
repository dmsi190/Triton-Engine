// render_pipeline_stage_bit_enum.hpp

#pragma once

#include "types.hpp"

namespace triton
{
	enum class EPipelineStageBit : types::dword
	{
		Unknown,
		Vertex = 1,
		Pixel = 2,
		Compute = 4,
		Transfer = 8
	};
}