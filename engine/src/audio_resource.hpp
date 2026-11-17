// audio_resource.hpp

#pragma once

#include "types.hpp"

namespace triton
{
    struct SAudioResource
    {
        types::qword source = 0;
        types::qword buffer = 0;
    };
}