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