// render_access_bit_enum.hpp

#pragma once

#include "types.hpp"

namespace triton
{
	enum class EAccessBit : types::dword
	{
		Unknown,
		ShaderRead,
		TransferWrite
	};
}