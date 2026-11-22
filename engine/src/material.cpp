// material.cpp

#include <vector>
#include "material.hpp"
#include "render_binding_group_binding_struct.hpp"
#include "shader_stage_bit_enum.hpp"
#include "creator.hpp"
#include "render_command_proxy.hpp"

using namespace types;

triton::XMaterial::XMaterial(
    const SMemAlloc& mem_alloc,
    cContext* context,
    const std::optional<XTexture*>& diffuseTexture,
    const std::optional<XTexture*>& normalTexture,
    const std::optional<XTexture*>& roughnessTexture,
    const std::optional<XTexture*>& metallicTexture,
    const cVector4& diffuseColor,
    const cVector4& specularColor,
    types::f32 shininess
) :
    iObject(mem_alloc, context),
    _diffuseTexture(diffuseTexture.has_value() ? diffuseTexture.value() : nullptr),
    _normalTexture(normalTexture.has_value() ? normalTexture.value() : nullptr),
    _roughnessTexture(roughnessTexture.has_value() ? roughnessTexture.value() : nullptr),
    _metallicTexture(metallicTexture.has_value() ? metallicTexture.value() : nullptr),
    _diffuseColor(diffuseColor),
    _specularColor(specularColor),
    _shininess(shininess)
{
    std::vector<SRenderBindingGroupBinding>* bindings =
        new std::vector<SRenderBindingGroupBinding>();

    bindings->push_back({ 0, ERenderBindingGroupBindingType::TextureSampler, EShaderStageBit::Pixel, nullptr });
    bindings->push_back({ 1, ERenderBindingGroupBindingType::TextureSampler, EShaderStageBit::Pixel, nullptr });
    bindings->push_back({ 2, ERenderBindingGroupBindingType::TextureSampler, EShaderStageBit::Pixel, nullptr });
    bindings->push_back({ 3, ERenderBindingGroupBindingType::TextureSampler, EShaderStageBit::Pixel, nullptr });

    _gpuBindingGroupLayout = CRenderCommandProxy::CreateBindingGroupLayout(*bindings);

    delete bindings;

    std::vector<SRenderBindingGroupBinding>* buffersToBind =
        new std::vector<SRenderBindingGroupBinding>();
    std::vector<SRenderBindingGroupBinding>* texturesToBind =
        new std::vector<SRenderBindingGroupBinding>();

    if (diffuseTexture.has_value())
        texturesToBind->push_back(
            {
                0,
                ERenderBindingGroupBindingType::TextureSampler,
                EShaderStageBit::Pixel,
                (SGPUResource*)&diffuseTexture.value()->GetGPUResource()
            }
        );
    if (normalTexture.has_value())
        texturesToBind->push_back(
            {
                1,
                ERenderBindingGroupBindingType::TextureSampler,
                EShaderStageBit::Pixel,
                (SGPUResource*)&normalTexture.value()->GetGPUResource()
            }
        );
    if (roughnessTexture.has_value())
        texturesToBind->push_back(
            {
                 2,
                 ERenderBindingGroupBindingType::TextureSampler,
                 EShaderStageBit::Pixel,
                 (SGPUResource*)&roughnessTexture.value()->GetGPUResource()
            }
        );
    if (metallicTexture.has_value())
        texturesToBind->push_back(
            {
                3,
                ERenderBindingGroupBindingType::TextureSampler,
                EShaderStageBit::Pixel,
                (SGPUResource*)&metallicTexture.value()->GetGPUResource()
            }
        );

    _gpuBindingGroup = CRenderCommandProxy::CreateBindingGroup(
        1,
        _gpuBindingGroupLayout,
        *buffersToBind,
        *texturesToBind
    );

    delete texturesToBind;
    delete buffersToBind;
}

void triton::XMaterial::SetShininess(types::f32 shininess)
{
}