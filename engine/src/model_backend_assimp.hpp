// model_backend_assimp.hpp

#pragma once

#include <optional>
#include <assimp/Importer.hpp>
#include <assimp/scene.h>
#include <assimp/mesh.h>
#include <assimp/postprocess.h>
#include <vector>
#include <unordered_map>
#include "model_backend.hpp"
#include "vertex.hpp"
#include "material.hpp"
#include "skeleton.hpp"
#include "animation.hpp"

namespace triton
{
    class cContext;
    class cVector3;
    struct SSkeletonBone;
    struct SAnimation;
    struct SSkeletonData;

    struct SModelMaterialData final
    {
        types::s32 absoluteMaterialIndex = -1;
        std::string diffuseTextureFilePath = {};
        std::string normalTextureFilePath = {};
        std::string roughnessTextureFilePath = {};
        std::string metallicTextureFilePath = {};
        types::boolean bIsDiffuseEmbedded = types::K_FALSE;
        types::boolean bIsNormalEmbedded = types::K_FALSE;
        types::boolean bIsRoughnessEmbedded = types::K_FALSE;
        types::boolean bIsMetallicEmbedded = types::K_FALSE;
    };

    struct SBoneWeight final
    {
        types::usize boneIndex = 0;
        types::f32 weight = 0.0f;
    };

    class XModelBackendAssimp final : public IModelBackend
    {
    public:
        explicit XModelBackendAssimp(cContext* context) : IModelBackend(context) {}
        ~XModelBackendAssimp() override = default;

        std::optional<SModelData> CreateModel(
            const std::string& modelFolderPath,
            const std::string& modelLocalPath,
            EVertexBufferFormat vertexDataFormat
        ) override final;

        std::optional<SModelData> CreateModel(
            const types::u8* byteData,
            types::usize byteSize,
            EVertexBufferFormat vertexDataFormat
        ) override final;

        void DestroyModel(SModelData& model) override final;

    private:
        void ImportScene(
            Assimp::Importer& importer,
            const aiScene*& scene,
            const std::string& filePath
        );

        void ImportScene(
            Assimp::Importer& importer,
            const aiScene*& scene,
            const types::u8* byteData,
            types::usize byteSize
        );
        
        std::optional<SModelData> ParseImportedScene(
            cContext* context,
            const Assimp::Importer& importer,
            const aiScene* scene,
            EVertexBufferFormat vertexDataFormat,
            const std::string& modelFolderPath = ""
        );
        
        void CountVerticesIndices(
            const aiScene* scene,
            types::usize& vertexCount,
            types::usize& indexCount,
            std::vector<types::usize>& indexOffsets
        );
        
        void AllocateVertexIndexBuffers(
            EVertexBufferFormat vertexDataFormat,
            SMemAlloc<SRigidVertexGPULayout>& rigidVertexData,
            SMemAlloc<SSkinnedVertexGPULayout>& skinnedVertexData,
            SMemAlloc<u32>& indices,
            usize vertexCount,
            usize indexCount
        );
        
        void AllocateTempBitangentBuffer(SMemAlloc<cVector3>& bitangents, usize vertexCount);
        
        void ParseVertexData(
            const aiScene* scene,
            EVertexBufferFormat vertexDataFormat,
            const SMemAlloc<SRigidVertexGPULayout>& rigidVertexData,
            const SMemAlloc<SSkinnedVertexGPULayout>& skinnedVertexData,
            const SMemAlloc<cVector3>& bitangents
        );
        
        void CalculateHandedness(
            EVertexBufferFormat vertexDataFormat,
            const SMemAlloc<SRigidVertexGPULayout>& rigidVertexData,
            const SMemAlloc<SSkinnedVertexGPULayout>& skinnedVertexData,
            const SMemAlloc<cVector3>& bitangents,
            usize vertexCount
        );
        
        void ParseIndexData(
            const aiScene* scene,
            const SMemAlloc<types::u32>& indexData,
            const std::vector<types::usize>& indexOffsets
        );
        
        void ParseMaterialData(const aiScene* scene, std::vector<SModelMaterialData>& materials);
        
        void CreateMaterials(
            cContext* context,
            const std::string& modelFolderPath,
            const std::vector<SModelMaterialData>& materials,
            std::vector<XMaterial*>& modelMaterials,
            const aiScene* scene
        );

        void DeallocateTempBitangentBuffer(SMemAlloc<cVector3>& bitangents);
        
        void CreateBones(
            const aiScene* scene,
            usize vertexCount,
            std::unordered_map<std::string, usize>& boneIndices,
            std::vector<SSkeletonBone>& bones,
            std::vector<std::vector<SBoneWeight>>& vertexWeights
        );

        void FinalizeBoneWeights(
            cContext* context,
            const SMemAlloc<SSkinnedVertexGPULayout>& vertexData,
            types::usize vertexCount,
            std::vector<std::vector<SBoneWeight>>& vertexWeights
        );
        
        types::boolean AccumulateRootTransform(
            const aiNode* node,
            aiNode*& boneRootNode,
            const aiMatrix4x4& parentRootTransform,
            aiMatrix4x4& accumulatedRootTransform,
            std::unordered_map<std::string, types::usize>& boneIndices
        );

        void CreateBoneHierarchy(
            const aiNode* node,
            types::s32 parentBone,
            std::unordered_map<std::string, types::usize>& boneIndices,
            std::vector<SSkeletonBone>& bones
        );

        void CreateSkeleton(
            cContext* context,
            XSkeleton*& modelSkeleton,
            const std::vector<SSkeletonBone>& bones,
            const aiMatrix4x4& accumulatedRootTransform
        );

        void CreateAnimations(
            cContext* context,
            const aiScene* scene,
            const std::unordered_map<std::string, usize>& boneIndices,
            std::vector<XAnimation*>& modelAnimations
        );
        
        std::optional<XTexture*> CreateTexture(
            cContext* context,
            ETextureFormat dataFormat,
            const std::string& modelFolderPath,
            const std::string& textureFilePath,
            types::boolean bIsEmbedded,
            const aiTexture* texture
        );
        
        std::optional<XTexture*> CreateTextureFromModelData(
            cContext* context,
            ETextureFormat dataFormat,
            const std::string& textureFilePath,
            const aiTexture* texture
        );
        
        std::optional<XTexture*> CreateTextureFromFile(
            cContext* context,
            ETextureFormat dataFormat,
            const std::string& modelFolderPath,
            const std::string& textureFilePath
        );
        
        SModelData PrepareResult(
            EVertexBufferFormat vertexDataFormat,
            const SMemAlloc<SRigidVertexGPULayout>& rigidVertexData,
            const SMemAlloc<SSkinnedVertexGPULayout>& skinnedVertexData,
            const SMemAlloc<types::u32>& indexData,
            types::usize vertexCount,
            types::usize indexCount,
            const std::vector<XMaterial*>& modelMaterials,
            XSkeleton* modelSkeleton,
            const std::vector<XAnimation*>& modelAnimations
        );

        cMatrix4 ConvertMatrix(const aiMatrix4x4& mat);
    };
}