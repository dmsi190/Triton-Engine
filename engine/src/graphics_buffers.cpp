// graphics_buffers.cpp

#include "graphics_buffers.hpp"
#include "render_command_proxy.hpp"
#include "render_indirect_draw_command_struct.hpp"
#include "context.hpp"
#include "engine.hpp"
#include "application.hpp"
#include "world_render_instance.hpp"
#include "creator.hpp"
#include "gpu_buffer.hpp"

using namespace types;

void triton::CGraphicsBuffers::Initialize(cContext* context)
{
	_context = context;

    const SCapabilities& caps = _context->GetSubsystem<CEngine>()->GetApplication()->GetCapabilities();

    _static_instance_buffer = CCreator::Create<XGPUBuffer>(
        EGPUBufferType::Storage,
        sizeof(SRenderInstanceGPULayout),
        caps.maxRenderStaticInstanceCount * sizeof(SRenderInstanceGPULayout),
        0
    );

    _dynamic_instance_buffer = CCreator::Create<XGPUBuffer>(
        EGPUBufferType::Storage,
        sizeof(SRenderInstanceGPULayout),
        caps.maxRenderDynamicInstanceCount * sizeof(SRenderInstanceGPULayout),
        1
    );

    _main_indirect_draw_commands_buffer = CRenderCommandProxy::CreateBuffer(
        EGPUBufferType::Storage,
        sizeof(u32) + caps.render_group_count * sizeof(SRenderIndirectDrawCommand)
    );

    _shadow_indirect_draw_commands_buffer = CRenderCommandProxy::CreateBuffer(
        EGPUBufferType::Storage,
        sizeof(u32) + caps.render_group_count * sizeof(SRenderIndirectDrawCommand)
    );

    _main_command_render_group_indices_buffer = CRenderCommandProxy::CreateBuffer(
        EGPUBufferType::Storage,
        caps.render_group_count * sizeof(u32)
    );

    _shadow_command_render_group_indices_buffer = CRenderCommandProxy::CreateBuffer(
        EGPUBufferType::Storage,
        caps.render_group_count * sizeof(u32)
    );

    _visible_render_static_indices_buffer = CRenderCommandProxy::CreateBuffer(
        EGPUBufferType::Storage,
        caps.maxRenderStaticInstanceCount * sizeof(u32)
    );

    _visible_render_dynamic_indices_buffer = CRenderCommandProxy::CreateBuffer(
        EGPUBufferType::Storage,
        caps.maxRenderDynamicInstanceCount * sizeof(u32)
    );
}

void triton::CGraphicsBuffers::Release()
{
    CRenderCommandProxy::DestroyBuffer(_visible_render_dynamic_indices_buffer);
    CRenderCommandProxy::DestroyBuffer(_visible_render_static_indices_buffer);
    CRenderCommandProxy::DestroyBuffer(_shadow_command_render_group_indices_buffer);
    CRenderCommandProxy::DestroyBuffer(_main_command_render_group_indices_buffer);
    CRenderCommandProxy::DestroyBuffer(_shadow_indirect_draw_commands_buffer);
    CRenderCommandProxy::DestroyBuffer(_main_indirect_draw_commands_buffer);
}