// render_pass_processing.hpp

#include "render_pass_processing.hpp"

using namespace types;

triton::XRenderPassProcessing::XRenderPassProcessing(
    const SMemAlloc& mem_alloc,
    cContext* context,
    const std::vector<SGPUBufferResource>& inputBuffers,
    const std::vector<SShaderTextureBinding>& inputTextures,
    const SViewport& viewport,
    const SBlendState& blendState,
    const SDepthState& depthState,
    XRenderTarget* renderTarget,
    XShader* shader
) :
    IRenderPass(mem_alloc, context, ERenderPassDispatch::Processing),
    _inputBuffers(inputBuffers),
    _inputTextures(inputTextures),
    _viewport(viewport),
    _blendState(blendState),
    _depthState(depthState),
    _renderTarget(renderTarget),
    _shader(shader) {}

void triton::XRenderPassProcessing::Render()
{
    //CThreadGuard::AssertRender();

    // TODO: [Vulkan backend] This must be done using render command queue on main thread
    // ||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||
    // VVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVV
    /*IGraphicsBackend* gfxBackend = _context->GetBackend<IGraphicsBackend>();

    XShader& shader = *_context->GetPool<CShaderPool>()->Get(_shader);
    gfxBackend->BindShader(shader.GetGPUResource());

    gfxBackend->SetViewport(_viewport);
    gfxBackend->BindDepthState(_depthState);
    gfxBackend->BindBlendState(_blendState);

    for (auto& buf : _inputBuffers)
        gfxBackend->BindBuffer(buf);

    for (auto& tex : _inputTextures)
        gfxBackend->BindTextureNamed(shader.GetGPUResource(), tex.texture, tex.name, -1);
    
    XInputLayout& ia = *_context->GetPool<CInputLayoutPool>()->Get(_inputLayout);
    gfxBackend->BindInputLayout(ia.GetGPUResource());

    auto rtResult = _context->GetPool<CRenderTargetPool>()->Get(_renderTarget);
    if (rtResult.has_value())
        gfxBackend->BindRenderTarget(rtResult.value().get().GetGPUResource());
    else
        gfxBackend->UnbindRenderTarget();

    gfxBackend->DrawQuad();

    gfxBackend->UnbindInputLayout();

    if (rtResult.has_value())
        gfxBackend->UnbindRenderTarget();

    for (auto& tex : _inputTextures)
        gfxBackend->UnbindTexture(tex.texture);*/
}