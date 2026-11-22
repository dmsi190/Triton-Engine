// render_target.cpp

#include <functional>
#include "render_target.hpp"
#include "thread_guard.hpp"
#include "context.hpp"
#include "engine.hpp"
#include "synchronization.hpp"
#include "render_command_proxy.hpp"

using namespace types;

triton::XRenderTarget::XRenderTarget(
    const SMemAlloc& mem_alloc,
    cContext* context,
    const std::vector<XTexture*>& colorAttachments,
    XTexture* depthAttachment
) : iObject(mem_alloc, context),
    _colorAttachments(colorAttachments),
    _depthAttachment(depthAttachment)
{
    CThreadGuard::AssertMain();

    std::vector<std::reference_wrapper<SGPUTextureResource>> gpuColorAttachments;
    for (usize i = 0; i < colorAttachments.size(); i++)
        gpuColorAttachments.push_back(
            (SGPUTextureResource&)colorAttachments[i]->GetGPUResource()
        );

    const SGPUTextureResource& gpuDepthAttachment = depthAttachment->GetGPUResource();

    _gpuRenderTarget = CRenderCommandProxy::CreateRenderTarget(
        gpuColorAttachments,
        gpuDepthAttachment
    );
}

triton::XRenderTarget::~XRenderTarget()
{
    CThreadGuard::AssertMain();

    CRenderCommandProxy::DestroyRenderTarget(_gpuRenderTarget);
}