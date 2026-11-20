// memory_allocator.cpp

#include <cstdlib>
#include <cstring>
#include <cstddef>
#include "memory_allocator.hpp"
#include "log.hpp"

using namespace types;

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