// geometry_storage.cpp

#include "geometry_storage.hpp"
#include "application.hpp"
#include "context.hpp"
#include "graphics_buffer_formats.hpp"
#include "engine.hpp"
#include "vertex.hpp"
#include "gpu_buffer_types.hpp"
#include "creator.hpp"

using namespace types;

triton::CGeometryStorage::CGeometryStorage(cContext* context) : CSubsystem(context)
{
    const SCapabilities& caps = _context->GetSubsystem<CEngine>()->GetApplication()->GetCapabilities();

    _rigid_vertex_buffer = CCreator::Create<XGPUBuffer>(
        _context,
        EGPUBufferType::Vertex,
        0,
        caps.staticVertexBufferSize,
        0
    );

    _skinned_vertex_buffer = CCreator::Create<XGPUBuffer>(
        _context,
        EGPUBufferType::Vertex,
        0,
        caps.skinnedVertexBufferSize,
        0
    );

    _rigid_index_buffer = CCreator::Create<XGPUBuffer>(
        _context,
        EGPUBufferType::Index,
        0,
        caps.staticIndexBufferSize,
        0
    );
    
    _skinned_index_buffer = CCreator::Create<XGPUBuffer>(
        _context,
        EGPUBufferType::Index,
        0,
        caps.skinnedIndexBufferSize,
        0
    );
}

triton::CGeometryStorage::~CGeometryStorage()
{
    CCreator::Destroy<XGPUBuffer>(_skinned_index_buffer);
    CCreator::Destroy<XGPUBuffer>(_rigid_index_buffer);
    CCreator::Destroy<XGPUBuffer>(_skinned_vertex_buffer);
    CCreator::Destroy<XGPUBuffer>(_rigid_vertex_buffer);
}

std::optional<triton::SGeometryView> triton::CGeometryStorage::Store(
    EVertexBufferFormat format,
    const u8* vertices,
    usize vertex_count,
    const u8* indices,
    usize index_count
)
{
    if (format == EVertexBufferFormat::Rigid_48)
    {
        const usize vertices_byte_size = sizeof(SRigidVertexGPULayout) * vertex_count;
        const usize indices_byte_size = sizeof(u32) * index_count;

        _rigid_vertex_buffer->Write(_rigid_vertex_buffer_size, vertices, vertices_byte_size);
        _rigid_index_buffer->Write(_rigid_index_buffer_size, indices, indices_byte_size);

        const usize vertex_element_offset = _rigid_vertex_buffer_size / sizeof(SRigidVertexGPULayout);
        const usize index_element_offset = _rigid_index_buffer_size / sizeof(u32);

        SGeometryView geometry;
        geometry.vertex_count = vertex_count;
        geometry.draw_info.index_count = index_count;
        geometry.draw_info.vertex_element_offset = vertex_element_offset;
        geometry.draw_info.index_element_offset = index_element_offset;
        geometry.vertices = vertices;
        geometry.indices = indices;
        geometry.format = format;

        _rigid_vertex_buffer_size += vertices_byte_size;
        _rigid_index_buffer_size += indices_byte_size;

        return geometry;
    }
    else if (format == EVertexBufferFormat::Skinned_80)
    {
        const usize vertices_byte_size = sizeof(SSkinnedVertexGPULayout) * vertex_count;
        const usize indices_byte_size = sizeof(u32) * index_count;

        _skinned_vertex_buffer->Write(_skinned_vertex_buffer_size, vertices, vertices_byte_size);
        _skinned_index_buffer->Write(_skinned_index_buffer_size, indices, indices_byte_size);

        const usize vertex_element_offset = _skinned_vertex_buffer_size / sizeof(SSkinnedVertexGPULayout);
        const usize index_element_offset = _skinned_index_buffer_size / sizeof(u32);

        SGeometryView geometry;
        geometry.vertex_count = vertex_count;
        geometry.draw_info.index_count = index_count;
        geometry.draw_info.vertex_element_offset = vertex_element_offset;
        geometry.draw_info.index_element_offset = index_element_offset;
        geometry.vertices = vertices;
        geometry.indices = indices;
        geometry.format = format;

        _skinned_vertex_buffer_size += vertices_byte_size;
        _skinned_index_buffer_size += indices_byte_size;

        return geometry;
    }
    
    return std::nullopt;
}