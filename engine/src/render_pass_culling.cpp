// render_pass_culling.cpp

#include "render_pass_culling.hpp"
#include "render_command_proxy.hpp"
#include "context.hpp"
#include "application.hpp"
#include "render_indirect_draw_command_struct.hpp"
#include "camera_pool.hpp"
#include "world_render_group_pool.hpp"
#include "graphics_buffers.hpp"
#include "creator.hpp"

using namespace types;

triton::XRenderPassCulling::XRenderPassCulling(const SMemAlloc& mem_alloc, cContext* context) :
    IRenderPass(mem_alloc, context, ERenderPassDispatch::Culling),
    _shader(SGPUShaderResource::Invalid()),
    _binding_group_layout(SGPUBindingGroupLayoutResource::Invalid()),
    _binding_groups(),
    _pipeline(SGPUPipelineResource::Invalid())
{
    const SCapabilities& caps = _context->GetSubsystem<CEngine>()->GetApplication()->GetCapabilities();

    SShaderBytecodeFiles shader_files;
    shader_files.computeFilePath =
        "C:/My/My_Projects_Programming/TritonEngine/runtime/data/shaders/builtin/culling.comp.spv";

    _shader = CRenderCommandProxy::CreateShader(
        EShaderStageBit::Compute,
        shader_files
    );

    std::vector<SBindingGroupBinding>* bindings =
        CCreator::Create<std::vector<SBindingGroupBinding>>();

    bindings->push_back(
        SBindingGroupBinding(
            0,
            ERenderBindingGroupBindingType::StorageBuffer,
            EShaderStageBit::Compute,
            (SGPUResource*)&CGraphicsBuffers::GetMainIndirectDrawCommandsBuffer()
        )
    );
    bindings->push_back(
        SBindingGroupBinding(
            1,
            ERenderBindingGroupBindingType::StorageBuffer,
            EShaderStageBit::Compute,
            (SGPUResource*)&CGraphicsBuffers::GetMainIndirectDrawCommandsBuffer()
        )
    );
    bindings->push_back(
        SBindingGroupBinding(
            2,
            ERenderBindingGroupBindingType::StorageBuffer,
            EShaderStageBit::Compute,
            (SGPUResource*)&CGraphicsBuffers::GetMainCommandRenderGroupIndicesBuffer()
        )
    );
    bindings->push_back(
        SBindingGroupBinding(
            3,
            ERenderBindingGroupBindingType::StorageBuffer,
            EShaderStageBit::Compute,
            (SGPUResource*)&CGraphicsBuffers::GetShadowCommandRenderGroupIndicesBuffer()
        )
    );
    bindings->push_back(
        SBindingGroupBinding(
            4,
            ERenderBindingGroupBindingType::StorageBuffer,
            EShaderStageBit::Compute,
            (SGPUResource*)&CGraphicsBuffers::GetVisibleRenderStaticIndicesBuffer()
        )
    );
    bindings->push_back(
        SBindingGroupBinding(
            5,
            ERenderBindingGroupBindingType::StorageBuffer,
            EShaderStageBit::Compute,
            (SGPUResource*)&CGraphicsBuffers::GetVisibleRenderStaticIndicesBuffer()
        )
    );
    bindings->push_back(
        SBindingGroupBinding(
            6,
            ERenderBindingGroupBindingType::StorageBuffer,
            EShaderStageBit::Compute,
            (SGPUResource*)&_context->GetPool<CCameraPool>()->GetGPUBuffer()
        )
    );
    bindings->push_back(
        SBindingGroupBinding(
            7,
            ERenderBindingGroupBindingType::StorageBuffer,
            EShaderStageBit::Compute,
            (SGPUResource*)&_context->GetPool<CRenderGroupPool>()->GetGPUBuffer()
        )
    );
    bindings->push_back(
        SBindingGroupBinding(
            8,
            ERenderBindingGroupBindingType::StorageBuffer,
            EShaderStageBit::Compute,
            (SGPUResource*)&_context->GetPool<CRenderInstanceStaticPool>()->GetGPUBuffer()
        )
    );
    bindings->push_back(
        SBindingGroupBinding(
            9,
            ERenderBindingGroupBindingType::StorageBuffer,
            EShaderStageBit::Compute,
            (SGPUResource*)&_context->GetPool<CRenderInstanceDynamicPool>()->GetGPUBuffer()
        )
    );

    _binding_group_layout = CRenderCommandProxy::CreateBindingGroupLayout(*bindings);

    _binding_groups.push_back(
        CRenderCommandProxy::CreateBindingGroup(
            0,
            _binding_group_layout,
            *bindings,
            {}
        )
    );

    CCreator::Destroy<std::vector<SBindingGroupBinding>>(bindings);

    std::vector<SGPUBindingGroupLayoutResource>* binding_group_layouts =
        CCreator::Create<std::vector<SGPUBindingGroupLayoutResource>>(64);

    binding_group_layouts->push_back(_binding_group_layout);

    _pipeline = CRenderCommandProxy::CreatePipeline(
        _shader,
        *binding_group_layouts,
        False
    );

    CCreator::Destroy<std::vector<SGPUBindingGroupLayoutResource>>(binding_group_layouts);
}

triton::XRenderPassCulling::~XRenderPassCulling()
{
    CRenderCommandProxy::DestroyPipeline(_pipeline);
    CRenderCommandProxy::DestroyBindingGroup(_binding_groups.at(0));
    CRenderCommandProxy::DestroyBindingGroupLayout(_binding_group_layout);
    CRenderCommandProxy::DestroyShader(_shader);

    _binding_groups.clear();
}

void triton::XRenderPassCulling::Render()
{
    CRenderCommandProxy::Dispatch(
        _pipeline,
        _binding_groups,
        cVector3(1.0f)
    );
}