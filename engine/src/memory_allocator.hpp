// memory_allocator.hpp

#pragma once

#include "types.hpp"

namespace triton
{
    struct SMemoryAllocatorBin
    {
        types::usize max_block_byte_size;
        types::u8* blocks;
        types::u8* occupancy;
    };

    template <typename T>
    struct SMemAlloc
    {
        types::u32 bin_index;
        types::u32 block_index;
        T* mem;

        static SMemAlloc Invalid()
        {
            SMemAlloc<T> invalid_mem_alloc;
            invalid_mem_alloc.mem = nullptr;

            return invalid_mem_alloc;
        }

        types::boolean IsValid() const
        {
            if (this->mem)
                return True;
            else
                return False;
        }
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

        template <typename T>
        static SMemAlloc<T> Allocate(types::usize byte_size)
        {
            if (!_bin_count || !byte_size)
                return SMemAlloc<T>::Invalid();

            if (byte_size > _max_object_byte_size)
            {
                SMemAlloc<T> mem_alloc;
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

                    SMemoryAllocation mem_alloc;
                    mem_alloc.bin_index = bin_index;
                    mem_alloc.block_index = block_index;
                    mem_alloc.mem = mem;

                    return mem_alloc;
                }
            }

            return SMemAlloc<T>::Invalid();
        }

        template <typename T>
        static void Deallocate(SMemAlloc<T>& mem_alloc)
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

            mem_alloc = SMemAlloc<T>::Invalid();
        }
    };
}