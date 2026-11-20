// render_command_buffer_reset_option_enum.hpp

#pragma once

#include "types.hpp"

namespace triton
{
	enum class ERenderCommandBufferResetOption : types::dword
	{
		Clear,
		Begin,
		End
	};
}