// render_command_buffer_type_enum.hpp

#pragma once

#include "types.hpp"

namespace triton
{
	enum class ERenderCommandBufferType : types::dword
	{
		Unknown,
		Frame,
		Staging
	};
}