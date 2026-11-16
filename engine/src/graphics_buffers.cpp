// graphics_buffers.cpp

#include "graphics_buffers.hpp"
#include "render_command_proxy.hpp"
#include "render_indirect_draw_command_struct.hpp"
#include "context.hpp"
#include "engine.hpp"
#include "application.hpp"

using namespace types;

void triton::CGraphicsBuffers::Initialize(cContext* context)
{
	_context = context;

    const sCapabilities& caps = _context->GetSubsystem<CEngine>()->GetApplication()->GetCapabilities();

    _indirect_draw_commands_buffer = CRenderCommandProxy::CreateBuffer(
        EGPUBufferType::Storage,
        caps.render_group_count * sizeof(SIndirectDrawCommand)
    );

    _motion_type_per_command_buffer = CRenderCommandProxy::CreateBuffer(
        EGPUBufferType::Storage,
        caps.render_group_count * sizeof(u32)
    );
}

void triton::CGraphicsBuffers::Release()
{
    CRenderCommandProxy::DestroyBuffer(_motion_type_per_command_buffer);
    CRenderCommandProxy::DestroyBuffer(_indirect_draw_commands_buffer);
}