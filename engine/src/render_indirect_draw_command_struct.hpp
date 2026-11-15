// render_indirect_draw_command_struct.hpp

#pragma once

#include "types.hpp"

namespace triton
{
    struct SIndirectDrawCommand final
    {
        types::u32 indexCount;
        types::u32 instanceCount;
        types::u32 firstIndex;
        types::s32 vertexOffset;
        types::u32 firstInstance;
    };
}