// model_backend.hpp

#pragma once

#include <string>
#include <optional>
#include "backend.hpp"
#include "object.hpp"
#include "model_formats.hpp"
#include "vertex.hpp"
#include "material.hpp"
#include "skeleton.hpp"
#include "animation.hpp"
#include "vertex_buffer_format.hpp"

namespace triton
{
    struct SVertex;

    struct SModelData
    {
        EVertexBufferFormat vertexDataFormat = EVertexBufferFormat::Unknown;
        SMemAlloc<SRigidVertexGPULayout> rigidVertexData;
        SMemAlloc<SSkinnedVertexGPULayout> skinnedVertexData;
        SMemAlloc<types::u32> indexData;
        types::usize vertexCount = 0;
        types::usize indexCount = 0;
        std::vector<XMaterial*> materials = {};
        XSkeleton* skeleton = nullptr;
        std::vector<XAnimation*> animations = {};
    };

    class IModelBackend : public iBackend
    {
        TRITON_CLASS_NAME(IModelBackend)

    public:
        explicit IModelBackend(cContext* context) : iBackend(context) {}
        ~IModelBackend() override = default;

        virtual std::optional<SModelData> CreateModel(
            const std::string& modelFolderPath,
            const std::string& modelLocalPath,
            EVertexBufferFormat vertexDataFormat
        ) = 0;

        virtual std::optional<SModelData> CreateModel(
            const types::u8* byteData,
            types::usize byteSize,
            EVertexBufferFormat vertexDataFormat
        ) = 0;

        virtual void DestroyModel(SModelData& model) = 0;
    };
}