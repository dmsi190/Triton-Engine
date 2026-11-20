// render_pass.hpp

#pragma once

#include "object.hpp"
#include "render_pass_dispatch_enum.hpp"
#include "gpu_render_pass_resource.hpp"
#include "types.hpp"

namespace triton
{
    class cContext;

	struct IRenderPass : public iObject
	{
        TRITON_OBJECT(IRenderPass)

    protected:
        SGPURenderPassResource _gpuRenderPass = SGPURenderPassResource::Invalid();
        ERenderPassDispatch _dispatch;
        
    public:
        explicit IRenderPass(
            const SMemAlloc& mem_alloc,
            cContext* context,
            ERenderPassDispatch dispatch
        ) : iObject(mem_alloc, context), _dispatch(dispatch) {}

        ~IRenderPass() override = default;

        virtual void Render() = 0;
	};
}