// render_pass_geometry.cpp

#include "render_pass_geometry.hpp"
#include "thread_guard.hpp"
#include "render_binding_group_binding_struct.hpp"
#include "shader_stage_bit_enum.hpp"
#include "input_backend.hpp"
#include "geometry_storage.hpp"
#include "gpu_buffer.hpp"
#include "world_render_domain.hpp"
#include "graphics.hpp"
#include "render_pass_culling.hpp"
#include "graphics_buffers.hpp"
#include "render_command_proxy.hpp"
#include "creator.hpp"
#include "synchronization.hpp"

using namespace types;

triton::XRenderPassGeometry::XRenderPassGeometry(
    const SMemAlloc& mem_alloc,
    cContext* context,
    XRenderDomain* renderDomain,
    XRenderTarget* render_target,
    boolean bClearRenderTarget,
    const cVector4& clearColor,
    f32 clearDepth,
    const std::vector<ERenderResourceUsage>& srcAttachmentsUsage,
    const std::vector<ERenderResourceUsage>& dstAttachmentsUsage,
    XShader* shader,
    ERenderPrimitiveTopology primitiveTopology,
    const SViewport& viewport,
    const std::vector<SGPUBufferResource>& inputBuffers,
    const std::vector<SShaderTextureBinding>& inputTextures
) :
    IRenderPass(mem_alloc, context, ERenderPassDispatch::Geometry),
    _renderDomain(renderDomain),
    _bClearRenderTarget(bClearRenderTarget),
    _renderTarget(render_target),
    _shader(shader),
    _primitiveTopology(primitiveTopology),
    _viewport(viewport)
{
    const SGPURenderTargetResource& render_target_resource = render_target->GetGPUResource();

    _gpuRenderPass = CRenderCommandProxy::CreateRenderPass(
        render_target_resource,
        bClearRenderTarget,
        clearColor,
        clearDepth,
        srcAttachmentsUsage,
        dstAttachmentsUsage
    );

    std::vector<SRenderBindingGroupBinding>* bufferBindingGroupBindings =
        new std::vector<SRenderBindingGroupBinding>();

    bufferBindingGroupBindings->push_back({ 0, ERenderBindingGroupBindingType::StorageBuffer, EShaderStageBit::Vertex, nullptr });
    bufferBindingGroupBindings->push_back({ 1, ERenderBindingGroupBindingType::StorageBuffer, EShaderStageBit::Vertex, nullptr });
    bufferBindingGroupBindings->push_back({ 2, ERenderBindingGroupBindingType::StorageBuffer, EShaderStageBit::Vertex, nullptr });
    bufferBindingGroupBindings->push_back({ 3, ERenderBindingGroupBindingType::StorageBuffer, EShaderStageBit::Vertex, nullptr });
    bufferBindingGroupBindings->push_back({ 4, ERenderBindingGroupBindingType::StorageBuffer, EShaderStageBit::Vertex, nullptr });

    _gpuBindingGroupLayouts.push_back(CRenderCommandProxy::CreateBindingGroupLayout(*bufferBindingGroupBindings));

    delete bufferBindingGroupBindings;

    std::vector<SRenderBindingGroupBinding>* textureBindingGroupBindings =
        new std::vector<SRenderBindingGroupBinding>();

    textureBindingGroupBindings->push_back({ 0, ERenderBindingGroupBindingType::TextureSampler, EShaderStageBit::Pixel, nullptr });
    textureBindingGroupBindings->push_back({ 1, ERenderBindingGroupBindingType::TextureSampler, EShaderStageBit::Pixel, nullptr });
    textureBindingGroupBindings->push_back({ 2, ERenderBindingGroupBindingType::TextureSampler, EShaderStageBit::Pixel, nullptr });
    textureBindingGroupBindings->push_back({ 3, ERenderBindingGroupBindingType::TextureSampler, EShaderStageBit::Pixel, nullptr });

    _gpuBindingGroupLayouts.push_back(CRenderCommandProxy::CreateBindingGroupLayout(*textureBindingGroupBindings));

    delete textureBindingGroupBindings;

    std::vector<SRenderBindingGroupBinding>* bufferBindingGroupBufferBindings =
        new std::vector<SRenderBindingGroupBinding>();
    std::vector<SRenderBindingGroupBinding>* bufferBindingGroupTextureBindings =
        new std::vector<SRenderBindingGroupBinding>();

    const SGPUBufferResource& instanceStaticBuffer = CGraphicsBuffers::GetStaticInstanceBuffer()->GetGPUResource();
    const SGPUBufferResource& instanceDynamicBuffer = CGraphicsBuffers::GetDynamicInstanceBuffer()->GetGPUResource();
    const SGPUBufferResource& materialBuffer = CGraphicsBuffers::GetMaterialBuffer()->GetGPUResource();
    const SGPUBufferResource& skinnedBoneBuffer = CGraphicsBuffers::GetSkinnedBoneBuffer()->GetGPUResource();
    const SGPUBufferResource& main_command_render_group_indices_buffer = CGraphicsBuffers::GetMainCommandRenderGroupIndicesBuffer();
    
    bufferBindingGroupBufferBindings->push_back({ 0, ERenderBindingGroupBindingType::StorageBuffer, EShaderStageBit::Vertex, (SGPUResource*)&instanceStaticBuffer });
    bufferBindingGroupBufferBindings->push_back({ 1, ERenderBindingGroupBindingType::StorageBuffer, EShaderStageBit::Vertex, (SGPUResource*)&instanceDynamicBuffer });
    bufferBindingGroupBufferBindings->push_back({ 2, ERenderBindingGroupBindingType::StorageBuffer, EShaderStageBit::Vertex, (SGPUResource*)&materialBuffer });
    bufferBindingGroupBufferBindings->push_back({ 3, ERenderBindingGroupBindingType::StorageBuffer, EShaderStageBit::Vertex, (SGPUResource*)&skinnedBoneBuffer });
    bufferBindingGroupBufferBindings->push_back({ 4, ERenderBindingGroupBindingType::StorageBuffer, EShaderStageBit::Vertex, (SGPUResource*)&main_command_render_group_indices_buffer });

    _gpuBindingGroupBuffersArrays[0].push_back(
        CRenderCommandProxy::CreateBindingGroup(
            0,
            _gpuBindingGroupLayouts[0],
            *bufferBindingGroupBufferBindings,
            *bufferBindingGroupTextureBindings
        )
    );

    _gpuBindingGroupBuffersArrays[1].push_back(
        _gpuBindingGroupBuffersArrays[0][0]
    );

    delete bufferBindingGroupTextureBindings;
    delete bufferBindingGroupBufferBindings;

    _gpuPipeline = CRenderCommandProxy::CreatePipeline(
        _shader->GetGPUResource(),
        _gpuBindingGroupLayouts,
        True,
        _viewport,
        (SGPURenderTargetResource&)render_target_resource,
        _gpuRenderPass,
        primitiveTopology,
        EVertexBufferFormat::Rigid
    );
}

triton::XRenderPassGeometry::~XRenderPassGeometry()
{
    CRenderCommandProxy::DestroyPipeline(_gpuPipeline);

    for (auto& gpuBindingGroupLayout : _gpuBindingGroupLayouts)
        CRenderCommandProxy::DestroyBindingGroupLayout(gpuBindingGroupLayout);

    CRenderCommandProxy::DestroyRenderPass(_gpuRenderPass);
}

void triton::XRenderPassGeometry::Render()
{
    Bind();
    Draw();
    Unbind();
}

void triton::XRenderPassGeometry::Bind()
{
    // TODO: [Vulkan backend] This must be done using render command queue on main thread
    // ||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||
    // VVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVV
    /*IGraphicsBackend* gfxBackend = _context->GetBackend<IGraphicsBackend>();

    XShader& shader = *_context->GetPool<CShaderPool>()->Get(_shader);
    gfxBackend->BindShader(shader.GetGPUResource());

    gfxBackend->SetViewport(_viewport);
    gfxBackend->BindDepthState(_depthState);
    gfxBackend->BindBlendState(_blendState);

    for (auto& tex : _inputTextures)
        gfxBackend->BindTextureNamed(shader.GetGPUResource(), tex.texture, tex.name, -1);

    auto iaResult = _context->GetPool<CInputLayoutPool>()->Get(_inputLayout);
    if (iaResult)
        gfxBackend->BindInputLayout((*iaResult).get().GetGPUResource());

    auto rtResult = _context->GetPool<CRenderTargetPool>()->Get(_renderTarget);
    if (rtResult)
        gfxBackend->BindRenderTarget((*rtResult).get().GetGPUResource());
    else
        gfxBackend->UnbindRenderTarget();

    if (_clearState.has_value())
    {
        SClearState cs = *_clearState;
        gfxBackend->ClearColor(cs.color);
        gfxBackend->ClearDepth(cs.depth);
    }*/
}

void triton::XRenderPassGeometry::Draw()
{
    // TODO: [Vulkan backend] This must be done using render command queue on main thread
    // ||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||
    // VVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVV
    /*IGraphicsBackend* gfxBackend = _context->GetBackend<IGraphicsBackend>();

    auto cameraResult = _context->GetPool<CCameraPool>()->Get(_camera);
    if (!cameraResult.has_value())
        return;

    XCamera& camera = *cameraResult;
    XShader& shader = *_context->GetPool<CShaderPool>()->Get(_shader);

    // TODO: remove this line
    const cVector2 mouseDelta = _context->GetBackend<IInputBackend>()->GetMouseDelta();
    camera.Update(mouseDelta, 800, 600, 65.0f, 0.01f, 10000.0f, 0.1f);
    cVector4 cameraWorldPos = cVector4(camera._worldPosition.GetX(), camera._worldPosition.GetY(), camera._worldPosition.GetZ(), 0.0f);
    ///////////////////////////////////////////////////////////////////////////

    camera.Bind(this);

    static u32 time = 0;
    time += 1;

    gfxBackend->SetShaderUniform(shader.GetGPUResource(), "time", (u32)time);
    gfxBackend->SetShaderUniform(shader.GetGPUResource(), "cameraPositionWorldSpace", 1, (f32*)&cameraWorldPos);

    for (usize i = 0; i < _renderInstancePacks.size(); i++)
    {
        XRenderInstancePack& instancePack =
            *_context->
            GetPool<CRenderInstancePackPool>()->
            Get(_renderInstancePacks.at(i));

        const SGeometryView sharedGeometry = instancePack.GetSharedGeometry();

        gfxBackend->SetShaderUniform(shader.GetGPUResource(), "instanceBatchType", (u32)instancePack.GetMotionType());
        gfxBackend->SetShaderUniform(shader.GetGPUResource(), "instanceOffset", (u32)instancePack.GetBufferOffset());
        
        const XMaterial::THandle& sharedMaterialHandle = instancePack.GetSharedMaterial();
        const XMaterial& sharedMaterial = *_context->GetPool<CMaterialPool>()->Get(sharedMaterialHandle);
        
        if (_shadingModel == EShadingModel::PBR)
        {
            const XTexture& sharedMaterialDiffuseTex = *_context->GetPool<PTexturePool>()->Get(sharedMaterial.GetDiffuseTexture());
            const XTexture& sharedMaterialNormalTex = *_context->GetPool<PTexturePool>()->Get(sharedMaterial.GetNormalTexture());
            const XTexture& sharedMaterialRoughnessTex = *_context->GetPool<PTexturePool>()->Get(sharedMaterial.GetRoughnessTexture());
            const XTexture& sharedMaterialMetallicTex = *_context->GetPool<PTexturePool>()->Get(sharedMaterial.GetMetallicTexture());

            gfxBackend->BindTextureNamed(
                shader.GetGPUResource(),
                sharedMaterialDiffuseTex.GetGPUResource(),
                "inDiffuseTexture",
                0
            );
            gfxBackend->BindTextureNamed(
                shader.GetGPUResource(),
                sharedMaterialNormalTex.GetGPUResource(),
                "inNormalTexture",
                1
            );
            gfxBackend->BindTextureNamed(
                shader.GetGPUResource(),
                sharedMaterialRoughnessTex.GetGPUResource(),
                "inRoughnessTexture",
                2
            );
            gfxBackend->BindTextureNamed(
                shader.GetGPUResource(),
                sharedMaterialMetallicTex.GetGPUResource(),
                "inMetallicTexture",
                3
            );
        }
        else if (_shadingModel == EShadingModel::PBR)
        {
            const XTexture& sharedMaterialDiffuseTex = *_context->GetPool<PTexturePool>()->Get(sharedMaterial.GetDiffuseTexture());
            const XTexture& sharedMaterialNormalTex = *_context->GetPool<PTexturePool>()->Get(sharedMaterial.GetNormalTexture());

            gfxBackend->BindTextureNamed(
                shader.GetGPUResource(),
                sharedMaterialDiffuseTex.GetGPUResource(),
                "inDiffuseTexture",
                0
            );
            gfxBackend->BindTextureNamed(
                shader.GetGPUResource(),
                sharedMaterialNormalTex.GetGPUResource(),
                "inNormalTexture",
                1
            );
        }

        gfxBackend->Draw(
            sharedGeometry._indexCount,
            sharedGeometry._vertexElementOffset,
            sharedGeometry._indexElementOffset,
            instancePack.GetInstanceCount()
        );
    }*/

    XCamera* camera = _context->GetSubsystem<XGraphics>()->GetCamera();
    camera->Update(
        _context->GetBackend<IInputBackend>()->GetMouseDelta(),
        800,
        600,
        0.25f
    );

    XGPUBuffer* vertex_buffer = CGraphicsBuffers::GetRigidVertexBuffer();
    XGPUBuffer* index_buffer = CGraphicsBuffers::GetRigidIndexBuffer();

    const usize mainThreadWriteIndex = _context->GetSubsystem<XEngine>()->GetSynchronization()->GetWriteIndex();

    static f32 t = 0.0f;
    t += 1.0f;

    _pushConstantArrays[mainThreadWriteIndex].cameraViewProjectionMatrix = camera->GetViewProjectionMatrix();
    _pushConstantArrays[mainThreadWriteIndex].cameraWorldPosition = camera->GetWorldPosition();
    _pushConstantArrays[mainThreadWriteIndex].time = t;

    _context->GetSubsystem<XEngine>()->GetRenderCommandRecorder()->PushCommand(SRenderCommand(
        ERenderCommand::BeginRenderPass,
        (cpuword)&_gpuRenderPass,
        (cpuword)&_renderTarget->GetGPUResource(),
        (cpuword)&_gpuPipeline,
        (cpuword)&vertex_buffer->GetGPUResource(),
        (cpuword)&index_buffer->GetGPUResource(),
        (cpuword)&_pushConstantArrays[mainThreadWriteIndex],
        (cpuword)&_gpuBindingGroupBuffersArrays[mainThreadWriteIndex]
    ));

    const std::vector<XRenderGroup*>& render_groups = _renderDomain->GetRenderGroups();

    for (usize i = 0; i < render_groups.size(); i++)
    {
        XRenderGroup* render_group = render_groups.at(i);

        XMaterial* shared_material = render_group->GetSharedMaterial();

        const SGeometryView& sharedGeometry = render_group->GetSharedGeometry();

        _context->GetSubsystem<XEngine>()->GetRenderCommandRecorder()->PushCommand(SRenderCommand(
            ERenderCommand::ExecuteRenderPass,
            (cpuword)1,
            (cpuword)&shared_material->GetBindingGroup(),
            (cpuword)&_gpuPipeline,
            (cpuword)render_group->GetMotionType(),
            (cpuword)sharedGeometry.vertex_count,
            (cpuword)sharedGeometry.draw_info.index_count,
            (cpuword)render_group->GetInstanceCount(),
            (cpuword)sharedGeometry.draw_info.index_element_offset,
            (cpuword)sharedGeometry.draw_info.vertex_element_offset,
            (cpuword)render_group->GetInstanceBufferStartOffset()
        ));
    }

    _context->GetSubsystem<XEngine>()->GetRenderCommandRecorder()->PushCommand(SRenderCommand(
        ERenderCommand::EndRenderPass
    ));
}

void triton::XRenderPassGeometry::Unbind()
{
    // TODO: [Vulkan backend] This must be done using render command queue on main thread
    // ||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||
    // VVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVV
    /*IGraphicsBackend* gfxBackend = _context->GetBackend<IGraphicsBackend>();

    gfxBackend->UnbindInputLayout();

    for (auto& tex : _inputTextures)
        gfxBackend->UnbindTexture(tex.texture);

    gfxBackend->UnbindRenderTarget();*/
}