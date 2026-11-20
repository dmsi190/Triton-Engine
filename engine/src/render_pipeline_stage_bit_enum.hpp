// render_pipeline_stage_bit_enum.hpp

#pragma once

#include "types.hpp"

namespace triton
{
	enum class ERenderPipelineStageBit : types::dword
	{
		Unknown = 0,
		TopOfPipe = 1,
		VertexInput = 2,
		Vertex = 4,
		Pixel = 8,
		Compute = 16,
		Transfer = 32
	};
}