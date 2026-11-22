// render_pass_culling.cpp

#include "render_pass_culling.hpp"
#include "render_command_proxy.hpp"
#include "context.hpp"
#include "application.hpp"
#include "render_indirect_draw_command_struct.hpp"
#include "graphics_buffers.hpp"
#include "creator.hpp"
#include "gpu_buffer.hpp"

using namespace types;

triton::XRenderPassCulling::XRenderPassCulling(const SMemAlloc& mem_alloc, cContext* context) :
    IRenderPass(mem_alloc, context, ERenderPassDispatch::Culling),
    _shader(SGPUShaderResource::Invalid()),
    _binding_group_layout(SGPUBindingGroupLayoutResource::Invalid()),
    _binding_groups(),
    _pipeline(SGPUPipelineResource::Invalid())
{
    const SCapabilities& caps = _context->GetSubsystem<XEngine>()->GetApplication()->GetCapabilities();

    SShaderBytecodeFiles shader_files;
    shader_files.computeFilePath =
        "C:/My/My_Projects_Programming/TritonEngine/runtime/data/shaders/builtin/culling.comp.spv";

    _shader = CRenderCommandProxy::CreateShader(
        EShaderStageBit::Compute,
        shader_files
    );

    std::vector<SRenderBindingGroupBinding>* bindings =
        new std::vector<SRenderBindingGroupBinding>();

    bindings->push_back(
        SRenderBindingGroupBinding(
            0,
            ERenderBindingGroupBindingType::StorageBuffer,
            EShaderStageBit::Compute,
            (SGPUResource*)&CGraphicsBuffers::GetMainIndirectDrawCommandsBuffer()
        )
    );
    bindings->push_back(
        SRenderBindingGroupBinding(
            1,
            ERenderBindingGroupBindingType::StorageBuffer,
            EShaderStageBit::Compute,
            (SGPUResource*)&CGraphicsBuffers::GetMainIndirectDrawCommandsBuffer()
        )
    );
    bindings->push_back(
        SRenderBindingGroupBinding(
            2,
            ERenderBindingGroupBindingType::StorageBuffer,
            EShaderStageBit::Compute,
            (SGPUResource*)&CGraphicsBuffers::GetMainCommandRenderGroupIndicesBuffer()
        )
    );
    bindings->push_back(
        SRenderBindingGroupBinding(
            3,
            ERenderBindingGroupBindingType::StorageBuffer,
            EShaderStageBit::Compute,
            (SGPUResource*)&CGraphicsBuffers::GetShadowCommandRenderGroupIndicesBuffer()
        )
    );
    bindings->push_back(
        SRenderBindingGroupBinding(
            4,
            ERenderBindingGroupBindingType::StorageBuffer,
            EShaderStageBit::Compute,
            (SGPUResource*)&CGraphicsBuffers::GetVisibleRenderStaticIndicesBuffer()
        )
    );
    bindings->push_back(
        SRenderBindingGroupBinding(
            5,
            ERenderBindingGroupBindingType::StorageBuffer,
            EShaderStageBit::Compute,
            (SGPUResource*)&CGraphicsBuffers::GetVisibleRenderStaticIndicesBuffer()
        )
    );
    bindings->push_back(
        SRenderBindingGroupBinding(
            6,
            ERenderBindingGroupBindingType::StorageBuffer,
            EShaderStageBit::Compute,
            (SGPUResource*)&CGraphicsBuffers::GetCameraBuffer()->GetGPUResource()
        )
    );
    bindings->push_back(
        SRenderBindingGroupBinding(
            7,
            ERenderBindingGroupBindingType::StorageBuffer,
            EShaderStageBit::Compute,
            (SGPUResource*)&CGraphicsBuffers::GetRenderGroupBuffer()->GetGPUResource()
        )
    );
    bindings->push_back(
        SRenderBindingGroupBinding(
            8,
            ERenderBindingGroupBindingType::StorageBuffer,
            EShaderStageBit::Compute,
            (SGPUResource*)&CGraphicsBuffers::GetStaticInstanceBuffer()->GetGPUResource()
        )
    );
    bindings->push_back(
        SRenderBindingGroupBinding(
            9,
            ERenderBindingGroupBindingType::StorageBuffer,
            EShaderStageBit::Compute,
            (SGPUResource*)&CGraphicsBuffers::GetDynamicInstanceBuffer()->GetGPUResource()
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

    delete bindings;

    std::vector<SGPUBindingGroupLayoutResource>* binding_group_layouts =
        new std::vector<SGPUBindingGroupLayoutResource>();

    binding_group_layouts->push_back(_binding_group_layout);

    _pipeline = CRenderCommandProxy::CreatePipeline(
        _shader,
        *binding_group_layouts,
        False
    );

    delete binding_group_layouts;
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