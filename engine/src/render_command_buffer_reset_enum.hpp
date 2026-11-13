// render_command_buffer_reset_enum.hpp

#pragma once

#include "types.hpp"

namespace triton
{
	enum class ECommandBufferReset : types::dword
	{
		Clear,
		Begin,
		End
	};
}