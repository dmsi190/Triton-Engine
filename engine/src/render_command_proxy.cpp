// render_command_proxy.cpp

#include "render_command_proxy.hpp"
#include "context.hpp"
#include "engine.hpp"
#include "synchronization.hpp"
#include "creator.hpp"

using namespace types;

void triton::CRenderCommandProxy::SetContext(cContext* context)
{
	_context = context;
}

triton::SGPUBufferResource triton::CRenderCommandProxy::CreateBuffer(
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
        WaitForRenderCommandResult<SGPUBufferResource>();
}

void triton::CRenderCommandProxy::WriteBuffer(
    const SGPUBufferResource& buffer,
    usize offset,
    const u8* data,
    usize byte_size
)
{
    _context->GetSubsystem<CEngine>()->GetRenderCommandRecorder()->PushCommand(SRenderCommand(
        ERenderCommand::WRITE_BUFFER,
        (cpuword)&buffer,
        (cpuword)offset,
        (cpuword)data,
        (cpuword)byte_size
    ));

    _context->GetSubsystem<CEngine>()->
        GetSynchronization()->
        WaitForRenderCommandResult<void*>();
}

void triton::CRenderCommandProxy::DestroyBuffer(SGPUBufferResource& buffer)
{
    _context->GetSubsystem<CEngine>()->GetRenderCommandRecorder()->PushCommand(SRenderCommand(
        ERenderCommand::DESTROY_BUFFER,
        (cpuword)&buffer
    ));

    _context->GetSubsystem<CEngine>()->
        GetSynchronization()->
        WaitForRenderCommandResult<void*>();
}

triton::SGPUShaderResource triton::CRenderCommandProxy::CreateShader(
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
        WaitForRenderCommandResult<SGPUShaderResource>();
}

void triton::CRenderCommandProxy::DestroyShader(SGPUShaderResource& shader)
{
    _context->GetSubsystem<CEngine>()->GetRenderCommandRecorder()->PushCommand(SRenderCommand(
        ERenderCommand::DESTROY_SHADER,
        (cpuword)&shader
    ));

    _context->GetSubsystem<CEngine>()->
        GetSynchronization()->
        WaitForRenderCommandResult<void*>();
}

triton::SGPUBindingGroupLayoutResource triton::CRenderCommandProxy::CreateBindingGroupLayout(
    const std::vector<SRenderBindingGroupBinding>& bindings
)
{
    _context->GetSubsystem<CEngine>()->GetRenderCommandRecorder()->PushCommand(SRenderCommand(
        ERenderCommand::CreateBindingGroupLayout,
        (cpuword)&bindings
    ));

    return _context->GetSubsystem<CEngine>()->
        GetSynchronization()->
        WaitForRenderCommandResult<SGPUBindingGroupLayoutResource>();
}

void triton::CRenderCommandProxy::DestroyBindingGroupLayout(SGPUBindingGroupLayoutResource& binding_group_layout)
{
    _context->GetSubsystem<CEngine>()->GetRenderCommandRecorder()->PushCommand(SRenderCommand(
        ERenderCommand::DestroyBindingGroupLayout,
        (cpuword)&binding_group_layout
    ));

    _context->GetSubsystem<CEngine>()->
        GetSynchronization()->
        WaitForRenderCommandResult<void*>();
}

triton::SGPUBindingGroupResource triton::CRenderCommandProxy::CreateBindingGroup(
    u32 index,
    const SGPUBindingGroupLayoutResource& layout,
    const std::vector<SRenderBindingGroupBinding>& buffers_to_bind,
    const std::vector<SRenderBindingGroupBinding>& textures_to_bind
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
        WaitForRenderCommandResult<SGPUBindingGroupResource>();
}

void triton::CRenderCommandProxy::DestroyBindingGroup(SGPUBindingGroupResource& binding_group)
{
    _context->GetSubsystem<CEngine>()->GetRenderCommandRecorder()->PushCommand(SRenderCommand(
        ERenderCommand::DestroyBindingGroup,
        (cpuword)&binding_group
    ));

    _context->GetSubsystem<CEngine>()->
        GetSynchronization()->
        WaitForRenderCommandResult<void*>();
}

triton::SGPUPipelineResource triton::CRenderCommandProxy::CreatePipeline(
    const SGPUShaderResource& shader,
    const std::vector<SGPUBindingGroupLayoutResource>& bindingGroupLayouts,
    types::boolean bUsePushConstants,
    const std::optional<std::reference_wrapper<SViewport>> viewport,
    std::optional<std::reference_wrapper<SGPURenderTargetResource>> renderTarget,
    const std::optional<std::reference_wrapper<SGPURenderPassResource>> renderPass,
    ERenderPrimitiveTopology primitiveTopology,
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
        WaitForRenderCommandResult<SGPUPipelineResource>();
}

void triton::CRenderCommandProxy::DestroyPipeline(SGPUPipelineResource& pipeline)
{
    _context->GetSubsystem<CEngine>()->GetRenderCommandRecorder()->PushCommand(SRenderCommand(
        ERenderCommand::DESTROY_PIPELINE,
        (cpuword)&pipeline
    ));

    _context->GetSubsystem<CEngine>()->
        GetSynchronization()->
        WaitForRenderCommandResult<void*>();
}

triton::SGPURenderPassResource triton::CRenderCommandProxy::CreateRenderPass(
    const SGPURenderTargetResource& render_target_resource,
    boolean clear_render_target,
    const cVector4 clear_color,
    const f32 clear_depth,
    const std::vector<ERenderResourceUsage>& src_render_target_attachments_usage,
    const std::vector<ERenderResourceUsage>& dst_render_target_attachments_usage
)
{
    _context->GetSubsystem<CEngine>()->GetRenderCommandRecorder()->PushCommand(SRenderCommand(
        ERenderCommand::CREATE_RENDER_PASS,
        (cpuword)&render_target_resource,
        (cpuword)clear_render_target,
        (cpuword)&clear_color,
        (cpuword)&clear_depth,
        (cpuword)&src_render_target_attachments_usage,
        (cpuword)&dst_render_target_attachments_usage
    ));

    return _context->GetSubsystem<CEngine>()->
        GetSynchronization()->
        WaitForRenderCommandResult<SGPURenderPassResource>();
}

void triton::CRenderCommandProxy::DestroyRenderPass(const SGPURenderPassResource& render_pass)
{
    _context->GetSubsystem<CEngine>()->GetRenderCommandRecorder()->PushCommand(SRenderCommand(
        ERenderCommand::DESTROY_RENDER_PASS,
        (cpuword)&render_pass
    ));

    _context->GetSubsystem<CEngine>()->
        GetSynchronization()->
        WaitForRenderCommandResult<void*>();
}

triton::SGPURenderTargetResource triton::CRenderCommandProxy::CreateRenderTarget(
    const std::vector<std::reference_wrapper<SGPUTextureResource>>& color_attachments,
    const SGPUTextureResource& depth_attachment
)
{
    _context->GetSubsystem<CEngine>()->GetRenderCommandRecorder()->PushCommand(SRenderCommand(
        ERenderCommand::CREATE_RENDER_TARGET,
        (cpuword)color_attachments.size(),
        (cpuword)color_attachments.data(),
        (cpuword)&depth_attachment
    ));

    return _context->GetSubsystem<CEngine>()->
        GetSynchronization()->
        WaitForRenderCommandResult<SGPURenderTargetResource>();
}

void triton::CRenderCommandProxy::DestroyRenderTarget(const SGPURenderTargetResource& render_target)
{
    _context->GetSubsystem<CEngine>()->GetRenderCommandRecorder()->PushCommand(SRenderCommand(
        ERenderCommand::DESTROY_RENDER_TARGET,
        (cpuword)&render_target
    ));

    _context->GetSubsystem<CEngine>()->
        GetSynchronization()->
        WaitForRenderCommandResult<void*>();
}

triton::SGPUTextureResource triton::CRenderCommandProxy::CreateTexture(
    boolean create_sampler,
    ETextureFormat format,
    ETextureUsageBit usage_mask,
    ETextureDimension dimension,
    const cVector3& size
)
{
    cVector3* size_ptr = CCreator::Create<cVector3>(size);

    _context->GetSubsystem<CEngine>()->GetRenderCommandRecorder()->PushCommand(SRenderCommand(
        ERenderCommand::CREATE_TEXTURE,
        (cpuword)create_sampler,
        (cpuword)format,
        (cpuword)usage_mask,
        (cpuword)ETextureDimension::Texture2D,
        (cpuword)size_ptr
    ));

    SGPUTextureResource result = _context->GetSubsystem<CEngine>()->
        GetSynchronization()->
        WaitForRenderCommandResult<SGPUTextureResource>();

    CCreator::Destroy<cVector3>(size_ptr);
}

void triton::CRenderCommandProxy::WriteTexture(const SGPUTextureResource& texture, const void* data)
{
    _context->GetSubsystem<CEngine>()->GetRenderCommandRecorder()->PushCommand(SRenderCommand(
        ERenderCommand::WRITE_TEXTURE,
        (cpuword)&texture,
        (cpuword)data
    ));

    _context->GetSubsystem<CEngine>()->
        GetSynchronization()->
        WaitForRenderCommandResult<void*>();
}

void triton::CRenderCommandProxy::GenerateTextureMips(const SGPUTextureResource& texture)
{
    _context->GetSubsystem<CEngine>()->GetRenderCommandRecorder()->PushCommand(SRenderCommand(
        ERenderCommand::GENERATE_TEXTURE_MIPS,
        (cpuword)&texture
    ));

    _context->GetSubsystem<CEngine>()->
        GetSynchronization()->
        WaitForRenderCommandResult<void*>();
}

void triton::CRenderCommandProxy::DestroyTexture(const SGPUTextureResource& texture)
{
    _context->GetSubsystem<CEngine>()->GetRenderCommandRecorder()->PushCommand(SRenderCommand(
        ERenderCommand::DESTROY_TEXTURE,
        (cpuword)&texture
    ));

    _context->GetSubsystem<CEngine>()->
        GetSynchronization()->
        WaitForRenderCommandResult<void*>();
}

void triton::CRenderCommandProxy::Dispatch(
    const SGPUPipelineResource& pipeline,
    const std::vector<SGPUBindingGroupResource>& binding_groups,
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

void triton::CRenderCommandProxy::FinalizeSwapchain(const SGPUTextureResource& present_texture)
{
    _context->GetSubsystem<CEngine>()->GetRenderCommandRecorder()->PushCommand(SRenderCommand(
        ERenderCommand::FinalizeSwapchain,
        (cpuword)&present_texture
    ));

    _context->GetSubsystem<CEngine>()->
        GetSynchronization()->
        WaitForRenderCommandResult<void*>();
}

void triton::CRenderCommandProxy::ReleaseSwapchainResources()
{
    _context->GetSubsystem<CEngine>()->GetRenderCommandRecorder()->PushCommand(SRenderCommand(
        ERenderCommand::ReleaseSwapchainResources
    ));

    _context->GetSubsystem<CEngine>()->
        GetSynchronization()->
        WaitForRenderCommandResult<void*>();
}