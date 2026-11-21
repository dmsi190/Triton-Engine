// world_render_instance.cpp

#include <cstddef>
#include "world_render_instance.hpp"
#include "gpu_buffer.hpp"
#include "graphics_buffers.hpp"

using namespace types;

void triton::SRenderInstance::SetMaterialIndex(
	ERenderInstanceMotionType motion_type,
	types::s32 index
)
{
	WriteStaging(
		motion_type,
		offsetof(SRenderInstanceGPULayout, materialIndex),
		(const void*)&index,
		sizeof(s32)
	);
}

void triton::SRenderInstance::SetWorldMatrix(
	ERenderInstanceMotionType motion_type,
	const cMatrix4& world_matrix
)
{
	WriteStaging(
		motion_type,
		offsetof(SRenderInstanceGPULayout, world),
		(const void*)&world_matrix,
		sizeof(cMatrix4)
	);
}

void triton::SRenderInstance::WriteStaging(
	ERenderInstanceMotionType motion_type,
	usize element_local_byte_offset,
	const void* data,
	usize byte_size
)
{
	XGPUBuffer* buffer = nullptr;

	if (motion_type == ERenderInstanceMotionType::Static)
		buffer = CGraphicsBuffers::GetStaticInstanceBuffer();
	else if (motion_type == ERenderInstanceMotionType::Dynamic)
		buffer = CGraphicsBuffers::GetDynamicInstanceBuffer();

	buffer->WriteStaging(
		instance_buffer_index,
		element_local_byte_offset,
		data,
		byte_size
	);
}