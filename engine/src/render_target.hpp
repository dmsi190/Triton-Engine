// render_target.hpp

#pragma once

#include <vector>
#include "object.hpp"
#include "gpu_render_target_resource.hpp"
#include "texture.hpp"
#include "context.hpp"

namespace triton
{
	class cContext;

	class XRenderTarget : public iObject
	{
		TRITON_OBJECT(XRenderTarget)

		std::vector<XTexture*>		_colorAttachments;
		XTexture*					_depthAttachment;
		SGPURenderTargetResource	_gpuRenderTarget = SGPURenderTargetResource::Invalid();

	public:
		explicit XRenderTarget(
			const SMemAlloc& mem_alloc,
			cContext* context,
			const std::vector<XTexture*>& colorAttachments,
			XTexture* depthAttachment
		);

		~XRenderTarget() override;

		inline const SGPURenderTargetResource& GetGPUResource() const { return _gpuRenderTarget; }

		inline const std::vector<XTexture*>& GetColorAttachments() const { return _colorAttachments; }

		inline XTexture* GetDepthAttachment() const { return _depthAttachment; }

		inline const cVector3 GetColorAttachmentSize(types::usize attachmentIndex)
		{
			return _colorAttachments.at(attachmentIndex)->GetSize();
		}

		struct TGPULayout {};
	};
}