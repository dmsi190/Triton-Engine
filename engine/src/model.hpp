// model.hpp

#pragma once

#include <vector>
#include <filesystem>
#include "../../tools/tasset/src/tasset/tasset.hpp"
#include "object.hpp"
#include "vertex.hpp"
#include "animation.hpp"
#include "material.hpp"
#include "model_backend.hpp"
#include "types.hpp"

namespace triton
{
    class cContext;

    enum class EModelFileType
    {
        Unknown,
        Raw,
        Asset
    };

	class XModel : public iObject
	{
        TRITON_OBJECT(XModel)

        SModelData              _data;
        asset::CModel3DAsset*   _asset = nullptr;
        EVertexBufferFormat     _vertexDataFormat = EVertexBufferFormat::Unknown;
        EModelFileType          _fileType = EModelFileType::Unknown;

    public:
        explicit XModel(
            const SMemAlloc& mem_alloc,
            cContext* context,
            EVertexBufferFormat vertexDataFormat,
            EModelFileType fileType,
            const std::filesystem::path& filePath
        );

        ~XModel() override;

        inline EVertexBufferFormat GetVertexFormat() { return _vertexDataFormat; }

        inline const SRigidVertexGPULayout* GetRigidVertices() const { return (const SRigidVertexGPULayout*)_data.rigidVertexData.mem; }

        inline const SSkinnedVertexGPULayout* GetSkinnedVertices() const { return (const SSkinnedVertexGPULayout*)_data.skinnedVertexData.mem; }

        inline const types::u32* GetIndices() const { return (const types::u32*)_data.indexData.mem; }

        inline types::usize GetVertexCount() const { return _data.vertexCount; }

        inline types::usize GetIndexCount() const { return _data.indexCount; }

        inline XMaterial* GetMaterial() const
        {
            return _data.materials.size() > 0 ? _data.materials.at(0) : nullptr;
        }
	};
}