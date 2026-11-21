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

    class XGeometryStorage final : public XSubsystem
    {
        TRITON_OBJECT(XGeometryStorage)

        types::usize    _rigid_vertex_buffer_cursor;
        types::usize    _skinned_vertex_buffer_cursor;
        types::usize    _rigid_index_buffer_cursor;
        types::usize    _skinned_index_buffer_cursor;

    public:
        explicit XGeometryStorage(const SMemAlloc& mem_alloc, cContext* context)
            : XSubsystem(mem_alloc, context),
              _rigid_vertex_buffer_cursor(0),
              _skinned_vertex_buffer_cursor(0),
              _rigid_index_buffer_cursor(0),
              _skinned_index_buffer_cursor(0) {}
        
        ~XGeometryStorage() override = default;

        std::optional<triton::SGeometryView> Store(
            EVertexBufferFormat format,
            const types::u8* vertices,
            types::usize vertex_count,
            const types::u8* indices,
            types::usize index_count
        );
    };
}