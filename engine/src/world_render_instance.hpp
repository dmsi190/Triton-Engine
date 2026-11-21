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
    class XWorldObject;

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
        types::boolean              active;
        XWorldObject*               connected_object;

        explicit SRenderInstance(
            types::s32 prev_node_index,
            types::s32 next_node_index,
            types::u32 instance_buffer_index,
            XWorldObject* connected_object
        ) : prev_node_index(prev_node_index),
            next_node_index(next_node_index),
            instance_buffer_index(instance_buffer_index),
            active(types::True),
            connected_object(connected_object) {}

        ~SRenderInstance() = default;

        void SetMaterialIndex(
            ERenderInstanceMotionType motion_type,
            types::s32 index
        ) const;

        void SetWorldMatrix(
            ERenderInstanceMotionType motion_type,
            const cMatrix4& world_matrix
        ) const;

        void WriteStaging(
            ERenderInstanceMotionType motion_type,
            types::usize element_local_byte_offset,
            const void* data,
            types::usize byte_size
        ) const;
	};
}