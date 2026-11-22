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

        SGPUBindingGroupLayoutResource  _gpuBindingGroupLayout = SGPUBindingGroupLayoutResource::Invalid();
        SGPUBindingGroupResource        _gpuBindingGroup = SGPUBindingGroupResource::Invalid();
        XTexture*                       _diffuseTexture;
        XTexture*                       _normalTexture;
        XTexture*                       _roughnessTexture;
        XTexture*                       _metallicTexture;
        cVector4                        _diffuseColor = cVector4(1.0f);
        cVector4                        _specularColor = cVector4(1.0f);
        types::f32                      _shininess = 0.0f;

    public:
        explicit XMaterial(
            const SMemAlloc& mem_alloc,
            cContext* context,
            const std::optional<XTexture*>& diffuseTexture,
            const std::optional<XTexture*>& normalTexture,
            const std::optional<XTexture*>& roughnessTexture,
            const std::optional<XTexture*>& metallicTexture,
            const cVector4& diffuseColor,
            const cVector4& specularColor,
            types::f32 shininess
        );

        ~XMaterial() override = default;

        inline const SGPUBindingGroupLayoutResource& GetBindingGroupLayout() const { return _gpuBindingGroupLayout; }

        inline const SGPUBindingGroupResource& GetBindingGroup() const { return _gpuBindingGroup; }

        inline XTexture* GetDiffuseTexture() const { return _diffuseTexture; }

        inline XTexture* GetNormalTexture() const { return _normalTexture; }

        inline XTexture* GetRoughnessTexture() const { return _roughnessTexture; }

        inline XTexture* GetMetallicTexture() const { return _metallicTexture; }

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