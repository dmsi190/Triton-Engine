// image_decoder_multi.hpp

#pragma once

#include <filesystem>
#include <stb_image.h>
#include <tinyddsloader.h>
#include "image_decoder.hpp"

namespace triton
{
	class BImageDecoderMulti : public IImageDecoder
	{
		TRITON_OBJECT(BImageDecoderMulti)

	public:
		explicit BImageDecoderMulti(const SMemAlloc& mem_alloc, cContext* context)
			: IImageDecoder(mem_alloc, context) {}
		
		~BImageDecoderMulti() override = default;

		SImageResource Decode(
			EImageFormat expectedDataFormat,
			const std::filesystem::path& filePath
		) override final;

		SImageResource Decode(
			EImageFileFormat fileFormat,
			EImageFormat expectedDataFormat,
			const types::u8* data,
			types::usize dataByteSize
		) override final;

		EImageFormat TextureFormatToImageFormat(ETextureFormat textureFormat) override final;

	private:
		types::usize DetermineChannelCount(EImageFormat format);

		types::boolean ValidateDDSFormat(
			tinyddsloader::DDSFile::DXGIFormat ddsFormat,
			EImageFormat expectedFormat
		);

		SImageResource ReturnPNGData(
			EImageFormat expectedDataFormat,
			const std::string& filePathStr,
			const stbi_uc* stbData,
			int stbWidth,
			int stbHeight,
			int stbDesiredChannels
		);

		SImageResource ReturnDDSData(
			EImageFormat expectedDataFormat,
			const std::string& filePathStr,
			tinyddsloader::Result ddsResult,
			tinyddsloader::DDSFile& dds
		);
	};
}