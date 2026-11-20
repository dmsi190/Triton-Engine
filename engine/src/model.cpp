// model.cpp

#include "model.hpp"
#include "context.hpp"
#include "tasset.hpp"
#include "skeleton_pool.hpp"
#include "texture.hpp"
#include "image_decoder.hpp"
#include "graphics_texture_usage_enum.hpp"
#include "creator.hpp"

using namespace types;

triton::XModel::XModel(
	const SMemoryAllocation& mem_alloc,
    cContext* context,
	EVertexBufferFormat vertexDataFormat,
	EModelFileType fileType,
    const std::filesystem::path& filePath
) : iObject(mem_alloc, context),
	_vertexDataFormat(vertexDataFormat),
	_fileType(fileType)
{
    if (_fileType == EModelFileType::Raw)
    {
		std::filesystem::path modelFolderPath = filePath.parent_path();
		std::filesystem::path modelLocalPath = filePath.filename();

		auto result = _context->GetBackend<IModel3DBackend>()->CreateModel(
			modelFolderPath.generic_string(),
			modelLocalPath.generic_string(),
			vertexDataFormat
		);
		if (!result)
			return;

        _data = *result;
    }
    else if (_fileType == EModelFileType::Asset)
    {
		std::filesystem::path assetFolderPath = filePath.parent_path();

		_asset = CCreator::Create<asset::CModel3DAsset>();
		_asset->LoadAssetFile(filePath);

		// TODO: rewrite TAsset tools to rigid/skinned models support
		//_data.rigidVertexData = (const SRigidVertexGPULayout*)_asset->rigidVertexData;
		//_data.skinnedVertexData = (const SSkinnedVertexGPULayout*)_asset->skinnedVertexData;
		_data.vertexCount = _asset->vertexCount;
		_data.indexData = _asset->indexData;
		_data.indexCount = _asset->indexCount;

		for (usize i = 0; i < _asset->materialData.size(); i++)
		{
			asset::SModel3DMaterialData& assetMaterialData = _asset->materialData[i];

			auto ProcessTexture = [this](
				const std::filesystem::path& assetFolderPath,
				const std::filesystem::path& assetTexturePath,
				const asset::STextureData& assetTextureData,
				ETextureFormat textureFormat,
				XTexture* outHandle
				) {
					std::filesystem::path texturePath = assetTexturePath;
					if (!assetTexturePath.is_absolute())
						texturePath = assetFolderPath / assetTexturePath;
					if (std::filesystem::exists(texturePath))
						outHandle = CCreator::Create<XTexture>(
							types::K_FALSE,
							textureFormat,
							(dword)ETextureUsageBit::Sampled,
							ETextureDimension::Texture2D,
							0,
							_context->GetBackend<IImageDecoder>()->TextureFormatToImageFormat(textureFormat),
							texturePath.generic_string()
						);
					else
						outHandle = CCreator::Create<XTexture>(
							types::K_FALSE,
							textureFormat,
							(dword)ETextureUsageBit::Sampled,
							ETextureDimension::Texture2D,
							cVector3(assetTextureData.width, assetTextureData.height, 1),
							assetTextureData.pixelByteData,
							0
						);
				};

			XTexture* diffuseTexture;
			XTexture* normalTexture;
			XTexture* roughnessTexture;
			XTexture* metallicTexture;

			ProcessTexture(
				assetFolderPath,
				std::filesystem::path(assetMaterialData.diffuseTexturePath),
				assetMaterialData.diffuseTexture,
				ETextureFormat::RGBA8_SRGB_Mips,
				diffuseTexture
			);
			ProcessTexture(
				assetFolderPath,
				std::filesystem::path(assetMaterialData.normalTexturePath),
				assetMaterialData.normalTexture,
				ETextureFormat::RGBA8,
				normalTexture
			);
			ProcessTexture(
				assetFolderPath,
				std::filesystem::path(assetMaterialData.roughnessTexturePath),
				assetMaterialData.roughnessTexture,
				ETextureFormat::R8,
				roughnessTexture
			);
			ProcessTexture(
				assetFolderPath,
				std::filesystem::path(assetMaterialData.metallicTexturePath),
				assetMaterialData.metallicTexture,
				ETextureFormat::R8,
				metallicTexture
			);

			XMaterial* material = CCreator::Create<XMaterial>(
				diffuseTexture,
				normalTexture,
				roughnessTexture,
				metallicTexture,
				cVector4(1.0f),
				cVector4(1.0f),
				0.0f
			);

			_data.materials.push_back(material);
		}

		std::vector<SSkeletonBone> bones;
		bones.resize(_asset->boneData.size());
		for (usize i = 0; i < _asset->boneData.size(); i++)
		{
			bones[i].name = _asset->boneData[i].name;
			bones[i].localMatrix = cMatrix4(_asset->boneData[i].localMatrix);
			bones[i].modelMatrix = cMatrix4(_asset->boneData[i].modelMatrix);
			bones[i].localParentBoneIndex = _asset->boneData[i].parentLocalBoneIndex;
			bones[i].localChildBoneIndices = _asset->boneData[i].childLocalBoneIndices;
		}

		if (bones.size() > 0)
		{
			_data.skeleton = CCreator::Create<XSkeleton>(
				bones,
				cMatrix4(_asset->rootBoneAccumulatedTransform)
			);
		}

		for (usize i = 0; i < _asset->animationData.size(); i++)
		{
			std::vector<SAnimationKey> keys;
			keys.resize(_asset->animationData[i].keys.size());
			for (usize j = 0; j < _asset->animationData[i].keys.size(); j++)
			{
				keys[j].local_bone_index = _asset->animationData[i].keys[j].localBoneIndex;
				keys[j].position_keys.resize(_asset->animationData[i].keys[j].positionKeys.size());
				for (usize k = 0; k < _asset->animationData[i].keys[j].positionKeys.size(); k++)
				{
					keys[j].position_keys[k].position = cVector3(_asset->animationData[i].keys[j].positionKeys[k].position);
					keys[j].position_keys[k].time = _asset->animationData[i].keys[j].positionKeys[k].time;
				}
				keys[j].rotation_keys.resize(_asset->animationData[i].keys[j].rotationKeys.size());
				for (usize k = 0; k < _asset->animationData[i].keys[j].rotationKeys.size(); k++)
				{
					keys[j].rotation_keys[k].rotation = cQuaternion(_asset->animationData[i].keys[j].rotationKeys[k].rotation);
					keys[j].rotation_keys[k].time = _asset->animationData[i].keys[j].rotationKeys[k].time;
				}
				keys[j].scale_keys.resize(_asset->animationData[i].keys[j].scaleKeys.size());
				for (usize k = 0; k < _asset->animationData[i].keys[j].scaleKeys.size(); k++)
				{
					keys[j].scale_keys[k].scale = cVector3(_asset->animationData[i].keys[j].scaleKeys[k].scale);
					keys[j].scale_keys[k].time = _asset->animationData[i].keys[j].scaleKeys[k].time;
				}
			}

			XAnimation* animHandle = CCreator::Create<XAnimation>(
				_asset->animationData[i].name,
				_asset->animationData[i].duration,
				_asset->animationData[i].ticksPerSecond,
				keys
			);

			_data.animations.push_back(animHandle);
		}
    }
}

triton::XModel::~XModel()
{
	if (_fileType == EModelFileType::Raw)
	{
		_context->GetBackend<IModel3DBackend>()->DestroyModel(_data);
	}
	else if (_fileType == EModelFileType::Asset)
	{
		_asset->Destroy();
		CCreator::Destroy<asset::CModel3DAsset>(_asset);
	}
}