// memory_allocator.cpp

#include <cstdlib>
#include <cstring>
#include <cstddef>
#include "memory_allocator.hpp"
#include "log.hpp"

using namespace types;

triton::SMemAlloc triton::SMemAlloc::Invalid()
{
	SMemAlloc invalid_mem_alloc;
	invalid_mem_alloc.mem = nullptr;

	return invalid_mem_alloc;
}

boolean triton::SMemAlloc::IsValid() const
{
	if (this->mem)
		return True;
	else
		return False;
}

void triton::CMemoryAllocator::Initialize(usize bin_count, usize bin_capacity, usize bin_mem_step)
{
	if (_bin_count)
		return;

	if (!bin_count || !bin_capacity || !bin_mem_step || bin_count > k_max_size || bin_capacity > k_max_size)
	{
		Print("Error: failed to initialize memory allocator");
		return;
	}

	_bin_count = bin_count;
	_bin_capacity = bin_capacity;
	_bin_mem_step = bin_mem_step;

	_max_object_byte_size = _bin_count * _bin_mem_step;

	_bins = (SMemoryAllocatorBin*)std::malloc(_bin_count * sizeof(SMemoryAllocatorBin));

	const usize alignment = alignof(std::max_align_t);

	for (usize i = 0; i < _bin_count; ++i)
	{
		const usize aligned_block_size =
			((_bin_mem_step + alignment - 1) / alignment) * alignment;

		_bins[i].max_block_byte_size = (i + 1) * aligned_block_size;

		const usize blocks_byte_size = _bin_capacity * _bins[i].max_block_byte_size;
		_bins[i].blocks = (u8*)std::malloc(blocks_byte_size);
		std::memset(_bins[i].blocks, 0, blocks_byte_size);

		const usize occupancy_byte_size = _bin_capacity;
		_bins[i].occupancy = (u8*)std::malloc(occupancy_byte_size);
		std::memset(_bins[i].occupancy, 0, occupancy_byte_size);
	}
}

void triton::CMemoryAllocator::Release()
{
	if (!_bin_count)
		return;

	if (_bins)
	{
		for (usize i = 0; i < _bin_count; ++i)
		{
			if (_bins[i].occupancy)
				std::free(_bins[i].occupancy);

			if (_bins[i].blocks)
				std::free(_bins[i].blocks);
		}

		std::free(_bins);

		_bins = nullptr;
	}

	_max_object_byte_size = _bin_mem_step = _bin_capacity = _bin_count = 0;
}

triton::SMemAlloc triton::CMemoryAllocator::Allocate(types::usize byte_size)
{
	if (!_bin_count || !byte_size)
		return SMemAlloc::Invalid();

	if (byte_size > _max_object_byte_size)
	{
		SMemAlloc mem_alloc;
		mem_alloc.bin_index = k_max_size;
		mem_alloc.block_index = k_max_size;
		mem_alloc.mem = (u8*)std::malloc(byte_size);

		return mem_alloc;
	}

	const usize bin_index = (byte_size - 1) / _bin_mem_step;

	const SMemoryAllocatorBin& bin = _bins[bin_index];

	for (usize block_index = 0, block_byte_cursor = 0, occupancy_byte_cursor = 0;
		block_index < _bin_capacity;
		++block_index, block_byte_cursor += bin.max_block_byte_size, ++occupancy_byte_cursor)
	{
		u8* mem = (u8*)bin.blocks + block_byte_cursor;
		u8* is_occupied_ptr = (u8*)bin.occupancy + occupancy_byte_cursor;

		if (*is_occupied_ptr == 0)
		{
			*is_occupied_ptr = 1;

			SMemAlloc mem_alloc;
			mem_alloc.bin_index = bin_index;
			mem_alloc.block_index = block_index;
			mem_alloc.mem = mem;

			return mem_alloc;
		}
	}

	return SMemAlloc::Invalid();
}

void triton::CMemoryAllocator::Deallocate(SMemAlloc& mem_alloc)
{
	if (!_bin_count || !mem_alloc.IsValid())
		return;

	if (mem_alloc.bin_index == k_max_size &&
		mem_alloc.block_index == k_max_size)
	{
		std::free(mem_alloc.mem);
		return;
	}

	const SMemoryAllocatorBin& bin = _bins[mem_alloc.bin_index];

	u8* is_occupied_ptr = (u8*)bin.occupancy + mem_alloc.block_index;

	*is_occupied_ptr = 0;

	mem_alloc = SMemAlloc::Invalid();
}