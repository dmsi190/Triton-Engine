// texture.hpp

#pragma once

#include <filesystem>
#include "object.hpp"
#include "math.hpp"
#include "gpu_texture_resource.hpp"
#include "texture_formats.hpp"
#include "texture_dimensions.hpp"
#include "image_format_enum.hpp"
#include "image_file_format_enum.hpp"
#include "graphics_texture_usage_bit_enum.hpp"

namespace triton
{
	class cContext;

	class XTexture : public iObject
	{
		TRITON_OBJECT(XTexture)

		SGPUTextureResource _gpuTexture = SGPUTextureResource::Invalid();

	public:
		explicit XTexture(
			const SMemAlloc& mem_alloc,
			cContext* context,
			types::boolean bCreateSampler,
			ETextureFormat textureFormat,
			types::dword usageMask,
			ETextureDimension textureDimension,
			types::s32 textureSlot,
			EImageFormat expectedDataFormat,
			const std::filesystem::path& filePath
		);

		explicit XTexture(
			const SMemAlloc& mem_alloc,
			cContext* context,
			types::boolean bCreateSampler,
			ETextureFormat textureFormat,
			types::dword usageMask,
			ETextureDimension textureDimension,
			types::s32 textureSlot,
			EImageFileFormat containerFormat,
			EImageFormat expectedDataFormat,
			const types::u8* data,
			types::usize dataByteSize
		);

		explicit XTexture(
			const SMemAlloc& mem_alloc,
			cContext* context,
			types::boolean bCreateSampler,
			ETextureFormat textureFormat,
			types::dword usageMask,
			ETextureDimension textureDimension,
			const cVector3& size,
			const types::u8* data,
			types::s32 textureSlot
		);

		~XTexture() override;

		inline const SGPUTextureResource& GetGPUResource() const { return _gpuTexture; }

		inline const cVector3& GetSize() const { return _gpuTexture.size; }

		inline ETextureFormat GetFormat() const { return _gpuTexture.format; }

		inline types::s32 GetSlot() const { return _gpuTexture.slot; }

		static types::usize TextureFormatToChannelCount(ETextureFormat textureFormat);
		
	private:
		void CreateOnGpu(
			types::boolean bCreateSampler,
			ETextureFormat format,
			ETextureUsageBit usageMask,
			ETextureDimension dimension,
			const cVector3& size,
			const types::u8* data,
			types::s32 slot
		);

		void GenerateMips();
	};
}