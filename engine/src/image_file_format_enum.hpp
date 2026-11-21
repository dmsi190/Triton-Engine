// image_file_format_enum.hpp

#pragma once

#include "types.hpp"

namespace triton
{
    enum class EImageFileFormat : types::dword
    {
        Unknown,
        PNG,
        DDS
    };
}