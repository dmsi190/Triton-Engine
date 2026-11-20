// gpu_render_target_resource.hpp

#pragma once

#include <vector>
#include "gpu_resource.hpp"
#include "gpu_texture_resource.hpp"
#include "graphics_image_layout_enum.hpp"
#include "types.hpp"

namespace triton
{
    struct SGPURenderTargetResource : public SGPUResource
    {
        std::vector<SGPUTextureResource>    color_attachments;
        SGPUTextureResource                 depth_attachment = SGPUTextureResource::Invalid();

        explicit SGPURenderTargetResource(
            types::qword instance,
            types::qword view,
            const std::vector<SGPUTextureResource>& color_attachments,
            const SGPUTextureResource& depth_attachment
        ) : SGPUResource(instance, view),
            depth_attachment(depth_attachment)
        {
            for (types::usize i = 0; i < color_attachments.size(); i++)
                this->color_attachments.push_back(color_attachments[i]);
        }

        ~SGPURenderTargetResource() override = default;

        static SGPURenderTargetResource Invalid()
        {
            return SGPURenderTargetResource(
                0, 0, std::vector<SGPUTextureResource>(), SGPUTextureResource::Invalid()
            );
        }
    };
}