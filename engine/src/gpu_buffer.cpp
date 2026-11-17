// gpu_buffer.cpp

#include <cstdlib>
#include <cstring>
#include "gpu_buffer.hpp"
#include "render_command_proxy.hpp"
#include "context.hpp"
#include "engine.hpp"
#include "application.hpp"

using namespace types;

triton::XGPUBuffer::XGPUBuffer(
    const SMemoryAllocation& mem_alloc,
    cContext* context,
    EGPUBufferType type,
    usize element_byte_size,
    usize byte_size,
    s32 slot
) : iObject(mem_alloc, context),
    _element_byte_size(element_byte_size),
    _element_count(byte_size / _element_byte_size),
    _reserve_capacity(
        _context->GetSubsystem<CEngine>()->
        GetApplication()->
        GetCapabilities().staging_buffer_reserve_capacity
    ),
    _staging_buffer(nullptr),
    _staging_element_count(0),
    _occupancy_buffer(nullptr),
    _dirty_bit(False),
    _slot(slot)
{
    _gpu_buffer = CRenderCommandProxy::CreateBuffer(type, byte_size);

    ResizeBuffers(_reserve_capacity);
}

triton::XGPUBuffer::~XGPUBuffer()
{
    if (_staging_buffer)
        std::free(_staging_buffer);

    CRenderCommandProxy::DestroyBuffer(_gpu_buffer);
}

void triton::XGPUBuffer::Write(usize offset, const u8* data, usize byte_size)
{
    CRenderCommandProxy::WriteBuffer(_gpu_buffer, offset, data, byte_size);
}

types::usize triton::XGPUBuffer::AcquireFreeStagingIndex()
{
    for (usize i = 0; i < _staging_element_count; ++i)
    {
        if (_occupancy_buffer[i] == 0)
        {
            _occupancy_buffer[i] = 1;

            return i;
        }
    }

    const usize old_staging_element_count = _staging_element_count;

    ResizeBuffers(old_staging_element_count + _reserve_capacity);

    _occupancy_buffer[old_staging_element_count] = 1;

    return old_staging_element_count;
}

void triton::XGPUBuffer::ReleaseStagingIndex(types::usize index)
{
    _occupancy_buffer[index] = 0;

    usize cut_count = 0;
    
    for (usize i = _staging_element_count; i > 0; --i)
    {
        if (_occupancy_buffer[i - 1] == 1)
            break;

        ++cut_count;
    }

    if (cut_count >= _reserve_capacity)
        ResizeBuffers(_staging_element_count - cut_count);
}

void triton::XGPUBuffer::WriteStaging(types::usize index, const void* element)
{
    void* write_ptr = (u8*)_staging_buffer + index * _element_byte_size;

    std::memcpy(write_ptr, element, _element_byte_size);

    _dirty_bit = True;
}

void triton::XGPUBuffer::Flush()
{
    if (_dirty_bit == True)
        Write(0, (const u8*)_staging_buffer, _staging_element_count * _element_byte_size);

    _dirty_bit = True;
}

void triton::XGPUBuffer::ResizeBuffers(usize new_size)
{
    if (new_size == _staging_element_count)
        return;

    const usize prev_staging_element_count = _staging_element_count;

    _staging_element_count = new_size;

    const usize new_staging_buffer_byte_size = _staging_element_count * _element_byte_size;

    if (!_staging_buffer)
        _staging_buffer = std::malloc(new_staging_buffer_byte_size);
    else
        _staging_buffer = std::realloc(_staging_buffer, new_staging_buffer_byte_size);

    const usize new_occupancy_buffer_byte_size = _staging_element_count * sizeof(u8);

    if (!_occupancy_buffer)
        _occupancy_buffer = (u8*)std::malloc(new_occupancy_buffer_byte_size);
    else
        _occupancy_buffer = (u8*)std::realloc(_occupancy_buffer, new_occupancy_buffer_byte_size);

    if (new_size > _staging_element_count)
        std::memset(&_occupancy_buffer[prev_staging_element_count], 0, new_size - prev_staging_element_count);
}