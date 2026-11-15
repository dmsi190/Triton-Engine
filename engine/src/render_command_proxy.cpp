// render_command_proxy.cpp

#include "render_command_proxy.hpp"
#include "context.hpp"
#include "engine.hpp"
#include "synchronization.hpp"

using namespace types;

void triton::CRenderCommandProxy::SetContext(cContext* context)
{
	_context = context;
}

triton::CGPUBufferResource triton::CRenderCommandProxy::CreateBuffer(
	EGPUBufferType type,
	usize byte_size
)
{
    _context->GetSubsystem<CEngine>()->GetRenderCommandRecorder()->PushCommand(SRenderCommand(
        ERenderCommand::CREATE_BUFFER,
        (cpuword)type,
        (cpuword)byte_size
    ));

    return _context->GetSubsystem<CEngine>()->
        GetSynchronization()->
        WaitForRenderCommandResult<CGPUBufferResource>();
}

void triton::CRenderCommandProxy::DestroyBuffer(CGPUBufferResource& buffer)
{
    _context->GetSubsystem<CEngine>()->GetRenderCommandRecorder()->PushCommand(SRenderCommand(
        ERenderCommand::DESTROY_BUFFER,
        (cpuword)&buffer
    ));

    _context->GetSubsystem<CEngine>()->
        GetSynchronization()->
        WaitForRenderCommandResult<void*>();
}

triton::CGPUShaderResource triton::CRenderCommandProxy::CreateShader(
    EShaderStageBit stage_mask,
    const SShaderBytecodeFiles& files
)
{
    _context->GetSubsystem<CEngine>()->GetRenderCommandRecorder()->PushCommand(SRenderCommand(
        ERenderCommand::CREATE_SHADER,
        (cpuword)stage_mask,
        (cpuword)&files
    ));

    return _context->GetSubsystem<CEngine>()->
        GetSynchronization()->
        WaitForRenderCommandResult<CGPUShaderResource>();
}

void triton::CRenderCommandProxy::DestroyShader(CGPUShaderResource& shader)
{
    _context->GetSubsystem<CEngine>()->GetRenderCommandRecorder()->PushCommand(SRenderCommand(
        ERenderCommand::DESTROY_SHADER,
        (cpuword)&shader
    ));

    _context->GetSubsystem<CEngine>()->
        GetSynchronization()->
        WaitForRenderCommandResult<void*>();
}

triton::CGPUBindingGroupLayoutResource triton::CRenderCommandProxy::CreateBindingGroupLayout(
    const std::vector<SBindingGroupBinding>& bindings
)
{
    _context->GetSubsystem<CEngine>()->GetRenderCommandRecorder()->PushCommand(SRenderCommand(
        ERenderCommand::CreateBindingGroupLayout,
        (cpuword)&bindings
    ));

    return _context->GetSubsystem<CEngine>()->
        GetSynchronization()->
        WaitForRenderCommandResult<CGPUBindingGroupLayoutResource>();
}

void triton::CRenderCommandProxy::DestroyBindingGroupLayout(CGPUBindingGroupLayoutResource& binding_group_layout)
{
    _context->GetSubsystem<CEngine>()->GetRenderCommandRecorder()->PushCommand(SRenderCommand(
        ERenderCommand::DestroyBindingGroupLayout,
        (cpuword)&binding_group_layout
    ));

    _context->GetSubsystem<CEngine>()->
        GetSynchronization()->
        WaitForRenderCommandResult<void*>();
}

triton::CGPUBindingGroupResource triton::CRenderCommandProxy::CreateBindingGroup(
    u32 index,
    const CGPUBindingGroupLayoutResource& layout,
    const std::vector<SBindingGroupBinding>& buffers_to_bind,
    const std::vector<SBindingGroupBinding>& textures_to_bind
)
{
    _context->GetSubsystem<CEngine>()->GetRenderCommandRecorder()->PushCommand(SRenderCommand(
        ERenderCommand::CreateBindingGroup,
        (cpuword)0,
        (cpuword)&layout,
        (cpuword)&buffers_to_bind,
        (cpuword)&textures_to_bind
    ));

    return _context->GetSubsystem<CEngine>()->
        GetSynchronization()->
        WaitForRenderCommandResult<CGPUBindingGroupResource>();
}

void triton::CRenderCommandProxy::DestroyBindingGroup(CGPUBindingGroupResource& binding_group)
{
    _context->GetSubsystem<CEngine>()->GetRenderCommandRecorder()->PushCommand(SRenderCommand(
        ERenderCommand::DestroyBindingGroup,
        (cpuword)&binding_group
    ));

    _context->GetSubsystem<CEngine>()->
        GetSynchronization()->
        WaitForRenderCommandResult<void*>();
}

triton::CGPUPipelineResource triton::CRenderCommandProxy::CreatePipeline(
    const CGPUShaderResource& shader,
    const std::vector<CGPUBindingGroupLayoutResource>& bindingGroupLayouts,
    types::boolean bUsePushConstants,
    const std::optional<std::reference_wrapper<SViewport>> viewport,
    std::optional<std::reference_wrapper<CGPURenderTargetResource>> renderTarget,
    const std::optional<std::reference_wrapper<CGPURenderPassResource>> renderPass,
    EPrimitiveTopology primitiveTopology,
    EVertexBufferFormat vertexBufferFormat
)
{
    _context->GetSubsystem<CEngine>()->GetRenderCommandRecorder()->PushCommand(SRenderCommand(
        ERenderCommand::CREATE_PIPELINE,
        (cpuword)&shader,
        (cpuword)&bindingGroupLayouts,
        (cpuword)bUsePushConstants,
        (cpuword)(viewport.has_value() ? &viewport->get() : 0),
        (cpuword)(renderTarget.has_value() ? &renderTarget->get() : 0),
        (cpuword)(renderPass.has_value() ? &renderPass->get() : 0),
        (cpuword)primitiveTopology,
        (cpuword)vertexBufferFormat
    ));

    return _context->GetSubsystem<CEngine>()->
        GetSynchronization()->
        WaitForRenderCommandResult<CGPUPipelineResource>();
}

void triton::CRenderCommandProxy::DestroyPipeline(CGPUPipelineResource& pipeline)
{
    _context->GetSubsystem<CEngine>()->GetRenderCommandRecorder()->PushCommand(SRenderCommand(
        ERenderCommand::DESTROY_PIPELINE,
        (cpuword)&pipeline
    ));

    _context->GetSubsystem<CEngine>()->
        GetSynchronization()->
        WaitForRenderCommandResult<void*>();
}

void triton::CRenderCommandProxy::Dispatch(
    const CGPUPipelineResource& pipeline,
    const std::vector<CGPUBindingGroupResource>& binding_groups,
    const cVector3& work_group_count
)
{
    _context->GetSubsystem<CEngine>()->GetRenderCommandRecorder()->PushCommand(SRenderCommand(
        ERenderCommand::Dispatch,
        (cpuword)&pipeline,
        (cpuword)&binding_groups,
        (cpuword)work_group_count.GetX(),
        (cpuword)work_group_count.GetY(),
        (cpuword)work_group_count.GetZ()
    ));
}