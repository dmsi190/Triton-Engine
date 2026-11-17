#pragma once

#include "vertex_buffer_format.hpp"
#include "types.hpp"

namespace triton
{
	struct SGeometryDrawInfo final
	{
		types::u32 index_count;
		types::u32 index_element_offset;
		types::s32 vertex_element_offset;
		types::u32 instance_offset;
	};

	struct SGeometryView final
	{
		SGeometryDrawInfo draw_info;
		types::u32 vertex_count = 0;
		const types::u8* vertices = nullptr;
		const types::u8* indices = nullptr;
		EVertexBufferFormat format = EVertexBufferFormat::Unknown;
	};
}