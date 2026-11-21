// world_render_instance.hpp

#pragma once

#include "object.hpp"
#include "material.hpp"
#include "math.hpp"
#include "skin.hpp"
#include "render_instance_motion_type.hpp"
#include "types.hpp"

namespace triton
{
    struct SRenderInstanceGPULayout final
    {
        types::f32 use2D = 0.0f;
        types::s32 materialIndex = -1;
        types::s32 skinnedBoneBufferOffset = -1;
        types::dword propertyBits = 0;
        cMatrix4 world = cMatrix4(1.0f);
    };

	struct SRenderInstance
	{
        types::s32                  prev_node_index;
        types::s32                  next_node_index;
        types::u32                  instance_buffer_index;

        explicit SRenderInstance(
            types::s32 prev_node_index,
            types::s32 next_node_index,
            types::u32 instance_buffer_index
        ) : prev_node_index(prev_node_index),
            next_node_index(next_node_index),
            instance_buffer_index(instance_buffer_index) {}

        ~SRenderInstance() = default;

        inline void SetMaterial(const SHandle& material)
        {
            _material = material;
            WriteToStaging();
        }

        inline void SetSkin(const SHandle& skin)
        {
            XSkin& sk = *_context->GetPool<CSkinPool>()->Get(skin);
            _skinnedBoneBufferOffset = sk.GetSkinnedBoneBufferOffset();
            WriteToStaging();
        }

        inline void SetWorldPosition(const cVector3& worldPosition) { _transform.SetPosition(worldPosition); }

        inline void SetRotation(const cVector3& rotation) { _transform.SetRotation(rotation); }

        inline void SetScale(const cVector3& scale) { _transform.SetScale(scale); }
        
        void WriteToStaging();
	};
}