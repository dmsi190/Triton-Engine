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