// render_pass_culling.cpp

#include "render_pass_culling.hpp"
#include "render_command_proxy.hpp"
#include "context.hpp"
#include "application.hpp"
#include "render_indirect_draw_command_struct.hpp"
#include "camera_pool.hpp"
#include "world_render_group_pool.hpp"
#include "render_instance_static_pool.hpp"
#include "render_instance_dynamic_pool.hpp"

using namespace types;

triton::XRenderPassCulling::XRenderPassCulling(cContext* context, s32 poolIndex) :
    IRenderPass(context, poolIndex, ERenderPassDispatch::Culling),
    _indirect_draw_command_counter_buffer(CGPUBufferResource::Invalid()),
    _indirect_draw_commands_buffer(CGPUBufferResource::Invalid()),
    _motion_type_per_command_buffer(CGPUBufferResource::Invalid()),
    _shader(CGPUShaderResource::Invalid()),
    _binding_group_layout(CGPUBindingGroupLayoutResource::Invalid()),
    _binding_group(CGPUBindingGroupResource::Invalid()),
    _pipeline(CGPUPipelineResource::Invalid())
{
    const sCapabilities& caps = _context->GetSubsystem<CEngine>()->GetApplication()->GetCapabilities();

    _indirect_draw_command_counter_buffer = CRenderCommandProxy::CreateBuffer(EGPUBufferType::Storage, sizeof(u32));

    _indirect_draw_commands_buffer = CRenderCommandProxy::CreateBuffer(
        EGPUBufferType::Storage,
        caps.render_group_count * sizeof(SIndirectDrawCommand)
    );

    _motion_type_per_command_buffer = CRenderCommandProxy::CreateBuffer(
        EGPUBufferType::Storage,
        caps.render_group_count * sizeof(u32)
    );

    SShaderBytecodeFiles shader_files;
    shader_files.computeFilePath =
        "C:/My/My_Projects_Programming/TritonEngine/runtime/data/shaders/builtin/culling.comp.spv";

    _shader = CRenderCommandProxy::CreateShader(
        EShaderStageBit::Compute,
        shader_files
    );

    std::vector<SBindingGroupBinding>* bindings =
        CObjectAllocator::Create<std::vector<SBindingGroupBinding>>(64);

    bindings->push_back(
        SBindingGroupBinding(
            0,
            EBindingGroupBindingType::StorageBuffer,
            EShaderStageBit::Compute,
            (CGPUResource*)&_indirect_draw_command_counter_buffer
        )
    );
    bindings->push_back(
        SBindingGroupBinding(
            1,
            EBindingGroupBindingType::StorageBuffer,
            EShaderStageBit::Compute,
            (CGPUResource*)&_indirect_draw_commands_buffer
        )
    );
    bindings->push_back(
        SBindingGroupBinding(
            2,
            EBindingGroupBindingType::StorageBuffer,
            EShaderStageBit::Compute,
            (CGPUResource*)&_motion_type_per_command_buffer
        )
    );
    bindings->push_back(
        SBindingGroupBinding(
            3,
            EBindingGroupBindingType::StorageBuffer,
            EShaderStageBit::Compute,
            (CGPUResource*)&_context->GetPool<CCameraPool>()->GetGPUBuffer()
        )
    );
    bindings->push_back(
        SBindingGroupBinding(
            4,
            EBindingGroupBindingType::StorageBuffer,
            EShaderStageBit::Compute,
            (CGPUResource*)&_context->GetPool<CRenderGroupPool>()->GetGPUBuffer()
        )
    );
    bindings->push_back(
        SBindingGroupBinding(
            5,
            EBindingGroupBindingType::StorageBuffer,
            EShaderStageBit::Compute,
            (CGPUResource*)&_context->GetPool<CRenderInstanceStaticPool>()->GetGPUBuffer()
        )
    );
    bindings->push_back(
        SBindingGroupBinding(
            6,
            EBindingGroupBindingType::StorageBuffer,
            EShaderStageBit::Compute,
            (CGPUResource*)&_context->GetPool<CRenderInstanceDynamicPool>()->GetGPUBuffer()
        )
    );

    _binding_group_layout = CRenderCommandProxy::CreateBindingGroupLayout(*bindings);

    _binding_group = CRenderCommandProxy::CreateBindingGroup(
        0,
        _binding_group_layout,
        *bindings,
        {}
    );

    CObjectAllocator::Destroy<std::vector<SBindingGroupBinding>>(bindings);

    std::vector<CGPUBindingGroupLayoutResource>* binding_group_layouts =
        CObjectAllocator::Create<std::vector<CGPUBindingGroupLayoutResource>>(64);

    binding_group_layouts->push_back(_binding_group_layout);

    _pipeline = CRenderCommandProxy::CreatePipeline(
        _shader,
        *binding_group_layouts,
        False
    );

    CObjectAllocator::Destroy<std::vector<CGPUBindingGroupLayoutResource>>(binding_group_layouts);
}

triton::XRenderPassCulling::~XRenderPassCulling()
{
    CRenderCommandProxy::DestroyPipeline(_pipeline);
    CRenderCommandProxy::DestroyBindingGroup(_binding_group);
    CRenderCommandProxy::DestroyBindingGroupLayout(_binding_group_layout);
    CRenderCommandProxy::DestroyShader(_shader);
    CRenderCommandProxy::DestroyBuffer(_motion_type_per_command_buffer);
    CRenderCommandProxy::DestroyBuffer(_indirect_draw_commands_buffer);
    CRenderCommandProxy::DestroyBuffer(_indirect_draw_command_counter_buffer);
}

void triton::XRenderPassCulling::Render()
{
    Bind();
    Draw();
    Unbind();
}

void triton::XRenderPassCulling::Bind()
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

void triton::XRenderPassCulling::Draw()
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

    auto cameraResult = _context->GetPool<CCameraPool>()->Get(
        _context->GetSubsystem<CGraphics>()->GetCamera()
    );
    if (!cameraResult.has_value())
        return;

    XCamera& camera = *cameraResult;
    camera.Update(
        _context->GetBackend<IInputBackend>()->GetMouseDelta(),
        800,
        600,
        0.25f
    );

    SHandle vertexBufferHandle = _context->GetSubsystem<CGeometryStorage>()->GetRigidVertexBuffer();
    SHandle indexBufferHandle = _context->GetSubsystem<CGeometryStorage>()->GetRigidIndexBuffer();
    XGPUBuffer& vertexBuffer = *_context->GetPool<CGPUBufferPool>()->Get(vertexBufferHandle);
    XGPUBuffer& indexBuffer = *_context->GetPool<CGPUBufferPool>()->Get(indexBufferHandle);
    const CGPUBufferResource& gpuVertexBuffer = vertexBuffer.GetGPUResource();
    const CGPUBufferResource& gpuIndexBuffer = indexBuffer.GetGPUResource();
    XRenderTarget& renderTarget = *_context->GetPool<CRenderTargetPool>()->Get(_renderTarget);

    const usize mainThreadWriteIndex = _context->GetSubsystem<CEngine>()->GetSynchronization()->GetWriteIndex();

    static f32 t = 0.0f;
    t += 1.0f;

    _pushConstantArrays[mainThreadWriteIndex].cameraViewProjectionMatrix = camera.GetViewProjectionMatrix();
    _pushConstantArrays[mainThreadWriteIndex].cameraWorldPosition = camera.GetWorldPosition();
    _pushConstantArrays[mainThreadWriteIndex].time = t;

    _context->GetSubsystem<CEngine>()->GetRenderCommandRecorder()->PushCommand(SRenderCommand(
        ERenderCommand::BeginRenderPass,
        (cpuword)&_gpuRenderPass,
        (cpuword)&renderTarget.GetGPUResource(),
        (cpuword)&_gpuPipeline,
        (cpuword)&gpuVertexBuffer,
        (cpuword)&gpuIndexBuffer,
        (cpuword)&_pushConstantArrays[mainThreadWriteIndex],
        (cpuword)&_gpuBindingGroupBuffersArrays[mainThreadWriteIndex]
    ));

    auto renderDomainLookup = _context->GetPool<CRenderDomainPool>()->Get(_renderDomain);

    if (!renderDomainLookup.has_value())
        return;

    XRenderDomain& renderDomain = *renderDomainLookup;
    const std::vector<SHandle>& renderGroups = renderDomain.GetRenderGroups();

    for (usize i = 0; i < renderGroups.size(); i++)
    {
        XRenderGroup& renderGroup =
            *_context->
            GetPool<CRenderGroupPool>()->
            Get(renderGroups.at(i));

        const XMaterial& sharedMaterial =
            *_context->GetPool<CMaterialPool>()->Get(renderGroup.GetSharedMaterial());

        const SGeometryView& sharedGeometry =
            renderGroup.GetSharedGeometry();

        _context->GetSubsystem<CEngine>()->GetRenderCommandRecorder()->PushCommand(SRenderCommand(
            ERenderCommand::ExecuteRenderPass,
            (cpuword)1,
            (cpuword)&sharedMaterial.GetBindingGroup(),
            (cpuword)&_gpuPipeline,
            (cpuword)renderGroup.GetMotionType(),
            (cpuword)sharedGeometry._vertexCount,
            (cpuword)sharedGeometry.draw_info.index_count,
            (cpuword)renderGroup.GetInstanceCount(),
            (cpuword)sharedGeometry.draw_info.index_element_offset,
            (cpuword)sharedGeometry.draw_info.vertex_element_offset,
            (cpuword)renderGroup.GetInstanceBufferOffset()
        ));
    }

    _context->GetSubsystem<CEngine>()->GetRenderCommandRecorder()->PushCommand(SRenderCommand(
        ERenderCommand::EndRenderPass
    ));
}

void triton::XRenderPassCulling::Unbind()
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