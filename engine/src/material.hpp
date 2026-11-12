// material.hpp

#pragma once

#include <optional>
#include "object.hpp"
#include "math.hpp"
#include "texture.hpp"
#include "gpu_binding_group_resource.hpp"
#include "gpu_binding_group_layout_resource.hpp"

namespace triton
{
    class XMaterial : public iObject
    {
        TRITON_OBJECT(XMaterial)

        CGPUBindingGroupLayoutResource  _gpuBindingGroupLayout = CGPUBindingGroupLayoutResource::Invalid();
        CGPUBindingGroupResource        _gpuBindingGroup = CGPUBindingGroupResource::Invalid();
        SHandle                         _diffuseTexture;
        SHandle                         _normalTexture;
        SHandle                         _roughnessTexture;
        SHandle                         _metallicTexture;
        cVector4                        _diffuseColor = cVector4(1.0f);
        cVector4                        _specularColor = cVector4(1.0f);
        types::f32                      _shininess = 0.0f;

    public:
        explicit XMaterial(cContext* context, types::s32 poolIndex) : iObject(context, poolIndex) {}

        explicit XMaterial(
            cContext* context,
            types::s32 poolIndex,
            const std::optional<SHandle>& diffuseTexture,
            const std::optional<SHandle>& normalTexture,
            const std::optional<SHandle>& roughnessTexture,
            const std::optional<SHandle>& metallicTexture,
            const cVector4& diffuseColor,
            const cVector4& specularColor,
            types::f32 shininess
        );

        ~XMaterial() override = default;

        inline const CGPUBindingGroupLayoutResource& GetBindingGroupLayout() const { return _gpuBindingGroupLayout; }

        inline const CGPUBindingGroupResource& GetBindingGroup() const { return _gpuBindingGroup; }

        inline const SHandle& GetDiffuseTexture() const { return _diffuseTexture; }

        inline const SHandle& GetNormalTexture() const { return _normalTexture; }

        inline const SHandle& GetRoughnessTexture() const { return _roughnessTexture; }

        inline const SHandle& GetMetallicTexture() const { return _metallicTexture; }

        inline const cVector4& GetDiffuseColor() const { return _diffuseColor; }

        inline const cVector4& GetSpecularColor() const { return _specularColor; }

        inline types::f32 GetShininess() const { return _shininess; }

        void SetShininess(types::f32 shininess);

        struct TGPULayout
        {
            cVector4 diffuseColor = cVector4(0.0f);
            cVector4 specularColor = cVector4(0.0f);
            types::f32 shininess = 0.0f;
        };
    };
}