// creator.hpp

#pragma once

#include "memory_allocator.hpp"
#include "types.hpp"

namespace triton
{
	class CCreator final
	{
	public:
		template <typename T, typename... Args>
		static T* Create(Args&&... args)
		{
			SMemoryAllocation mem_alloc = CMemoryAllocator::Allocate(sizeof(T));

			T* object = (T*)mem_alloc.mem;

			new (object) T(mem_alloc, std::forward<Args>(args)...);

			return object;
		}

		template <typename T>
		static void Destroy(T* object)
		{
			object->~T();

			CMemoryAllocator::Deallocate(object->_mem_alloc);
		}
	};
}