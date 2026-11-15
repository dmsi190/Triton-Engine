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
		types::u32 _vertexCount = 0;
		types::u8* _vertexData = nullptr;
		types::u8* _indexData = nullptr;
		EVertexBufferFormat _format = EVertexBufferFormat::Unknown;
	};
}