// geometry_storage.cpp

#include "geometry_storage.hpp"
#include "application.hpp"
#include "context.hpp"
#include "engine.hpp"
#include "gpu_buffer_types.hpp"
#include "creator.hpp"
#include "graphics_buffers.hpp"
#include "vertex.hpp"

using namespace types;

std::optional<triton::SGeometryView> triton::XGeometryStorage::Store(
    EVertexBufferFormat format,
    const u8* vertices,
    usize vertex_count,
    const u8* indices,
    usize index_count
)
{
    if (format == EVertexBufferFormat::Rigid)
    {
        const usize vertices_byte_size = sizeof(SRigidVertexGPULayout) * vertex_count;
        const usize indices_byte_size = sizeof(u32) * index_count;

        CGraphicsBuffers::GetRigidVertexBuffer()->Write(_rigid_vertex_buffer_cursor, vertices, vertices_byte_size);
        CGraphicsBuffers::GetRigidIndexBuffer()->Write(_rigid_index_buffer_cursor, indices, indices_byte_size);

        const usize vertex_element_offset = _rigid_vertex_buffer_cursor / sizeof(SRigidVertexGPULayout);
        const usize index_element_offset = _rigid_index_buffer_cursor / sizeof(u32);

        SGeometryView geometry;
        geometry.vertex_count = vertex_count;
        geometry.draw_info.index_count = index_count;
        geometry.draw_info.vertex_element_offset = vertex_element_offset;
        geometry.draw_info.index_element_offset = index_element_offset;
        geometry.vertices = vertices;
        geometry.indices = indices;
        geometry.format = format;

        _rigid_vertex_buffer_cursor += vertices_byte_size;
        _rigid_index_buffer_cursor += indices_byte_size;

        return geometry;
    }
    else if (format == EVertexBufferFormat::Skinned)
    {
        const usize vertices_byte_size = sizeof(SSkinnedVertexGPULayout) * vertex_count;
        const usize indices_byte_size = sizeof(u32) * index_count;

        CGraphicsBuffers::GetSkinnedVertexBuffer()->Write(_skinned_vertex_buffer_cursor, vertices, vertices_byte_size);
        CGraphicsBuffers::GetSkinnedIndexBuffer()->Write(_skinned_index_buffer_cursor, indices, indices_byte_size);

        const usize vertex_element_offset = _skinned_vertex_buffer_cursor / sizeof(SSkinnedVertexGPULayout);
        const usize index_element_offset = _skinned_index_buffer_cursor / sizeof(u32);

        SGeometryView geometry;
        geometry.vertex_count = vertex_count;
        geometry.draw_info.index_count = index_count;
        geometry.draw_info.vertex_element_offset = vertex_element_offset;
        geometry.draw_info.index_element_offset = index_element_offset;
        geometry.vertices = vertices;
        geometry.indices = indices;
        geometry.format = format;

        _skinned_vertex_buffer_cursor += vertices_byte_size;
        _skinned_index_buffer_cursor += indices_byte_size;

        return geometry;
    }
    
    return std::nullopt;
}