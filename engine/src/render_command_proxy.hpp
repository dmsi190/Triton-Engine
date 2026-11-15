// render_command_proxy.hpp

#pragma once

#include "gpu_buffer_resource.hpp"
#include "object.hpp"

namespace triton
{
	class cContext;
	
	class CRenderCommandProxy final
	{
		TRITON_CLASS_NAME(CRenderCommandProxy)

		static cContext* _context;

	public:
		static void SetContext(cContext* context);

		static CGPUBufferResource CreateBuffer(EGPUBufferType type, types::usize byte_size);
	};
}