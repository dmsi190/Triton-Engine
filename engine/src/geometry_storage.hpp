// geometry_storage.hpp

#pragma once

#include <optional>
#include "object.hpp"
#include "geometry_view.hpp"
#include "vertex_buffer_format.hpp"
#include "gpu_buffer.hpp"
#include "subsystem.hpp"
#include "types.hpp"

namespace triton
{
    class XGPUBuffer;
    class cEngine;
    class cContext;

    class CGeometryStorage final : public CSubsystem
    {
        TRITON_CLASS_NAME(CGeometryStorage)

        XGPUBuffer*     _rigid_vertex_buffer;
        XGPUBuffer*     _skinned_vertex_buffer;
        XGPUBuffer*     _rigid_index_buffer;
        XGPUBuffer*     _skinned_index_buffer;
        types::usize    _rigid_vertex_buffer_size;
        types::usize    _skinned_vertex_buffer_size;
        types::usize    _rigid_index_buffer_size;
        types::usize    _skinned_index_buffer_size;

    public:
        explicit CGeometryStorage(cContext* context);
        ~CGeometryStorage();

        std::optional<triton::SGeometryView> Store(
            EVertexBufferFormat format,
            const types::u8* vertices,
            types::usize vertex_count,
            const types::u8* indices,
            types::usize index_count
        );

        inline XGPUBuffer* GetRigidVertexBuffer() const
        {
            return _rigid_vertex_buffer;
        }

        inline XGPUBuffer* GetSkinnedVertexBuffer() const
        {
            return _skinned_vertex_buffer;
        }

        inline XGPUBuffer* GetRigidIndexBuffer() const
        {
            return _rigid_index_buffer;
        }

        inline XGPUBuffer* GetSkinnedIndexBuffer() const
        {
            return _skinned_index_buffer;
        }
    };
}