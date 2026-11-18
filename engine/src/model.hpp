// model.hpp

#pragma once

#include <vector>
#include <filesystem>
#include "../../tools/tasset/src/tasset/tasset.hpp"
#include "object.hpp"
#include "vertex.hpp"
#include "animation.hpp"
#include "material.hpp"
#include "model3d_backend.hpp"
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
        TRITON_CLASS_NAME(XModel)

        SModel3DData            _data;
        asset::CModel3DAsset*   _asset = nullptr;
        EVertexBufferFormat     _vertexDataFormat = EVertexBufferFormat::Unknown;
        EModelFileType          _fileType = EModelFileType::Unknown;

    public:
        explicit XModel(const SMemoryAllocation& mem_alloc, cContext* context) : iObject(mem_alloc, context) {}

        explicit XModel(
            const SMemoryAllocation& mem_alloc,
            cContext* context,
            EVertexBufferFormat vertexDataFormat,
            EModelFileType fileType,
            const std::filesystem::path& filePath
        );

        ~XModel() override;

        inline EVertexBufferFormat GetVertexFormat() { return _vertexDataFormat; }

        inline const SRigidVertexGPULayout* GetRigidVertices() const { return _data.rigidVertexData; }

        inline const SSkinnedVertexGPULayout* GetSkinnedVertices() const { return _data.skinnedVertexData; }

        inline const types::u32* GetIndices() const { return _data.indexData; }

        inline types::usize GetVertexCount() const { return _data.vertexCount; }

        inline types::usize GetIndexCount() const { return _data.indexCount; }

        inline XMaterial* GetMaterial() const
        {
            return _data.materials.size() > 0 ? _data.materials.at(0) : SHandle();
        }
	};
}