// memory_allocator.hpp

#pragma once

#include "types.hpp"

namespace triton
{
    struct SMemAlloc
    {
        types::u32 bin_index;
        types::u32 block_index;
        types::u8* mem = nullptr;

        static SMemAlloc Invalid();

        types::boolean IsValid() const;
    };

    struct SMemoryAllocatorBin
    {
        types::usize max_block_byte_size;
        types::u8* blocks;
        types::u8* occupancy;
    };

    class CMemoryAllocator
    {
        inline static types::usize _bin_count = 0;
        inline static types::usize _bin_capacity;
        inline static types::usize _bin_mem_step;
        inline static types::usize _max_object_byte_size;
        inline static SMemoryAllocatorBin* _bins;

    public:
        static constexpr types::usize k_max_size = (1ull << 32ull) - 1ull;

        static void Initialize(types::usize bin_count, types::usize bin_capacity, types::usize bin_mem_step);

        static void Release();

        static SMemAlloc Allocate(types::usize byte_size);

        static void Deallocate(SMemAlloc& mem_alloc);
    };
}