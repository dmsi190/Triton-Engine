// gpu_texture.hpp

#pragma once

#include "math.hpp"
#include "texture_formats.hpp"
#include "texture_dimensions.hpp"
#include "gpu_resource.hpp"
#include "gpu_buffer_resource.hpp"

namespace triton
{
    struct SGPUTextureResource : public SGPUResource
    {
        types::qword        sampler = 0;
        types::qword        device_memory = 0;
        cVector3            size = cVector3(0.0f);
        ETextureFormat      format = ETextureFormat::Unknown;
        types::dword        usage_mask = 0;
        ETextureDimension   dimension = ETextureDimension::Unknown;
        types::s32          slot = -1;

        explicit SGPUTextureResource(
            types::qword instance,
            types::qword view_instance,
            types::qword sampler,
            types::qword device_memory,
            ETextureFormat format,
            types::dword usage_mask,
            ETextureDimension dimension,
            const cVector3& size,
            types::s32 slot
        ) : SGPUResource(instance, view_instance),
            sampler(sampler),
            device_memory(device_memory),
            format(format),
            usage_mask(usage_mask),
            dimension(dimension),
            size(size),
            slot(slot) {}

        ~SGPUTextureResource() override = default;

        static SGPUTextureResource Invalid()
        {
            return SGPUTextureResource(
                0,
                0,
                0,
                0,
                ETextureFormat::Unknown,
                0,
                ETextureDimension::Unknown,
                cVector3(0.0f),
                0
            );
        }
    };
}