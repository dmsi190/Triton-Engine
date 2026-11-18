// gpu_buffer_resource.hpp

#pragma once

#include "gpu_resource.hpp"
#include "gpu_buffer_types.hpp"
#include "types.hpp"

namespace triton
{
    struct SGPUBufferResource : public SGPUResource
    {
        types::qword    device_memory = 0;
        EGPUBufferType  type = EGPUBufferType::Unknown;
        types::usize    byte_size = 0;
        types::s32      slot = -1;

        explicit SGPUBufferResource(
            types::qword instance,
            types::qword view_instance,
            types::qword device_memory,
            EGPUBufferType type,
            types::usize byte_size,
            types::s32 slot
        ) :
            SGPUResource(instance, view_instance),
            device_memory(device_memory),
            type(type),
            byte_size(byte_size),
            slot(slot) {}

        ~SGPUBufferResource() override = default;

        static SGPUBufferResource Invalid()
        {
            return SGPUBufferResource(
                0, 0, 0, EGPUBufferType::Unknown, 0, 0
            );
        }
    };
}