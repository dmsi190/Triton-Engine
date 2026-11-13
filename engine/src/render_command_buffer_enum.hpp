// render_command_buffer_enum.hpp

#pragma once

#include "types.hpp"

namespace triton
{
	enum class ECommandBuffer : types::dword
	{
		Unknown,
		Frame,
		Staging
	};
}