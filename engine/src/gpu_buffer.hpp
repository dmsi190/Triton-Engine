// gpu_buffer.hpp

#pragma once

#include "object.hpp"
#include "gpu_buffer_resource.hpp"

namespace triton
{
	class cContext;

	struct SGPUBufferFrame
	{
		types::usize begin_index;
		types::usize count;
	};

	class XGPUBuffer : public iObject
	{
		TRITON_OBJECT(XGPUBuffer)

		types::usize		_element_byte_size;
		types::usize		_element_count;
		types::usize		_reserve_capacity;
		SGPUBufferResource	_gpu_buffer = SGPUBufferResource::Invalid();
		types::usize		_staging_element_count;
		void*				_staging_buffer;
		types::u8*			_occupancy_buffer;
		types::boolean		_dirty_bit;
		types::u32			_slot;

	public:
		explicit XGPUBuffer(
			const SMemAlloc& mem_alloc,
			cContext* context,
			EGPUBufferType type,
			types::usize element_byte_size,
			types::usize byte_size,
			types::s32 slot
		);

		~XGPUBuffer() override;

		void Write(types::usize offset, const types::u8* data, types::usize byte_size);

		types::usize AcquireFreeStagingIndex();

		SGPUBufferFrame AcquireFreeStagingFrame(types::usize count);

		void ReleaseStagingIndex(types::usize index);

		void ReleaseStagingFrame(const SGPUBufferFrame& frame);

		void WriteStaging(
			types::usize index,
			types::usize element_local_byte_offset,
			const void* data,
			types::usize byte_size
		);

		void Flush();

		inline const SGPUBufferResource& GetGPUResource() const { return _gpu_buffer; }

	private:
		void ResizeBuffers(types::usize new_size);
	};
}