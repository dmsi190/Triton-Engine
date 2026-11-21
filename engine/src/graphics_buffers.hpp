// graphics_buffers.hpp

#pragma once

#include "object.hpp"
#include "gpu_buffer_resource.hpp"

namespace triton
{
	class cContext;
	class XGPUBuffer;

	class CGraphicsBuffers final
	{
		inline static cContext*				_context;
		inline static XGPUBuffer*			_static_instance_buffer;
		inline static XGPUBuffer*			_dynamic_instance_buffer;
		inline static SGPUBufferResource	_main_indirect_draw_commands_buffer = SGPUBufferResource::Invalid();
		inline static SGPUBufferResource	_shadow_indirect_draw_commands_buffer = SGPUBufferResource::Invalid();
		inline static SGPUBufferResource	_main_command_render_group_indices_buffer = SGPUBufferResource::Invalid();
		inline static SGPUBufferResource	_shadow_command_render_group_indices_buffer = SGPUBufferResource::Invalid();
		inline static SGPUBufferResource	_visible_render_static_indices_buffer = SGPUBufferResource::Invalid();
		inline static SGPUBufferResource	_visible_render_dynamic_indices_buffer = SGPUBufferResource::Invalid();

	public:
		static void Initialize(cContext* context);

		static void Release();

		static XGPUBuffer* GetStaticInstanceBuffer() { return _static_instance_buffer; }

		static XGPUBuffer* GetDynamicInstanceBuffer() { return _dynamic_instance_buffer; }

		static const SGPUBufferResource& GetMainIndirectDrawCommandsBuffer() { return _main_indirect_draw_commands_buffer; }

		static const SGPUBufferResource& GetShadowIndirectDrawCommandsBuffer() { return _shadow_indirect_draw_commands_buffer; }

		static const SGPUBufferResource& GetMainCommandRenderGroupIndicesBuffer() { return _main_command_render_group_indices_buffer; }

		static const SGPUBufferResource& GetShadowCommandRenderGroupIndicesBuffer() { return _shadow_command_render_group_indices_buffer; }

		static const SGPUBufferResource& GetVisibleRenderStaticIndicesBuffer() { return _visible_render_static_indices_buffer; }

		static const SGPUBufferResource& GetVisibleRenderDynamicIndicesBuffer() { return _visible_render_dynamic_indices_buffer; }
	};
}