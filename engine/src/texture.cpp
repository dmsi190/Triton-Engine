// texture.cpp

#include "texture.hpp"
#include "context.hpp"
#include "synchronization.hpp"
#include "image_decoder.hpp"
#include "render_command_proxy.hpp"
#include "creator.hpp"

using namespace types;

triton::XTexture::XTexture(
	const SMemAlloc& mem_alloc,
	cContext* context,
	types::boolean bCreateSampler,
	ETextureFormat textureFormat,
	dword usageMask,
	ETextureDimension textureDimension,
	s32 textureSlot,
	EImageFormat expectedDataFormat,
	const std::filesystem::path& filePath
) : iObject(mem_alloc, context)
{
	SImageResource tex = _context->GetBackend<IImageDecoder>()->Decode(
		expectedDataFormat,
		filePath
	);

	CreateOnGpu(
		bCreateSampler,
		textureFormat,
		usageMask,
		textureDimension,
		tex.size,
		tex.data,
		textureSlot
	);

	GenerateMips();
}

triton::XTexture::XTexture(
	const SMemAlloc& mem_alloc,
	cContext* context,
	types::boolean bCreateSampler,
	ETextureFormat textureFormat,
	dword usageMask,
	ETextureDimension textureDimension,
	s32 textureSlot,
	EImageFileFormat containerFormat,
	EImageFormat expectedDataFormat,
	const u8* data,
	usize dataByteSize
) : iObject(mem_alloc, context)
{
	SImageResource tex = _context->GetBackend<IImageDecoder>()->Decode(
		containerFormat,
		expectedDataFormat,
		data,
		dataByteSize
	);

	CreateOnGpu(
		bCreateSampler,
		textureFormat,
		usageMask,
		textureDimension,
		tex.size,
		tex.data,
		textureSlot
	);

	GenerateMips();
}

triton::XTexture::XTexture(
	const SMemAlloc& mem_alloc,
	cContext* context,
	types::boolean bCreateSampler,
	ETextureFormat textureFormat,
	dword usageMask,
	ETextureDimension textureDimension,
	const cVector3& size,
	const types::u8* data,
	s32 textureSlot
) : iObject(mem_alloc, context)
{
	CreateOnGpu(
		bCreateSampler,
		textureFormat,
		usageMask,
		textureDimension,
		size,
		data,
		textureSlot
	);

	GenerateMips();
}

triton::XTexture::~XTexture()
{
	if (_gpuTexture.format == ETextureFormat::Unknown)
		return;

	CRenderCommandProxy::DestroyTexture(_gpuTexture);
}

void triton::XTexture::CreateOnGpu(
	boolean bCreateSampler,
	ETextureFormat format,
	ETextureUsageBit usageMask,
	ETextureDimension dimension,
	const cVector3& size,
	const u8* data,
	s32 slot
)
{
	_gpuTexture = CRenderCommandProxy::CreateTexture(
		bCreateSampler,
		format,
		usageMask,
		dimension,
		size
	);

	const usize dataByteSize =
		size.GetX() *
		size.GetY() *
		size.GetZ() *
		TextureFormatToChannelCount(format);

	CRenderCommandProxy::WriteTexture(_gpuTexture, data);
}

void triton::XTexture::GenerateMips()
{
	if (_gpuTexture.format == ETextureFormat::RGBA8_SRGB_Mips)
		CRenderCommandProxy::GenerateTextureMips(_gpuTexture);
}

usize triton::XTexture::TextureFormatToChannelCount(ETextureFormat textureFormat)
{
	if (textureFormat == ETextureFormat::R8)
		return 1;
	else if (textureFormat == ETextureFormat::RGBA8 ||
		textureFormat == ETextureFormat::RGBA8_SRGB ||
		textureFormat == ETextureFormat::RGBA8_SRGB_Mips ||
		textureFormat == ETextureFormat::BGRA8_SRGB ||
		textureFormat == ETextureFormat::DepthStencil)
		return 4;

	return 0;
}