// gpu_buffer.hpp

#pragma once

#include "object.hpp"
#include "gpu_buffer_resource.hpp"

namespace triton
{
	class cContext;

	class XGPUBuffer : public iObject
	{
		TRITON_CLASS_NAME(XGPUBuffer)

		types::usize _element_byte_size;
		types::usize _element_count;
		types::usize _reserve_capacity;
		CGPUBufferResource _gpu_buffer = CGPUBufferResource::Invalid();
		types::usize _staging_element_count;
		void* _staging_buffer;
		types::u8* _occupancy_buffer;
		types::boolean _dirty_bit;

	public:
		explicit XGPUBuffer(
			const SMemoryAllocation& mem_alloc,
			cContext* context,
			EGPUBufferType type,
			types::usize element_byte_size,
			const types::u8* data,
			types::usize byte_size,
			types::s32 slot
		);

		~XGPUBuffer() override;

		void Write(types::usize offset, const types::u8* data, types::usize byte_size);

		types::usize AcquireFreeStagingIndex();

		void ReleaseStagingIndex(types::usize index);

		void WriteStaging(types::usize index, const void* element);

		void Flush();

		inline const CGPUBufferResource& GetGPUResource() const { return _gpu_buffer; }

	private:
		void ResizeBuffers(types::usize new_size);
	};
}