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
#include "graphics_buffers.hpp"

using namespace types;

triton::XRenderPassCulling::XRenderPassCulling(cContext* context, s32 poolIndex) :
    IRenderPass(context, poolIndex, ERenderPassDispatch::Culling),
    _indirect_draw_command_counter_buffer(CGPUBufferResource::Invalid()),
    _shader(CGPUShaderResource::Invalid()),
    _binding_group_layout(CGPUBindingGroupLayoutResource::Invalid()),
    _binding_groups(),
    _pipeline(CGPUPipelineResource::Invalid())
{
    const sCapabilities& caps = _context->GetSubsystem<CEngine>()->GetApplication()->GetCapabilities();

    _indirect_draw_command_counter_buffer = CRenderCommandProxy::CreateBuffer(EGPUBufferType::Storage, sizeof(u32));

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
            (CGPUResource*)&CGraphicsBuffers::GetIndirectDrawCommandsBuffer()
        )
    );
    bindings->push_back(
        SBindingGroupBinding(
            2,
            EBindingGroupBindingType::StorageBuffer,
            EShaderStageBit::Compute,
            (CGPUResource*)&CGraphicsBuffers::GetMotionTypePerCommandBuffer()
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

    _binding_groups.push_back(
        CRenderCommandProxy::CreateBindingGroup(
            0,
            _binding_group_layout,
            *bindings,
            {}
        )
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
    CRenderCommandProxy::DestroyBindingGroup(_binding_groups.at(0));
    CRenderCommandProxy::DestroyBindingGroupLayout(_binding_group_layout);
    CRenderCommandProxy::DestroyShader(_shader);
    CRenderCommandProxy::DestroyBuffer(_indirect_draw_command_counter_buffer);

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