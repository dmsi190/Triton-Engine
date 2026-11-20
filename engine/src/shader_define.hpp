// shader_define.hpp

#pragma once

#include <string>
#include "types.hpp"

namespace triton
{
    struct SShaderDefine final
    {
        SShaderDefine(const std::string& name, types::usize index)
            : name(name), index(index) {}

        std::string name;
        types::usize index;
    };
}