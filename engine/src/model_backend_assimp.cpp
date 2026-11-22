// model_backend_assimp.cpp

#include <filesystem>
#include "model_backend_assimp.hpp"
#include "model.hpp"
#include "context.hpp"
#include "math.hpp"
#include "skeleton_bone.hpp"
#include "animation.hpp"
#include "image_decoder.hpp"
#include "graphics_texture_usage_bit_enum.hpp"
#include "log.hpp"
#include "creator.hpp"
#include "application.hpp"

using namespace types;

std::optional<triton::SModelData> triton::XModelBackendAssimp::CreateModel(
    const std::string& modelFolderPath,
    const std::string& modelLocalPath,
    EVertexBufferFormat vertexDataFormat
)
{
    Assimp::Importer importer;
    const aiScene* scene = nullptr;

    ImportScene(
        importer,
        scene,
        modelFolderPath + "/" + modelLocalPath
    );

    return ParseImportedScene(
        _context,
        importer,
        scene,
        vertexDataFormat,
        modelFolderPath
    );
}

std::optional<triton::SModelData> triton::XModelBackendAssimp::CreateModel(
    const types::u8* byteData,
    const types::usize byteSize,
    EVertexBufferFormat vertexDataFormat
)
{
    Assimp::Importer importer;
    const aiScene* scene = nullptr;

    ImportScene(
        importer,
        scene,
        byteData,
        byteSize
    );

    return ParseImportedScene(
        _context,
        importer,
        scene,
        vertexDataFormat
    );
}

std::optional<triton::SModelData> triton::XModelBackendAssimp::ParseImportedScene(
    cContext* context,
    const Assimp::Importer& importer,
    const aiScene* scene,
    EVertexBufferFormat vertexDataFormat,
    const std::string& modelFolderPath
)
{
    if (!scene)
        return std::nullopt;

    usize vertexCount = 0;
    usize indexCount = 0;
    std::vector<usize> indexOffsets = {};
    SMemAlloc rigidVertexData;
    SMemAlloc skinnedVertexData;
    SMemAlloc indexData;
    SMemAlloc bitangents;
    std::vector<SModelMaterialData> materials = {};
    std::vector<XMaterial*> modelMaterials = {};
    std::unordered_map<std::string, types::usize> boneIndices = {};
    std::vector<SSkeletonBone> bones = {};
    std::vector<std::vector<SBoneWeight>> vertexWeights;
    XSkeleton* modelSkeleton = nullptr;
    usize boneOffset = 0;
    std::vector<XAnimation*> modelAnimations = {};
    aiNode* boneRootNode = nullptr;
    aiMatrix4x4 parentRootTransform = aiMatrix4x4();
    aiMatrix4x4 accumulatedRootTransform = aiMatrix4x4();

    CountVerticesIndices(scene, vertexCount, indexCount, indexOffsets);

    AllocateVertexIndexBuffers(
        vertexDataFormat,
        rigidVertexData,
        skinnedVertexData,
        indexData,
        vertexCount,
        indexCount
    );
    AllocateTempBitangentBuffer(bitangents, vertexCount);
    ParseVertexData(
        scene,
        vertexDataFormat,
        rigidVertexData,
        skinnedVertexData,
        bitangents
    );
    CalculateHandedness(
        vertexDataFormat,
        rigidVertexData,
        skinnedVertexData,
        bitangents,
        vertexCount
    );
    DeallocateTempBitangentBuffer(bitangents);

    // TODO: encapsulate "is_directory()" check to proper file system backend
    if (std::filesystem::is_directory(modelFolderPath))
    {
        ParseMaterialData(scene, materials);
        CreateMaterials(
            context,
            modelFolderPath,
            materials,
            modelMaterials,
            scene
        );
    }

    ParseIndexData(scene, indexData, indexOffsets);

    if (vertexDataFormat == EVertexBufferFormat::Skinned)
    {
        CreateBones(scene, vertexCount, boneIndices, bones, vertexWeights);
        FinalizeBoneWeights(context, skinnedVertexData, vertexCount, vertexWeights);
        AccumulateRootTransform(
            scene->mRootNode,
            boneRootNode,
            parentRootTransform,
            accumulatedRootTransform,
            boneIndices
        );
        CreateBoneHierarchy(scene->mRootNode, -1, boneIndices, bones);
        CreateSkeleton(
            context,
            modelSkeleton,
            bones,
            accumulatedRootTransform
        );
        CreateAnimations(
            context,
            scene,
            boneIndices,
            modelAnimations
        );
    }

    return PrepareResult(
        vertexDataFormat,
        rigidVertexData,
        skinnedVertexData,
        indexData,
        vertexCount,
        indexCount,
        modelMaterials,
        modelSkeleton,
        modelAnimations
    );
}

void triton::XModelBackendAssimp::DestroyModel(SModelData& model)
{
    model.vertexCount = 0;
    model.indexCount = 0;

    CMemoryAllocator::Deallocate(model.indexData);
    CMemoryAllocator::Deallocate(model.skinnedVertexData);
    CMemoryAllocator::Deallocate(model.rigidVertexData);
}

void triton::XModelBackendAssimp::ImportScene(
    Assimp::Importer& importer,
    const aiScene*& scene,
    const std::string& filePath
)
{
    importer.SetPropertyInteger(AI_CONFIG_PP_RVC_FLAGS, aiComponent_NORMALS);

    scene = importer.ReadFile(
        filePath.c_str(),
        // aiProcess_RemoveComponent |
        aiProcess_GenSmoothNormals |
        aiProcess_JoinIdenticalVertices |
        aiProcess_CalcTangentSpace |
        aiProcess_Triangulate |
        aiProcess_ImproveCacheLocality
    );

    if (!scene)
        Print("Error: can't load 3d model from file '" + filePath + "'");
}

void triton::XModelBackendAssimp::ImportScene(
    Assimp::Importer& importer,
    const aiScene*& scene,
    const types::u8* byteData,
    types::usize byteSize
)
{
    importer.SetPropertyInteger(AI_CONFIG_PP_RVC_FLAGS, aiComponent_NORMALS);

    scene = importer.ReadFileFromMemory(
        byteData,
        byteSize,
        // aiProcess_RemoveComponent |
        aiProcess_GenSmoothNormals |
        aiProcess_JoinIdenticalVertices |
        aiProcess_CalcTangentSpace |
        aiProcess_Triangulate |
        aiProcess_ImproveCacheLocality
    );

    if (!scene)
        Print("Error: can't load 3d model from byte data");
}

void triton::XModelBackendAssimp::CountVerticesIndices(
    const aiScene* scene,
    usize& vertexCount,
    usize& indexCount,
    std::vector<usize>& indexOffsets
)
{
    for (usize meshIndex = 0; meshIndex < scene->mNumMeshes; meshIndex++)
    {
        indexOffsets.push_back(vertexCount);
        vertexCount += scene->mMeshes[meshIndex]->mNumVertices;
    }

    for (usize meshIndex = 0; meshIndex < scene->mNumMeshes; meshIndex++)
        if (scene->mMeshes[meshIndex]->HasFaces())
            for (usize faceIndex = 0; faceIndex < scene->mMeshes[meshIndex]->mNumFaces; faceIndex++)
                indexCount += scene->mMeshes[meshIndex]->mFaces[faceIndex].mNumIndices;
}

void triton::XModelBackendAssimp::AllocateVertexIndexBuffers(
    EVertexBufferFormat vertexDataFormat,
    SMemAlloc& rigidVertexData,
    SMemAlloc& skinnedVertexData,
    SMemAlloc& indices,
    usize vertexCount,
    usize indexCount
)
{
    if (vertexDataFormat == EVertexBufferFormat::Rigid)
        rigidVertexData = CMemoryAllocator::Allocate(
            vertexCount * sizeof(SRigidVertexGPULayout)
        );
    else if (vertexDataFormat == EVertexBufferFormat::Skinned)
        skinnedVertexData = CMemoryAllocator::Allocate(
            vertexCount * sizeof(SSkinnedVertexGPULayout)
        );

    indices = CMemoryAllocator::Allocate(indexCount * sizeof(u32));
}

void triton::XModelBackendAssimp::AllocateTempBitangentBuffer(SMemAlloc& bitangents, usize vertexCount)
{
    bitangents = CMemoryAllocator::Allocate(vertexCount * sizeof(cVector3));
}

void triton::XModelBackendAssimp::ParseVertexData(
    const aiScene* scene,
    EVertexBufferFormat vertexDataFormat,
    const SMemAlloc& rigidVertexData,
    const SMemAlloc& skinnedVertexData,
    const SMemAlloc& bitangents_memory
)
{
    SRigidVertexGPULayout* rigidVertices = (SRigidVertexGPULayout*)rigidVertexData.mem;
    SSkinnedVertexGPULayout* skinnedVertices = (SSkinnedVertexGPULayout*)skinnedVertexData.mem;
    cVector3* bitangents = (cVector3*)bitangents_memory.mem;

    usize globalVertexIndex = 0;
    for (usize meshIndex = 0; meshIndex < scene->mNumMeshes; meshIndex++)
    {
        const aiMesh* mesh = scene->mMeshes[meshIndex];
        for (usize vertexIndex = 0; vertexIndex < mesh->mNumVertices; vertexIndex++)
        {
            const aiVector3D position = mesh->HasPositions() ? mesh->mVertices[vertexIndex] : aiVector3D(0.0f);
            const aiVector3D texcoord = mesh->HasTextureCoords(0) ? mesh->mTextureCoords[0][vertexIndex] : aiVector3D(0.0f);
            const aiVector3D normal = mesh->HasNormals() ? mesh->mNormals[vertexIndex] : aiVector3D(0.0f);
            const aiVector3D tangent = mesh->HasTangentsAndBitangents() ? mesh->mTangents[vertexIndex] : aiVector3D(0.0f);
            const aiVector3D bitangent = mesh->HasTangentsAndBitangents() ? mesh->mBitangents[vertexIndex] : aiVector3D(0.0f);
            const s32 materialIndex = mesh->mMaterialIndex;

            if (vertexDataFormat == EVertexBufferFormat::Rigid)
            {
                rigidVertices[globalVertexIndex].position = cVector3(position.x, position.y, position.z);
                rigidVertices[globalVertexIndex].texcoord = cVector2(texcoord.x, texcoord.y);
                rigidVertices[globalVertexIndex].normal = cVector3(normal.x, normal.y, normal.z);
                rigidVertices[globalVertexIndex].tangent = cVector4(tangent.x, tangent.y, tangent.z, 0.0f);
            }
            else if (vertexDataFormat == EVertexBufferFormat::Skinned)
            {
                skinnedVertices[globalVertexIndex].position = cVector3(position.x, position.y, position.z);
                skinnedVertices[globalVertexIndex].texcoord = cVector2(texcoord.x, texcoord.y);
                skinnedVertices[globalVertexIndex].normal = cVector3(normal.x, normal.y, normal.z);
                skinnedVertices[globalVertexIndex].tangent = cVector4(tangent.x, tangent.y, tangent.z, 0.0f);
            }

            bitangents[globalVertexIndex] = cVector3(bitangent.x, bitangent.y, bitangent.z);
            
            globalVertexIndex += 1;
        }
    }
}

void triton::XModelBackendAssimp::CalculateHandedness(
    EVertexBufferFormat vertexDataFormat,
    const SMemAlloc& rigidVertexData,
    const SMemAlloc& skinnedVertexData,
    const SMemAlloc& bitangents_memory,
    usize vertexCount
)
{
    SRigidVertexGPULayout* rigidVertices = (SRigidVertexGPULayout*)rigidVertexData.mem;
    SSkinnedVertexGPULayout* skinnedVertices = (SSkinnedVertexGPULayout*)skinnedVertexData.mem;
    cVector3* bitangents = (cVector3*)bitangents_memory.mem;

    for (usize i = 0; i < vertexCount; i++)
    {
        // TODO: encapsulate GLM code to math-related backend
        if (vertexDataFormat == EVertexBufferFormat::Rigid)
        {
            glm::vec3 normal = glm::vec3(
                rigidVertices[i].normal.GetX(),
                rigidVertices[i].normal.GetY(),
                rigidVertices[i].normal.GetZ()
            );
            glm::vec3 tangent = glm::vec3(
                rigidVertices[i].tangent.GetX(),
                rigidVertices[i].tangent.GetY(),
                rigidVertices[i].tangent.GetZ()
            );
            glm::vec3 bitangent = glm::vec3(
                bitangents[i].GetX(),
                bitangents[i].GetY(),
                bitangents[i].GetZ()
            );
            f32 handedness = (glm::dot(glm::cross(normal, tangent), bitangent) < 0.0f) ? -1.0f : 1.0f;

            rigidVertices[i].tangent.SetW(handedness);
        }
        else if (vertexDataFormat == EVertexBufferFormat::Skinned)
        {
            glm::vec3 normal = glm::vec3(
                skinnedVertices[i].normal.GetX(),
                skinnedVertices[i].normal.GetY(),
                skinnedVertices[i].normal.GetZ()
            );
            glm::vec3 tangent = glm::vec3(
                skinnedVertices[i].tangent.GetX(),
                skinnedVertices[i].tangent.GetY(),
                skinnedVertices[i].tangent.GetZ()
            );
            glm::vec3 bitangent = glm::vec3(
                bitangents[i].GetX(),
                bitangents[i].GetY(),
                bitangents[i].GetZ()
            );
            f32 handedness = (glm::dot(glm::cross(normal, tangent), bitangent) < 0.0f) ? -1.0f : 1.0f;

            skinnedVertices[i].tangent.SetW(handedness);
        }
    }
}

void triton::XModelBackendAssimp::ParseIndexData(
    const aiScene* scene,
    const SMemAlloc& indexData,
    const std::vector<usize>& indexOffsets
)
{
    usize globalIndicesIndex = 0;

    for (usize meshIndex = 0; meshIndex < scene->mNumMeshes; meshIndex++)
    {
        const aiMesh* mesh = scene->mMeshes[meshIndex];
        if (mesh->HasFaces())
        {
            for (usize faceIndex = 0; faceIndex < mesh->mNumFaces; faceIndex++)
            {
                const aiFace& face = mesh->mFaces[faceIndex];
                for (usize indicesIndex = 0; indicesIndex < face.mNumIndices; indicesIndex++)
                {
                    indexData.mem[globalIndicesIndex] = indexOffsets.at(meshIndex) + face.mIndices[indicesIndex];
                    globalIndicesIndex += 1;
                }
            }
        }
    }
}

void triton::XModelBackendAssimp::ParseMaterialData(const aiScene* scene, std::vector<SModelMaterialData>& materials)
{
    for (usize materialIndex = 0; materialIndex < scene->mNumMaterials; materialIndex++)
    {
        const aiMaterial* material = scene->mMaterials[materialIndex];
        aiString diffuseTexturePath = aiString("");
        material->GetTexture(aiTextureType_DIFFUSE, 0, &diffuseTexturePath);
        aiString normalTexturePath = aiString("");
        material->GetTexture(aiTextureType_NORMALS, 0, &normalTexturePath);
        aiString roughnessTexturePath = aiString("");
        material->GetTexture(aiTextureType_SHININESS, 0, &roughnessTexturePath);
        aiString metallicTexturePath = aiString("");
        material->GetTexture(aiTextureType_METALNESS, 0, &metallicTexturePath);

        SModelMaterialData m3dmd;
        m3dmd.diffuseTextureFilePath = diffuseTexturePath.C_Str();
        m3dmd.normalTextureFilePath = normalTexturePath.C_Str();
        m3dmd.roughnessTextureFilePath = roughnessTexturePath.C_Str();
        m3dmd.metallicTextureFilePath = metallicTexturePath.C_Str();
        m3dmd.bIsDiffuseEmbedded = scene->GetEmbeddedTexture(diffuseTexturePath.C_Str()) ? K_TRUE : K_FALSE;
        m3dmd.bIsNormalEmbedded = scene->GetEmbeddedTexture(normalTexturePath.C_Str()) ? K_TRUE : K_FALSE;
        m3dmd.bIsRoughnessEmbedded = scene->GetEmbeddedTexture(roughnessTexturePath.C_Str()) ? K_TRUE : K_FALSE;
        m3dmd.bIsMetallicEmbedded = scene->GetEmbeddedTexture(metallicTexturePath.C_Str()) ? K_TRUE : K_FALSE;

        materials.push_back(m3dmd);

        for (usize i = 0; i < material->mNumProperties; i++)
        {
            std::cout <<
                "name(" << material->mProperties[i]->mKey.C_Str() << ") " <<
                "semantic(" << material->mProperties[i]->mSemantic << ") " <<
                "dataLength(" << material->mProperties[i]->mDataLength << ") " <<
                "index(" << material->mProperties[i]->mIndex << ") " <<
                "type(" << material->mProperties[i]->mType << ") ";
            for (usize j = 0; j < material->mProperties[i]->mDataLength; j++)
                std::cout << material->mProperties[i]->mData[j];
            std::cout << "\n";
        }

        for (int t = 0; t <= aiTextureType_TRANSMISSION; ++t)
        {
            auto type = (aiTextureType)t;
            unsigned count = material->GetTextureCount(type);

            if (count)
            {
                printf("TYPE %d:\n", t);

                for (unsigned i = 0; i < count; ++i)
                {
                    aiString path;
                    material->GetTexture(type, i, &path);
                    printf("    %s\n", path.C_Str());
                }
            }
        }
    }
}

void triton::XModelBackendAssimp::CreateMaterials(
    cContext* context,
    const std::string& modelFolderPath,
    const std::vector<SModelMaterialData>& materials,
    std::vector<XMaterial*>& modelMaterials,
    const aiScene* scene
)
{
    for (auto& material : materials)
    {
        auto diffOpt = CreateTexture(
            context,
            ETextureFormat::RGBA8_SRGB_Mips,
            modelFolderPath,
            material.diffuseTextureFilePath,
            material.bIsDiffuseEmbedded,
            scene->GetEmbeddedTexture(material.diffuseTextureFilePath.c_str())
        );
        auto normOpt = CreateTexture(
            context,
            ETextureFormat::RGBA8,
            modelFolderPath,
            material.normalTextureFilePath,
            material.bIsNormalEmbedded,
            scene->GetEmbeddedTexture(material.normalTextureFilePath.c_str())
        );
        auto rghnOpt = CreateTexture(
            context,
            ETextureFormat::R8,
            modelFolderPath,
            material.roughnessTextureFilePath,
            material.bIsRoughnessEmbedded,
            scene->GetEmbeddedTexture(material.roughnessTextureFilePath.c_str())
        );
        auto metlOpt = CreateTexture(
            context,
            ETextureFormat::R8,
            modelFolderPath,
            material.metallicTextureFilePath,
            material.bIsMetallicEmbedded,
            scene->GetEmbeddedTexture(material.metallicTextureFilePath.c_str())
        );

        modelMaterials.push_back(
            CCreator::Create<XMaterial>(
                _context,
                diffOpt.has_value() ? *diffOpt : nullptr,
                normOpt.has_value() ? *normOpt : nullptr,
                rghnOpt.has_value() ? *rghnOpt : nullptr,
                metlOpt.has_value() ? *metlOpt : nullptr,
                cVector4(1.0f),
                cVector4(1.0f),
                0.0f
            )
        );
    }
}

void triton::XModelBackendAssimp::DeallocateTempBitangentBuffer(SMemAlloc& bitangents)
{
    CMemoryAllocator::Deallocate(bitangents);
}

void triton::XModelBackendAssimp::CreateBones(
    const aiScene* scene,
    usize vertexCount,
    std::unordered_map<std::string, usize>& boneIndices,
    std::vector<SSkeletonBone>& bones,
    std::vector<std::vector<SBoneWeight>>& vertexWeights
)
{
    vertexWeights.resize(vertexCount);

    usize vertexOffset = 0;

    for (usize meshIndex = 0; meshIndex < scene->mNumMeshes; meshIndex++)
    {
        const aiMesh* mesh = scene->mMeshes[meshIndex];

        for (usize boneIndex = 0; boneIndex < mesh->mNumBones; boneIndex++)
        {
            const aiBone* bone = mesh->mBones[boneIndex];

            std::string boneName = bone->mName.C_Str();
            usize realBoneIndex = 0;
            if (boneIndices.find(boneName) == boneIndices.end())
            {
                SSkeletonBone b = {};
                b.name = boneName;
                b.modelMatrix = ConvertMatrix(bone->mOffsetMatrix);

                realBoneIndex = bones.size();

                boneIndices.insert({ boneName, realBoneIndex });

                bones.push_back(std::move(b));
            }
            else
            {
                realBoneIndex = boneIndices[boneName];
            }

            for (usize weightIndex = 0; weightIndex < bone->mNumWeights; weightIndex++)
            {
                const aiVertexWeight& vw = bone->mWeights[weightIndex];
                const usize vertexIndex = vertexOffset + vw.mVertexId;

                vertexWeights[vertexIndex].push_back({ 
                    realBoneIndex,
                    vw.mWeight
                });
            }
        }

        vertexOffset += mesh->mNumVertices;
    }
}

void triton::XModelBackendAssimp::FinalizeBoneWeights(
    cContext* context,
    const SMemAlloc& vertexData,
    types::usize vertexCount,
    std::vector<std::vector<SBoneWeight>>& vertexWeights
)
{
    SSkinnedVertexGPULayout* skinnedVertices = (SSkinnedVertexGPULayout*)vertexData.mem;

    for (usize vertexIndex = 0; vertexIndex < vertexCount; vertexIndex++)
    {
        auto& weights = vertexWeights[vertexIndex];

        std::sort(
            weights.begin(),
            weights.end(),
            [](const SBoneWeight& a, const SBoneWeight& b)
            {
                return a.weight > b.weight;
            }
        );

        const SCapabilities& caps = context->GetSubsystem<XEngine>()->GetApplication()->GetCapabilities();

        if (weights.size() > caps.maxBoneCountPerVertex)
            weights.resize(caps.maxBoneCountPerVertex);

        f32 sum = 0.0f;

        for (const auto& w : weights)
            sum += w.weight;

        if (sum > 0.0f)
        {
            for (auto& w : weights)
                w.weight /= sum;
        }

        for (usize i = 0; i < weights.size(); i++)
        {
            skinnedVertices[vertexIndex].boneIndices[i] = weights[i].boneIndex;
            skinnedVertices[vertexIndex].boneWeights[i] = weights[i].weight;
        }
    }
}

boolean triton::XModelBackendAssimp::AccumulateRootTransform(
    const aiNode* node,
    aiNode*& boneRootNode,
    const aiMatrix4x4& parentRootTransform,
    aiMatrix4x4& accumulatedRootTransform,
    std::unordered_map<std::string, usize>& boneIndices
)
{
    aiMatrix4x4 current = parentRootTransform * node->mTransformation;

    auto it = boneIndices.find(node->mName.C_Str());

    if (it != boneIndices.end())
    {
        boneRootNode = (aiNode*)node;
        accumulatedRootTransform = parentRootTransform;
        return K_TRUE;
    }

    for (uint32_t i = 0; i < node->mNumChildren; i++)
    {
        if (AccumulateRootTransform(
            node->mChildren[i],
            boneRootNode,
            current,
            accumulatedRootTransform,
            boneIndices
        ))
            return K_TRUE;
    }

    return K_FALSE;
}

void triton::XModelBackendAssimp::CreateBoneHierarchy(
    const aiNode* node,
    s32 parentBone,
    std::unordered_map<std::string, usize>& boneIndices,
    std::vector<SSkeletonBone>& bones
)
{
    s32 currentBone = -1;

    auto it = boneIndices.find(node->mName.C_Str());

    if (it != boneIndices.end())
    {
        currentBone = it->second;

        bones[currentBone].localParentBoneIndex = parentBone;
        bones[currentBone].localMatrix = ConvertMatrix(node->mTransformation);

        if (parentBone != -1)
            bones[parentBone].localChildBoneIndices.push_back(currentBone);
    }

    for (uint32_t i = 0; i < node->mNumChildren; i++)
    {
        CreateBoneHierarchy(
            node->mChildren[i],
            currentBone == -1 ? parentBone : currentBone,
            boneIndices,
            bones
        );
    }
}

void triton::XModelBackendAssimp::CreateSkeleton(
    cContext* context,
    XSkeleton*& modelSkeleton,
    const std::vector<SSkeletonBone>& bones,
    const aiMatrix4x4& accumulatedRootTransform
)
{
    modelSkeleton = CCreator::Create<XSkeleton>(
        _context,
        bones,
        ConvertMatrix(accumulatedRootTransform)
    );
}

void triton::XModelBackendAssimp::CreateAnimations(
    cContext* context,
    const aiScene* scene,
    const std::unordered_map<std::string, usize>& boneIndices,
    std::vector<XAnimation*>& modelAnimations
)
{
    for (usize animIdx = 0; animIdx < scene->mNumAnimations; ++animIdx)
    {
        const aiAnimation* srcAnim = scene->mAnimations[animIdx];

        auto animationName = srcAnim->mName.C_Str();
        auto animationDuration = srcAnim->mDuration;
        auto animationTicksPerSecond = srcAnim->mTicksPerSecond;

        auto animationKeys = std::vector<SAnimationKey>();
        animationKeys.reserve(srcAnim->mNumChannels);

        for (usize channelIdx = 0; channelIdx < srcAnim->mNumChannels; ++channelIdx)
        {
            const aiNodeAnim* channel = srcAnim->mChannels[channelIdx];

            auto it = boneIndices.find(channel->mNodeName.C_Str());
            if (it == boneIndices.end())
                continue;

            SAnimationKey animKey = {};
            animKey.local_bone_index = it->second;

            // Position
            animKey.position_keys.reserve(channel->mNumPositionKeys);
            for (usize keyIdx = 0; keyIdx < channel->mNumPositionKeys; ++keyIdx)
            {
                const aiVectorKey& key = channel->mPositionKeys[keyIdx];
                SBonePositionKey bpk = {};
                bpk.time = key.mTime;
                bpk.position = cVector3(key.mValue.x, key.mValue.y, key.mValue.z);
                animKey.position_keys.push_back(bpk);
            }

            // Rotation
            animKey.rotation_keys.reserve(channel->mNumRotationKeys);
            for (usize keyIdx = 0; keyIdx < channel->mNumRotationKeys; ++keyIdx)
            {
                const aiQuatKey& key = channel->mRotationKeys[keyIdx];
                SBoneRotationKey brk = {};
                brk.time = key.mTime;
                brk.rotation = cQuaternion(key.mValue.w, key.mValue.x, key.mValue.y, key.mValue.z);
                animKey.rotation_keys.push_back(brk);
            }

            // Scale
            animKey.scale_keys.reserve(channel->mNumScalingKeys);
            for (usize keyIdx = 0; keyIdx < channel->mNumScalingKeys; ++keyIdx)
            {
                const aiVectorKey& key = channel->mScalingKeys[keyIdx];
                SBoneScaleKey bsk = {};
                bsk.time = key.mTime;
                bsk.scale = cVector3(key.mValue.x, key.mValue.y, key.mValue.z);
                animKey.scale_keys.push_back(bsk);
            }

            animationKeys.push_back(std::move(animKey));
        }

        modelAnimations.push_back(
            CCreator::Create<XAnimation>(
                _context,
                animationName,
                animationDuration,
                animationTicksPerSecond,
                animationKeys
            )
        );
    }
}

std::optional<triton::XTexture*> triton::XModelBackendAssimp::CreateTexture(
    cContext* context,
    ETextureFormat dataFormat,
    const std::string& modelFolderPath,
    const std::string& textureFilePath,
    boolean bIsEmbedded,
    const aiTexture* texture
)
{
    if (bIsEmbedded == K_TRUE)
        return CreateTextureFromModelData(context, dataFormat, textureFilePath, texture);
    else
        return CreateTextureFromFile(context, dataFormat, modelFolderPath, textureFilePath);
}

std::optional<triton::XTexture*> triton::XModelBackendAssimp::CreateTextureFromModelData(
    cContext* context,
    ETextureFormat dataFormat,
    const std::string& textureFilePath,
    const aiTexture* texture
)
{
    if (!texture)
        return std::nullopt;

    if (texture->mHeight == 0)
    {
        // Compressed PNG/JPG/DDS
        const u8* fileData = (u8*)texture->pcData;
        size_t fileByteSize = texture->mWidth;

        const char* fmtHint = &texture->achFormatHint[0];
        if (fmtHint[0] == 'p' && fmtHint[1] == 'n' && fmtHint[2] == 'g')
        {
            return CCreator::Create<XTexture>(
                _context,
                types::K_TRUE,
                dataFormat,
                ETextureUsageBit::Sampled,
                ETextureDimension::Texture2D,
                0,
                EImageFileFormat::PNG,
                _context->GetBackend<IImageDecoder>()->TextureFormatToImageFormat(dataFormat),
                fileData,
                fileByteSize
            );
        }
        else if (fmtHint[0] == 'd' && fmtHint[1] == 'd' && fmtHint[2] == 's')
        {
            return CCreator::Create<XTexture>(
                _context,
                types::K_TRUE,
                dataFormat,
                ETextureUsageBit::Sampled,
                ETextureDimension::Texture2D,
                0,
                EImageFileFormat::DDS,
                _context->GetBackend<IImageDecoder>()->TextureFormatToImageFormat(dataFormat),
                fileData,
                fileByteSize
            );
        }
        else
        {
            Print("Error: incorrect embedded texture format, only PNG/DDS embedded textures are supported");
            return std::nullopt;
        }
    }
    else
    {
        // Uncompressed BGRA
        // TODO: Implement embedded BGRA textures support
        const aiTexel* pixels = texture->pcData;
        unsigned width = texture->mWidth;
        unsigned height = texture->mHeight;
        Print("Error: uncompressed BGRA embedded textures are currenly unsupported, path: " + textureFilePath + "\n");
        
        return std::nullopt;
    }
}

std::optional<triton::XTexture*> triton::XModelBackendAssimp::CreateTextureFromFile(
    cContext* context,
    ETextureFormat dataFormat,
    const std::string& modelFolderPath,
    const std::string& textureFilePath
)
{
    std::string newTextureFilePath;
    std::filesystem::path path(textureFilePath);

    if (textureFilePath.length() > 0 && textureFilePath.at(0) == '*')
    {
        Print("Error: embedded textures are not supported, model folder path: " + modelFolderPath);
        return std::nullopt;
    }
    else if (path.is_absolute())
    {
        newTextureFilePath =
            std::filesystem::path(modelFolderPath).generic_string() +
            "/" +
            path.filename().string();
    }
    else
    {
        path = modelFolderPath / path;
        newTextureFilePath = path.generic_string();
    }
    
    return CCreator::Create<XTexture>(
        _context,
        types::K_TRUE,
        dataFormat,
        ETextureUsageBit::Sampled,
        ETextureDimension::Texture2D,
        0,
        _context->GetBackend<IImageDecoder>()->TextureFormatToImageFormat(dataFormat),
        newTextureFilePath
    );
}

triton::SModelData triton::XModelBackendAssimp::PrepareResult(
    EVertexBufferFormat vertexDataFormat,
    const SMemAlloc& rigidVertexData,
    const SMemAlloc& skinnedVertexData,
    const SMemAlloc& indexData,
    usize vertexCount,
    usize indexCount,
    const std::vector<XMaterial*>& modelMaterials,
    XSkeleton* modelSkeleton,
    const std::vector<XAnimation*>& modelAnimations
)
{
    SModelData m3dd;
    m3dd.vertexDataFormat = vertexDataFormat;
    m3dd.rigidVertexData = rigidVertexData;
    m3dd.skinnedVertexData = skinnedVertexData;
    m3dd.indexData = indexData;
    m3dd.vertexCount = vertexCount;
    m3dd.indexCount = indexCount;
    m3dd.materials = modelMaterials;
    m3dd.skeleton = modelSkeleton;
    m3dd.animations = modelAnimations;

    return m3dd;
}

triton::cMatrix4 triton::XModelBackendAssimp::ConvertMatrix(const aiMatrix4x4& m)
{
    glm::mat4 result;
    result[0] = glm::vec4(m.a1, m.b1, m.c1, m.d1);
    result[1] = glm::vec4(m.a2, m.b2, m.c2, m.d2);
    result[2] = glm::vec4(m.a3, m.b3, m.c3, m.d3);
    result[3] = glm::vec4(m.a4, m.b4, m.c4, m.d4);

    return cMatrix4(result);
}