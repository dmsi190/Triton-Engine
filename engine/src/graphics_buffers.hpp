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
		inline static CGPUBufferResource	_main_indirect_draw_commands_buffer = CGPUBufferResource::Invalid();
		inline static CGPUBufferResource	_shadow_indirect_draw_commands_buffer = CGPUBufferResource::Invalid();
		inline static CGPUBufferResource	_main_command_render_group_indices_buffer = CGPUBufferResource::Invalid();
		inline static CGPUBufferResource	_shadow_command_render_group_indices_buffer = CGPUBufferResource::Invalid();
		inline static CGPUBufferResource	_visible_render_static_indices_buffer = CGPUBufferResource::Invalid();
		inline static CGPUBufferResource	_visible_render_dynamic_indices_buffer = CGPUBufferResource::Invalid();

	public:
		static void Initialize(cContext* context);

		static void Release();

		static const CGPUBufferResource& GetMainIndirectDrawCommandsBuffer() { return _main_indirect_draw_commands_buffer; }

		static const CGPUBufferResource& GetShadowIndirectDrawCommandsBuffer() { return _shadow_indirect_draw_commands_buffer; }

		static const CGPUBufferResource& GetMainCommandRenderGroupIndicesBuffer() { return _main_command_render_group_indices_buffer; }

		static const CGPUBufferResource& GetShadowCommandRenderGroupIndicesBuffer() { return _shadow_command_render_group_indices_buffer; }

		static const CGPUBufferResource& GetVisibleRenderStaticIndicesBuffer() { return _visible_render_static_indices_buffer; }

		static const CGPUBufferResource& GetVisibleRenderDynamicIndicesBuffer() { return _visible_render_dynamic_indices_buffer; }
	};
}