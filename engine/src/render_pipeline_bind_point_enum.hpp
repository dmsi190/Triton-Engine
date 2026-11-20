// render_pipeline_bind_point_enum.hpp

#pragma once

#include "types.hpp"

namespace triton
{
	enum class ERenderPipelineBindPoint : types::dword
	{
		Unknown,
		Graphics,
		Compute
	};
}