// render_primitive_topology_enum.hpp

#pragma once

#include "types.hpp"

namespace triton
{
	enum class ERenderPrimitiveTopology : types::dword
	{
		Unknown,
		TriangleList,
		TriangleStrip
	};
}