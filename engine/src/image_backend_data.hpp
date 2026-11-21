// image_resource.hpp

#pragma once

#include "math.hpp"
#include "image_data_format_enum.hpp"
#include "types.hpp"

namespace triton
{
	struct SImageResource
	{
		SMemAlloc data;
		types::usize dataByteSize = 0;
		EImageDataFormat dataFormat = EImageDataFormat::Unknown;
		cVector3 size = cVector3(0.0f);
	};
}