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
			: IImageDecoder(mem_alloc, context) {
		}

		~BImageDecoderMulti() override = default;

		SImageResource DecodeImage(
			EImageDataFormat expectedDataFormat,
			const std::filesystem::path& filePath
		) override final;

		SImageResource DecodeImage(
			EImageFileFormat fileFormat,
			EImageDataFormat expectedDataFormat,
			const types::u8* data,
			types::usize dataByteSize
		) override final;

		void ReleaseImage(SImageResource& image) override final;

		EImageDataFormat TextureFormatToImageFormat(ETextureFormat textureFormat) override final;

	private:
		types::usize DetermineChannelCount(EImageDataFormat format);

		types::boolean ValidateDDSFormat(
			tinyddsloader::DDSFile::DXGIFormat ddsFormat,
			EImageDataFormat expectedFormat
		);

		SImageResource ReturnPNGData(
			EImageDataFormat expectedDataFormat,
			const std::string& filePathStr,
			const stbi_uc* stbData,
			int stbWidth,
			int stbHeight,
			int stbDesiredChannels
		);

		SImageResource ReturnDDSData(
			EImageDataFormat expectedDataFormat,
			const std::string& filePathStr,
			tinyddsloader::Result ddsResult,
			tinyddsloader::DDSFile& dds
		);
	};
}