// image_decoder.hpp

#pragma once

#include <filesystem>
#include "backend.hpp"
#include "image_backend_data.hpp"
#include "image_file_format_enum.hpp"
#include "texture_format.hpp"
#include "object.hpp"

namespace triton
{
	class IImageDecoder : public IBackend
	{
		TRITON_OBJECT(IImageDecoder)

	public:
		explicit IImageDecoder(const SMemAlloc& mem_alloc, cContext* context) : IBackend(mem_alloc, context) {}
		
		virtual ~IImageDecoder() = default;

		virtual SImageResource DecodeImage(
			EImageDataFormat expectedDataFormat,
			const std::filesystem::path& filePath
		) = 0;

		virtual SImageResource DecodeImage(
			EImageFileFormat fileFormat,
			EImageDataFormat expectedDataFormat,
			const types::u8* data,
			types::usize dataByteSize
		) = 0;

		virtual void ReleaseImage(SImageResource& image) = 0;

		virtual EImageDataFormat TextureFormatToImageFormat(ETextureFormat textureFormat) = 0;
	};
}