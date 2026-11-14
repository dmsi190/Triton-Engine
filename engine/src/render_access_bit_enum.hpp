// render_access_bit_enum.hpp

#pragma once

#include "types.hpp"

namespace triton
{
	enum class EAccessBit : types::dword
	{
		None = 0,
		IndexRead = 1,
		ShaderRead = 2,
		ShaderWrite = 4,
		TransferWrite = 8
	};
}