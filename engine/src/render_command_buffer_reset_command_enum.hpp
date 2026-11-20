// render_command_buffer_reset_command_enum.hpp

#pragma once

#include "types.hpp"

namespace triton
{
	enum class ERenderCommandBufferResetCommand : types::dword
	{
		Clear,
		Begin,
		End
	};
}