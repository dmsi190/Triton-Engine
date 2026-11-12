// material.cpp

#include <vector>
#include "material.hpp"
#include "material_pool.hpp"
#include "render_binding_group_binding_struct.hpp"
#include "shader_stage_bit_enum.hpp"

using namespace types;

triton::XMaterial::XMaterial(
    cContext* context,
    types::s32 poolIndex,
    const std::optional<SHandle>& diffuseTexture,
    const std::optional<SHandle>& normalTexture,
    const std::optional<SHandle>& roughnessTexture,
    const std::optional<SHandle>& metallicTexture,
    const cVector4& diffuseColor,
    const cVector4& specularColor,
    types::f32 shininess
) :
    iObject(context, poolIndex),
    _diffuseTexture(diffuseTexture.has_value() ? diffuseTexture.value() : SHandle()),
    _normalTexture(normalTexture.has_value() ? normalTexture.value() : SHandle()),
    _roughnessTexture(roughnessTexture.has_value() ? roughnessTexture.value() : SHandle()),
    _metallicTexture(metallicTexture.has_value() ? metallicTexture.value() : SHandle()),
    _diffuseColor(diffuseColor),
    _specularColor(specularColor),
    _shininess(shininess)
{
    CMaterialPool* pool = _context->GetPool<CMaterialPool>();
    pool->WriteToStaging(
        pool->GetPackedIndex(poolIndex),
        *this
    );

    std::vector<SBindingGroupBinding>* bindings =
        CObjectAllocator::Create<std::vector<SBindingGroupBinding>>(64);

    bindings->push_back({ 0, EBindingGroupBindingType::TextureSampler, (dword)EShaderStageBit::Pixel, nullptr });
    bindings->push_back({ 1, EBindingGroupBindingType::TextureSampler, (dword)EShaderStageBit::Pixel, nullptr });
    bindings->push_back({ 2, EBindingGroupBindingType::TextureSampler, (dword)EShaderStageBit::Pixel, nullptr });
    bindings->push_back({ 3, EBindingGroupBindingType::TextureSampler, (dword)EShaderStageBit::Pixel, nullptr });

    _context->GetSubsystem<CEngine>()->GetRenderCommandRecorder()->PushCommand(SRenderCommand(
        ERenderCommand::CreateBindingGroupLayout,
        (cpuword)bindings
    ));

    _gpuBindingGroupLayout =
        _context->GetSubsystem<CEngine>()->
        GetSynchronization()->
        WaitForRenderCommandResult<CGPUBindingGroupLayoutResource>();

    CObjectAllocator::Destroy<std::vector<SBindingGroupBinding>>(bindings);

    std::vector<SBindingGroupBinding>* buffersToBind =
        CObjectAllocator::Create<std::vector<SBindingGroupBinding>>(64);
    std::vector<SBindingGroupBinding>* texturesToBind =
        CObjectAllocator::Create<std::vector<SBindingGroupBinding>>(64);

    if (diffuseTexture.has_value())
        texturesToBind->push_back(
            {
                0,
                EBindingGroupBindingType::TextureSampler,
                (dword)EShaderStageBit::Pixel,
                (CGPUResource*)&(*_context->GetPool<PTexturePool>()->Get(diffuseTexture.value())).get().GetGPUResource()
            }
        );
    if (normalTexture.has_value())
        texturesToBind->push_back(
            {
                1,
                EBindingGroupBindingType::TextureSampler,
                (dword)EShaderStageBit::Pixel,
                (CGPUResource*)&(*_context->GetPool<PTexturePool>()->Get(normalTexture.value())).get().GetGPUResource()
            }
        );
    if (roughnessTexture.has_value())
        texturesToBind->push_back(
            {
                 2,
                 EBindingGroupBindingType::TextureSampler,
                 (dword)EShaderStageBit::Pixel,
                 (CGPUResource*)&(*_context->GetPool<PTexturePool>()->Get(roughnessTexture.value())).get().GetGPUResource()
            }
        );
    if (metallicTexture.has_value())
        texturesToBind->push_back(
            {
                3,
                EBindingGroupBindingType::TextureSampler,
                (dword)EShaderStageBit::Pixel,
                (CGPUResource*)&(*_context->GetPool<PTexturePool>()->Get(metallicTexture.value())).get().GetGPUResource()
            }
        );

    _context->GetSubsystem<CEngine>()->GetRenderCommandRecorder()->PushCommand(SRenderCommand(
        ERenderCommand::CreateBindingGroup,
        (cpuword)1,
        (cpuword)&_gpuBindingGroupLayout,
        (cpuword)buffersToBind,
        (cpuword)texturesToBind
    ));

    _gpuBindingGroup =
        _context->GetSubsystem<CEngine>()->
        GetSynchronization()->
        WaitForRenderCommandResult<CGPUBindingGroupResource>();

    CObjectAllocator::Destroy<std::vector<SBindingGroupBinding>>(texturesToBind);
    CObjectAllocator::Destroy<std::vector<SBindingGroupBinding>>(buffersToBind);
}

void triton::XMaterial::SetShininess(types::f32 shininess)
{
    _shininess = shininess;

    CMaterialPool* pool = _context->GetPool<CMaterialPool>();
    pool->WriteToStaging(
        pool->GetPackedIndex(_poolIndex),
        *this
    );
}