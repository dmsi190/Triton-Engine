// vertex_buffer_format_enum.hpp

#pragma once

#include "types.hpp"

namespace triton
{
	enum class EVertexBufferFormat : types::dword
	{
		Unknown,
		Rigid,
		Skinned
	};
}