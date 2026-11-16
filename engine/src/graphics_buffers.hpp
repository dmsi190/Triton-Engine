// graphics_buffers.hpp

#pragma once

#include "object.hpp"
#include "gpu_buffer_resource.hpp"

namespace triton
{
	class cContext;

	class CGraphicsBuffers final
	{
		TRITON_CLASS_NAME(CGraphicsBuffers)

		inline static cContext*				_context;
		inline static CGPUBufferResource	_indirect_draw_commands_buffer = CGPUBufferResource::Invalid();
		inline static CGPUBufferResource	_motion_type_per_command_buffer = CGPUBufferResource::Invalid();

	public:
		static void Initialize(cContext* context);

		static void Release();

		static const CGPUBufferResource& GetIndirectDrawCommandsBuffer() { return _indirect_draw_commands_buffer; }

		static const CGPUBufferResource& GetMotionTypePerCommandBuffer() { return _motion_type_per_command_buffer; }
	};
}