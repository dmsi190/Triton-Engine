// graphics_backend2_vulkan.cpp

#include <cstring>
#include <set>
#include <algorithm>
#include <cstdint>
#include <cstddef>
#include <iterator>
#include "graphics_backend2_vulkan.hpp"
#include "input_backend.hpp"
#include "context.hpp"
#include "filesystem_manager.hpp"
#include "log.hpp"
#include "memory_units.hpp"
#include "render_native_draw_command_info_struct.hpp"
#include "rasterizer_state.hpp"
#include "vertex.hpp"
#include "engine.hpp"
#include "application.hpp"

using namespace types;

constexpr const char* kValidationLayer = "VK_LAYER_KHRONOS_validation";

types::boolean CheckValidationLayerSupport()
{
	uint32_t layerCount = 0;

	VkResult result = vkEnumerateInstanceLayerProperties(
		&layerCount,
		nullptr
	);

	if (result != VK_SUCCESS)
		return False;

	std::vector<VkLayerProperties> availableLayers(layerCount);

	result = vkEnumerateInstanceLayerProperties(
		&layerCount,
		availableLayers.data()
	);

	if (result != VK_SUCCESS)
		return False;

	for (const VkLayerProperties& layer : availableLayers)
	{
		if (std::strcmp(layer.layerName, kValidationLayer) == 0)
			return True;
	}

	return False;
}

VKAPI_ATTR VkBool32 VKAPI_CALL DebugCallback(
	VkDebugUtilsMessageSeverityFlagBitsEXT messageSeverity,
	VkDebugUtilsMessageTypeFlagsEXT messageType,
	const VkDebugUtilsMessengerCallbackDataEXT* callbackData,
	void* userData
)
{
	triton::Print(
		"[Vulkan] " +
		std::string(callbackData->pMessage)
	);

	return VK_FALSE;
}

boolean IsZeroTerminated(const char* str, usize maxLength)
{
	if (!str)
		return False;

	for (usize i = 0; i < maxLength; ++i)
		if (str[i] == '\0')
			return True;

	return False;
}

void triton::BGraphicsBackend2Vulkan::Initialize(
	SWindowResource& window,
	boolean bEnableDebugging,
	const std::vector<const char*> extensions,
	EGraphicsAdapterType deviceType,
	usize staging_buffer_byte_size,
	const cVector2& swapchainSize,
	usize framesInFlight
)
{
	_framesInFlight = framesInFlight;

	CreateInstance(bEnableDebugging, extensions);
	CreateSurface(window);
	PickPhysicalDevice(deviceType);
	CreateLogicalDevice();
	CreateCommandPoolsAndCommandBuffers();
	CreateStagingBuffer(staging_buffer_byte_size);
	CreateStagingFence();
	CreateSwapchain(swapchainSize, framesInFlight);
	CreateDescriptorPool();
}

void triton::BGraphicsBackend2Vulkan::Shutdown()
{
	DestroyDescriptorPool();
	DestroySwapchain();
	DestroyStagingFence();
	DestroyStagingBuffer();
	DestroyCommandPoolsAndCommandBuffers();
	DestroyLogicalDevice();
	DestroySurface();
	DestroyInstance();
}

void triton::BGraphicsBackend2Vulkan::FinalizeSwapchain(const SGPUTextureResource& presentTexture)
{
	SViewport viewport;
	viewport.rect = cVector4(
		0,
		0,
		_swapchainRenderTargets[0].color_attachments[0].size.GetX(),
		_swapchainRenderTargets[0].color_attachments[0].size.GetY()
	);

	const std::vector<SRenderBindingGroupBinding> texturesToBind = {
		SRenderBindingGroupBinding(
			0,
			ERenderBindingGroupBindingType::TextureSampler,
			EShaderStageBit::Pixel,
			(SGPUResource*)&presentTexture
		)
	};

	for (usize i = 0; i < _swapchainRenderPasses.size(); i++)
	{
		_swapchainBindingGroups.push_back(
			CreateBindingGroup(
				0,
				_swapchainBindingGroupLayout,
				{},
				texturesToBind
			)
		);

		_swapchainPipelines.push_back(
			CreatePipeline(
				_swapchainShader,
				{ _swapchainBindingGroupLayout },
				False,
				viewport,
				_swapchainRenderTargets[i],
				_swapchainRenderPasses[i],
				ERenderPrimitiveTopology::TriangleStrip,
				EVertexBufferFormat::Unknown
			)
		);
	}

	_swapchainPresentTexture = presentTexture;
}

void triton::BGraphicsBackend2Vulkan::ReleaseSwapchainResources()
{
	for (auto& pipeline : _swapchainPipelines)
		DestroyPipeline(pipeline);

	for (auto& bindingGroup : _swapchainBindingGroups)
		DestroyBindingGroup(bindingGroup);
}

triton::SGPUTextureResource triton::BGraphicsBackend2Vulkan::CreateTexture(
	boolean bCreateSampler,
	ETextureFormat format,
	dword usageMask,
	ETextureDimension dimension,
	const cVector3& size,
	SGPUTextureResource* resultTexture // TODO: remove this argument completely and implement separate ERenderCommand::PipelineImageBarrier 
)
{
	const VkFormat nativeFormat = TextureFormatToNative(format);

	VkImageCreateInfo imageCreateInfo = {};
	imageCreateInfo.sType = VK_STRUCTURE_TYPE_IMAGE_CREATE_INFO;
	imageCreateInfo.imageType = TextureDimensionToNative(dimension);
	imageCreateInfo.format = nativeFormat;
	imageCreateInfo.extent = {
		(uint32_t)size.GetX(),
		(uint32_t)size.GetY(),
		1
	};
	imageCreateInfo.mipLevels = 1;
	imageCreateInfo.arrayLayers = 1;
	imageCreateInfo.samples = VK_SAMPLE_COUNT_1_BIT;
	imageCreateInfo.tiling = VK_IMAGE_TILING_OPTIMAL;
	imageCreateInfo.usage = TextureUsageToNative(usageMask);
	imageCreateInfo.sharingMode = VK_SHARING_MODE_EXCLUSIVE;
	imageCreateInfo.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;

	VkImage image = VK_NULL_HANDLE;
	
	VkResult result = vkCreateImage(
		_logicalDevice.device,
		&imageCreateInfo,
		nullptr,
		&image
	);

	if (result != VK_SUCCESS)
		Print("[Vulkan]: Error: failed to create image");

	VkMemoryRequirements memoryRequirements = {};
	vkGetImageMemoryRequirements(
		_logicalDevice.device,
		image,
		&memoryRequirements
	);

	VkDeviceMemory memory = AllocateDeviceMemory(
		memoryRequirements,
		VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT
	);

	result = vkBindImageMemory(
		_logicalDevice.device,
		image,
		memory,
		0
	);

	if (result != VK_SUCCESS)
		Print("[Vulkan]: Error: failed to bind memory to image");

	VkImageAspectFlags aspectMask = VK_IMAGE_ASPECT_NONE;

	if (usageMask & (dword)ETextureUsageBit::Sampled ||
		usageMask & (dword)ETextureUsageBit::ColorAttachment)
		aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
	if (usageMask & (dword)ETextureUsageBit::DepthStencilAttachment)
		aspectMask = VK_IMAGE_ASPECT_DEPTH_BIT;

	VkImageViewCreateInfo imageViewCreateInfo = {};
	imageViewCreateInfo.sType = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO;
	imageViewCreateInfo.image = image;
	imageViewCreateInfo.viewType = VK_IMAGE_VIEW_TYPE_2D;
	imageViewCreateInfo.format = nativeFormat;
	imageViewCreateInfo.components.r = VK_COMPONENT_SWIZZLE_IDENTITY;
	imageViewCreateInfo.components.g = VK_COMPONENT_SWIZZLE_IDENTITY;
	imageViewCreateInfo.components.b = VK_COMPONENT_SWIZZLE_IDENTITY;
	imageViewCreateInfo.components.a = VK_COMPONENT_SWIZZLE_IDENTITY;
	imageViewCreateInfo.subresourceRange.aspectMask = aspectMask;
	imageViewCreateInfo.subresourceRange.baseMipLevel = 0;
	imageViewCreateInfo.subresourceRange.levelCount = 1;
	imageViewCreateInfo.subresourceRange.baseArrayLayer = 0;
	imageViewCreateInfo.subresourceRange.layerCount = 1;

	VkImageView imageView = VK_NULL_HANDLE;

	vkCreateImageView(
		_logicalDevice.device,
		&imageViewCreateInfo,
		nullptr,
		&imageView
	);

	VkSampler sampler = VK_NULL_HANDLE;
	if (bCreateSampler == True)
	{
		VkSamplerCreateInfo samplerCreateInfo = {};
		samplerCreateInfo.sType = VK_STRUCTURE_TYPE_SAMPLER_CREATE_INFO;
		samplerCreateInfo.magFilter = VK_FILTER_LINEAR;
		samplerCreateInfo.minFilter = VK_FILTER_LINEAR;
		samplerCreateInfo.mipmapMode = VK_SAMPLER_MIPMAP_MODE_NEAREST;
		samplerCreateInfo.addressModeU = VK_SAMPLER_ADDRESS_MODE_REPEAT;
		samplerCreateInfo.addressModeV = VK_SAMPLER_ADDRESS_MODE_REPEAT;
		samplerCreateInfo.addressModeW = VK_SAMPLER_ADDRESS_MODE_REPEAT;
		samplerCreateInfo.mipLodBias = 0.0f;
		samplerCreateInfo.anisotropyEnable = VK_FALSE;
		samplerCreateInfo.maxAnisotropy = 1.0f;
		samplerCreateInfo.compareEnable = VK_FALSE;
		samplerCreateInfo.compareOp = VK_COMPARE_OP_ALWAYS;
		samplerCreateInfo.minLod = 0.0f;
		samplerCreateInfo.maxLod = 0.0f;
		samplerCreateInfo.borderColor = VK_BORDER_COLOR_FLOAT_OPAQUE_WHITE;
		samplerCreateInfo.unnormalizedCoordinates = VK_FALSE;

		result = vkCreateSampler(
			_logicalDevice.device,
			&samplerCreateInfo,
			nullptr,
			&sampler
		);

		if (result != VK_SUCCESS)
			Print("[Vulkan]: Error: failed to create sampler");
	}

	SGPUTextureResource resultTextureInstance = SGPUTextureResource(
		(qword)image,
		(qword)imageView,
		bCreateSampler == True ? (qword)sampler : 0,
		(qword)memory,
		format,
		usageMask,
		dimension,
		size,
		0
	);

	*resultTexture = resultTextureInstance;

	ERenderImageAspectBit aspect_mask = TextureFormatToAspect(resultTextureInstance.format);

	{
		ResetCommandBuffer(ERenderCommandBufferType::Staging, ERenderCommandBufferResetOption::Clear);
		ResetCommandBuffer(ERenderCommandBufferType::Staging, ERenderCommandBufferResetOption::Begin);

		ExecuteImageBarrier(
			ERenderCommandBufferType::Staging,
			resultTextureInstance,
			ERenderPipelineStageBit::TopOfPipe,
			(ERenderPipelineStageBit)((dword)ERenderPipelineStageBit::Pixel | (dword)ERenderPipelineStageBit::Compute),
			ERenderResourceAccessBit::None,
			ERenderResourceAccessBit::ShaderRead,
			aspect_mask,
			ERenderImageLayout::Undefined,
			ERenderImageLayout::ShaderRead
		);

		ResetCommandBuffer(ERenderCommandBufferType::Staging, ERenderCommandBufferResetOption::End);
		ResetFence(ERenderCommandBufferType::Staging);
		SubmitCommands(ERenderCommandBufferType::Staging);
		WaitForFence(ERenderCommandBufferType::Staging);
	}

	return resultTextureInstance;
}

void triton::BGraphicsBackend2Vulkan::WriteTexture(
	const SGPUTextureResource& texture,
	const u8* data
)
{
	if (texture.IsValid() == False || !data)
		return;

	ERenderImageAspectBit aspect_mask = TextureFormatToAspect(texture.format);

	{
		ResetCommandBuffer(ERenderCommandBufferType::Staging, ERenderCommandBufferResetOption::Clear);
		ResetCommandBuffer(ERenderCommandBufferType::Staging, ERenderCommandBufferResetOption::Begin);

		ExecuteImageBarrier(
			ERenderCommandBufferType::Staging,
			texture,
			(ERenderPipelineStageBit)((dword)ERenderPipelineStageBit::Pixel | (dword)ERenderPipelineStageBit::Compute),
			ERenderPipelineStageBit::Transfer,
			ERenderResourceAccessBit::ShaderRead,
			ERenderResourceAccessBit::TransferWrite,
			aspect_mask,
			ERenderImageLayout::ShaderRead,
			ERenderImageLayout::TransferDst
		);

		ResetCommandBuffer(ERenderCommandBufferType::Staging, ERenderCommandBufferResetOption::End);
		ResetFence(ERenderCommandBufferType::Staging);
		SubmitCommands(ERenderCommandBufferType::Staging);
		WaitForFence(ERenderCommandBufferType::Staging);
	}

	{
		const ETextureFormat texture_format = texture.format;
		
		usize pixel_byte_size = 0;

		if (texture_format == ETextureFormat::R8)
			pixel_byte_size = 1;
		else if (texture_format == ETextureFormat::RGBA8 ||
			texture_format == ETextureFormat::RGBA8_SRGB ||
			texture_format == ETextureFormat::RGBA8_SRGB_Mips ||
			texture_format == ETextureFormat::BGRA8_SRGB)
			pixel_byte_size = 4;

		if (!pixel_byte_size)
		{
			Print("Error: pixel byte size = 0");
			return;
		}

		const usize staging_buffer_byte_size = _staging_buffer.byte_size;

		const usize row_byte_size = texture.size.GetX() * pixel_byte_size;
		const usize rows_per_staging = staging_buffer_byte_size / row_byte_size;

		if (!rows_per_staging)
		{
			Print("Error: insufficient staging buffer byte size");
			return;
		}

		const usize row_width = texture.size.GetX();
		const usize row_count = texture.size.GetY();
		
		usize cur_row = 0;

		while (cur_row < row_count)
		{
			VkDeviceMemory staging_memory = (VkDeviceMemory)_staging_buffer.device_memory;

			void* staging_data = nullptr;

			vkMapMemory(
				_logicalDevice.device,
				staging_memory,
				0,
				staging_buffer_byte_size,
				0,
				&staging_data
			);

			const usize data_byte_offset = cur_row * row_byte_size;

			usize rows_to_write = rows_per_staging;

			if (cur_row + rows_to_write > row_count)
				rows_to_write = row_count - cur_row;

			memcpy(staging_data, &data[data_byte_offset], rows_to_write * row_byte_size);

			vkUnmapMemory(_logicalDevice.device, staging_memory);

			const cVector3 image_offset = cVector3(0, cur_row, 0);
			const cVector3 rect_size = cVector3(row_width, rows_to_write, 1);

			ResetCommandBuffer(ERenderCommandBufferType::Staging, ERenderCommandBufferResetOption::Clear);
			ResetCommandBuffer(ERenderCommandBufferType::Staging, ERenderCommandBufferResetOption::Begin);

			CopyBufferToImage(
				ERenderCommandBufferType::Staging,
				_staging_buffer,
				texture,
				aspect_mask,
				image_offset,
				rect_size
			);

			ResetCommandBuffer(ERenderCommandBufferType::Staging, ERenderCommandBufferResetOption::End);
			ResetFence(ERenderCommandBufferType::Staging);
			SubmitCommands(ERenderCommandBufferType::Staging);
			WaitForFence(ERenderCommandBufferType::Staging);
			
			cur_row += rows_to_write;
		}
	}

	{
		ResetCommandBuffer(ERenderCommandBufferType::Staging, ERenderCommandBufferResetOption::Clear);
		ResetCommandBuffer(ERenderCommandBufferType::Staging, ERenderCommandBufferResetOption::Begin);

		ExecuteImageBarrier(
			ERenderCommandBufferType::Staging,
			texture,
			ERenderPipelineStageBit::Transfer,
			(ERenderPipelineStageBit)((dword)ERenderPipelineStageBit::Pixel | (dword)ERenderPipelineStageBit::Compute),
			ERenderResourceAccessBit::TransferWrite,
			ERenderResourceAccessBit::ShaderRead,
			aspect_mask,
			ERenderImageLayout::TransferDst,
			ERenderImageLayout::ShaderRead
		);

		ResetCommandBuffer(ERenderCommandBufferType::Staging, ERenderCommandBufferResetOption::End);
		ResetFence(ERenderCommandBufferType::Staging);
		SubmitCommands(ERenderCommandBufferType::Staging);
		WaitForFence(ERenderCommandBufferType::Staging);
	}
}

void triton::BGraphicsBackend2Vulkan::DestroyTexture(SGPUTextureResource& texture)
{
	if (texture.IsValid())
	{
		if (texture.sampler)
			vkDestroySampler(
				_logicalDevice.device,
				(VkSampler)texture.sampler,
				nullptr
			);

		vkDestroyImageView(
			_logicalDevice.device,
			(VkImageView)texture.view,
			nullptr
		);

		vkDestroyImage(
			_logicalDevice.device,
			(VkImage)texture.instance,
			nullptr
		);

		vkFreeMemory(
			_logicalDevice.device,
			(VkDeviceMemory)texture.device_memory,
			nullptr
		);
	}
}

triton::SGPUBufferResource triton::BGraphicsBackend2Vulkan::CreateBuffer(
	EGPUBufferType type,
	const usize byteSize
)
{
	VkResult result;

	VkBufferCreateInfo bufferCreateInfo = {};
	bufferCreateInfo.sType = VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO;
	bufferCreateInfo.size = byteSize;
	bufferCreateInfo.usage = BufferTypeToNative(type);
	bufferCreateInfo.sharingMode = VK_SHARING_MODE_EXCLUSIVE;

	VkBuffer buffer = VK_NULL_HANDLE;

	result = vkCreateBuffer(
		_logicalDevice.device,
		&bufferCreateInfo,
		nullptr,
		&buffer
	);

	if (result != VK_SUCCESS)
		Print("[Vulkan]: Error: failed to create buffer");

	VkMemoryRequirements memoryRequirements = {};
	vkGetBufferMemoryRequirements(
		_logicalDevice.device,
		buffer,
		&memoryRequirements
	);

	dword memoryPropertyBits = VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT;

	if (type == EGPUBufferType::Staging)
		memoryPropertyBits |=
			VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT |
			VK_MEMORY_PROPERTY_HOST_COHERENT_BIT;

	VkDeviceMemory memory = AllocateDeviceMemory(
		memoryRequirements,
		memoryPropertyBits
	);

	result = vkBindBufferMemory(
		_logicalDevice.device,
		buffer,
		memory,
		0
	);

	if (result != VK_SUCCESS)
		Print("[Vulkan]: Error: failed to bind memory to buffer");

	return SGPUBufferResource(
		(qword)buffer,
		0,
		(qword)memory,
		type,
		byteSize,
		0
	);
}

void triton::BGraphicsBackend2Vulkan::WriteBuffer(
	const SGPUBufferResource& buffer,
	usize offset,
	const u8* data,
	usize data_byte_size
)
{
	if (buffer.IsValid() == False ||
		!data_byte_size ||
		offset + data_byte_size > buffer.byte_size)
		return;

	{
		ResetCommandBuffer(ERenderCommandBufferType::Staging, ERenderCommandBufferResetOption::Clear);
		ResetCommandBuffer(ERenderCommandBufferType::Staging, ERenderCommandBufferResetOption::Begin);

		ExecuteBufferBarrier(
			ERenderCommandBufferType::Staging,
			buffer,
			(ERenderPipelineStageBit)(
				(dword)ERenderPipelineStageBit::VertexInput |
				(dword)ERenderPipelineStageBit::Vertex |
				(dword)ERenderPipelineStageBit::Pixel |
				(dword)ERenderPipelineStageBit::Compute
			),
			ERenderPipelineStageBit::Transfer,
			(ERenderResourceAccessBit)(
				(dword)ERenderResourceAccessBit::IndexRead |
				(dword)ERenderResourceAccessBit::ShaderRead |
				(dword)ERenderResourceAccessBit::ShaderWrite
			),
			ERenderResourceAccessBit::TransferWrite
		);

		ResetCommandBuffer(ERenderCommandBufferType::Staging, ERenderCommandBufferResetOption::End);
		ResetFence(ERenderCommandBufferType::Staging);
		SubmitCommands(ERenderCommandBufferType::Staging);
		WaitForFence(ERenderCommandBufferType::Staging);
	}

	{
		const usize staging_buffer_byte_size = _staging_buffer.byte_size;

		const usize last_pos = offset + data_byte_size;

		usize cur_pos = offset;

		while (cur_pos < last_pos)
		{
			VkDeviceMemory staging_memory = (VkDeviceMemory)_staging_buffer.device_memory;

			void* staging_data = nullptr;

			vkMapMemory(
				_logicalDevice.device,
				(VkDeviceMemory)staging_memory,
				0,
				staging_buffer_byte_size,
				0,
				&staging_data
			);

			usize bytes_to_write = staging_buffer_byte_size;

			if (cur_pos + bytes_to_write > last_pos)
				bytes_to_write = last_pos - cur_pos;

			memcpy(staging_data, data, bytes_to_write);

			vkUnmapMemory(_logicalDevice.device, staging_memory);

			ResetCommandBuffer(ERenderCommandBufferType::Staging, ERenderCommandBufferResetOption::Clear);
			ResetCommandBuffer(ERenderCommandBufferType::Staging, ERenderCommandBufferResetOption::Begin);

			CopyBufferToBuffer(
				ERenderCommandBufferType::Staging,
				_staging_buffer,
				buffer,
				0,
				cur_pos,
				bytes_to_write
			);

			ResetCommandBuffer(ERenderCommandBufferType::Staging, ERenderCommandBufferResetOption::End);
			ResetFence(ERenderCommandBufferType::Staging);
			SubmitCommands(ERenderCommandBufferType::Staging);
			WaitForFence(ERenderCommandBufferType::Staging);

			cur_pos += bytes_to_write;
		}
	}

	{
		ResetCommandBuffer(ERenderCommandBufferType::Staging, ERenderCommandBufferResetOption::Clear);
		ResetCommandBuffer(ERenderCommandBufferType::Staging, ERenderCommandBufferResetOption::Begin);

		ExecuteBufferBarrier(
			ERenderCommandBufferType::Staging,
			buffer,
			ERenderPipelineStageBit::Transfer,
			(ERenderPipelineStageBit)(
				(dword)ERenderPipelineStageBit::VertexInput |
				(dword)ERenderPipelineStageBit::Vertex |
				(dword)ERenderPipelineStageBit::Pixel |
				(dword)ERenderPipelineStageBit::Compute
			),
			ERenderResourceAccessBit::TransferWrite,
			(ERenderResourceAccessBit)(
				(dword)ERenderResourceAccessBit::IndexRead |
				(dword)ERenderResourceAccessBit::ShaderRead |
				(dword)ERenderResourceAccessBit::ShaderWrite
			)
		);

		ResetCommandBuffer(ERenderCommandBufferType::Staging, ERenderCommandBufferResetOption::End);
		ResetFence(ERenderCommandBufferType::Staging);
		SubmitCommands(ERenderCommandBufferType::Staging);
		WaitForFence(ERenderCommandBufferType::Staging);
	}
}

void triton::BGraphicsBackend2Vulkan::DestroyBuffer(SGPUBufferResource& buffer)
{
	if (buffer.IsValid() == True)
	{
		vkDestroyBuffer(
			_logicalDevice.device,
			(VkBuffer)buffer.instance,
			nullptr
		);

		vkFreeMemory(
			_logicalDevice.device,
			(VkDeviceMemory)buffer.device_memory,
			nullptr
		);

		buffer.Invalidate();
	}
}

triton::SGPURenderTargetResource triton::BGraphicsBackend2Vulkan::CreateRenderTarget(
	const std::vector<SGPUTextureResource>& colorAttachments,
	const SGPUTextureResource& depthAttachment
)
{
	return SGPURenderTargetResource(
		(cpuword)1,
		0,
		colorAttachments,
		depthAttachment
	);
}

triton::SGPUPipelineResource triton::BGraphicsBackend2Vulkan::CreatePipeline(
	const SGPUShaderResource& shader,
	const std::vector<SGPUBindingGroupLayoutResource>& bindingGroupLayouts,
	types::boolean bUsePushConstants,
	const std::optional<std::reference_wrapper<SViewport>> viewport,
	std::optional<std::reference_wrapper<SGPURenderTargetResource>> renderTarget,
	const std::optional<std::reference_wrapper<SGPURenderPassResource>> renderPass,
	ERenderPrimitiveTopology primitiveTopology,
	EVertexBufferFormat vertexBufferFormat
)
{
	const dword stageMask = shader.stage_mask;

	const boolean bIsVertexPixel =
		((stageMask & (dword)EShaderStageBit::Vertex) &&
		(stageMask & (dword)EShaderStageBit::Pixel)) ? True : False;

	const boolean bIsCompute =
		(stageMask & (dword)EShaderStageBit::Compute) ? True : False;

	usize stageCount = 0;
	std::vector<VkPipelineShaderStageCreateInfo> stageCreateInfos;

	if (bIsVertexPixel == True)
	{
		stageCount = 2;
		
		VkPipelineShaderStageCreateInfo vertexStageCreateInfo = {};
		vertexStageCreateInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;
		vertexStageCreateInfo.stage = VK_SHADER_STAGE_VERTEX_BIT;
		vertexStageCreateInfo.module = (VkShaderModule)shader.modules[0];
		vertexStageCreateInfo.pName = "main";

		stageCreateInfos.push_back(vertexStageCreateInfo);

		VkPipelineShaderStageCreateInfo pixelStageCreateInfo = {};
		pixelStageCreateInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;
		pixelStageCreateInfo.stage = VK_SHADER_STAGE_FRAGMENT_BIT;
		pixelStageCreateInfo.module = (VkShaderModule)shader.modules[1];
		pixelStageCreateInfo.pName = "main";

		stageCreateInfos.push_back(pixelStageCreateInfo);
	}
	else if (bIsCompute == True)
	{
		stageCount = 1;

		VkPipelineShaderStageCreateInfo computeStageCreateInfo = {};
		computeStageCreateInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;
		computeStageCreateInfo.stage = VK_SHADER_STAGE_COMPUTE_BIT;
		computeStageCreateInfo.module = (VkShaderModule)shader.modules[0];
		computeStageCreateInfo.pName = "main";

		stageCreateInfos.push_back(computeStageCreateInfo);
	}

	VkPipeline pipeline = VK_NULL_HANDLE;

	if (bIsVertexPixel == True)
	{
		VkVertexInputBindingDescription vertexInputBinding = {};
		vertexInputBinding.binding = 0;
		vertexInputBinding.inputRate = VK_VERTEX_INPUT_RATE_VERTEX;

		if (vertexBufferFormat == EVertexBufferFormat::Rigid)
			vertexInputBinding.stride = sizeof(SRigidVertexGPULayout);
		else if (vertexBufferFormat == EVertexBufferFormat::Skinned)
			vertexInputBinding.stride = sizeof(SSkinnedVertexGPULayout);

		std::vector<VkVertexInputAttributeDescription> vertexInputAttributes;

		if (vertexBufferFormat == EVertexBufferFormat::Rigid)
		{
			vertexInputAttributes.resize(4);
			
			vertexInputAttributes[0].binding = 0;
			vertexInputAttributes[0].format = VK_FORMAT_R32G32B32_SFLOAT;
			vertexInputAttributes[0].location = 0;
			vertexInputAttributes[0].offset = offsetof(SRigidVertexGPULayout, position);

			vertexInputAttributes[1].binding = 0;
			vertexInputAttributes[1].format = VK_FORMAT_R32G32_SFLOAT;
			vertexInputAttributes[1].location = 1;
			vertexInputAttributes[1].offset = offsetof(SRigidVertexGPULayout, texcoord);

			vertexInputAttributes[2].binding = 0;
			vertexInputAttributes[2].format = VK_FORMAT_R32G32B32_SFLOAT;
			vertexInputAttributes[2].location = 2;
			vertexInputAttributes[2].offset = offsetof(SRigidVertexGPULayout, normal);

			vertexInputAttributes[3].binding = 0;
			vertexInputAttributes[3].format = VK_FORMAT_R32G32B32A32_SFLOAT;
			vertexInputAttributes[3].location = 3;
			vertexInputAttributes[3].offset = offsetof(SRigidVertexGPULayout, tangent);
		}

		VkPipelineVertexInputStateCreateInfo vertexInputStateCreateInfo = {};
		vertexInputStateCreateInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_VERTEX_INPUT_STATE_CREATE_INFO;
		
		if (vertexBufferFormat != EVertexBufferFormat::Unknown)
		{
			vertexInputStateCreateInfo.vertexBindingDescriptionCount = 1;
			vertexInputStateCreateInfo.pVertexBindingDescriptions = &vertexInputBinding;
			vertexInputStateCreateInfo.vertexAttributeDescriptionCount = vertexInputAttributes.size();
			vertexInputStateCreateInfo.pVertexAttributeDescriptions = vertexInputAttributes.data();
		}

		VkPipelineInputAssemblyStateCreateInfo inputAssemblyStateCreateInfo = {};
		inputAssemblyStateCreateInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_INPUT_ASSEMBLY_STATE_CREATE_INFO;
		inputAssemblyStateCreateInfo.topology = PrimitiveTopologyToNative(primitiveTopology);
		inputAssemblyStateCreateInfo.primitiveRestartEnable = VK_FALSE;

		VkViewport nativeViewport = ViewportToNative(viewport->get());

		VkRect2D scissor = {};
		scissor.offset.x = 0;
		scissor.offset.y = 0;
		scissor.extent.width = viewport->get().rect.GetZ();
		scissor.extent.height = viewport->get().rect.GetW();

		VkPipelineViewportStateCreateInfo viewportStateCreateInfo = {};
		viewportStateCreateInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_VIEWPORT_STATE_CREATE_INFO;
		viewportStateCreateInfo.viewportCount = 1;
		viewportStateCreateInfo.pViewports = &nativeViewport;
		viewportStateCreateInfo.scissorCount = 1;
		viewportStateCreateInfo.pScissors = &scissor;

		VkPipelineRasterizationStateCreateInfo rasterizationStateCreateInfo = {};
		rasterizationStateCreateInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_RASTERIZATION_STATE_CREATE_INFO;
		rasterizationStateCreateInfo.depthClampEnable = VK_FALSE;
		rasterizationStateCreateInfo.rasterizerDiscardEnable = VK_FALSE;
		rasterizationStateCreateInfo.polygonMode = VK_POLYGON_MODE_FILL;
		rasterizationStateCreateInfo.cullMode = VK_CULL_MODE_NONE;
		rasterizationStateCreateInfo.frontFace = VK_FRONT_FACE_COUNTER_CLOCKWISE;
		rasterizationStateCreateInfo.depthBiasEnable = VK_FALSE;
		rasterizationStateCreateInfo.depthBiasConstantFactor = 0.0f;
		rasterizationStateCreateInfo.depthBiasClamp = 0.0f;
		rasterizationStateCreateInfo.depthBiasSlopeFactor = 0.0f;
		rasterizationStateCreateInfo.lineWidth = 1.0f;

		VkPipelineMultisampleStateCreateInfo multisampleStateCreateInfo = {};
		multisampleStateCreateInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_MULTISAMPLE_STATE_CREATE_INFO;
		multisampleStateCreateInfo.rasterizationSamples = VK_SAMPLE_COUNT_1_BIT;
		multisampleStateCreateInfo.sampleShadingEnable = VK_FALSE;
		multisampleStateCreateInfo.minSampleShading = 1.0f;
		multisampleStateCreateInfo.alphaToCoverageEnable = VK_FALSE;
		multisampleStateCreateInfo.alphaToOneEnable = VK_FALSE;

		VkPipelineColorBlendAttachmentState colorBlendAttachmentState = {};
		colorBlendAttachmentState.blendEnable = VK_FALSE;
		colorBlendAttachmentState.colorWriteMask =
			VK_COLOR_COMPONENT_R_BIT |
			VK_COLOR_COMPONENT_G_BIT |
			VK_COLOR_COMPONENT_B_BIT |
			VK_COLOR_COMPONENT_A_BIT;

		VkPipelineColorBlendStateCreateInfo colorBlendStateCreateInfo = {};
		colorBlendStateCreateInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_COLOR_BLEND_STATE_CREATE_INFO;
		colorBlendStateCreateInfo.logicOpEnable = VK_FALSE;
		colorBlendStateCreateInfo.logicOp = VK_LOGIC_OP_COPY;
		colorBlendStateCreateInfo.attachmentCount = 1;
		colorBlendStateCreateInfo.pAttachments = &colorBlendAttachmentState;
		colorBlendStateCreateInfo.blendConstants[0] = 0.0f;
		colorBlendStateCreateInfo.blendConstants[1] = 0.0f;
		colorBlendStateCreateInfo.blendConstants[2] = 0.0f;
		colorBlendStateCreateInfo.blendConstants[3] = 0.0f;

		VkPipelineLayoutCreateInfo pipelineLayoutCreateInfo = {};
		pipelineLayoutCreateInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO;

		std::vector<VkDescriptorSetLayout> nativeDescriptorSetLayouts;
		if (bindingGroupLayouts.size())
		{
			for (usize i = 0; i < bindingGroupLayouts.size(); ++i)
				nativeDescriptorSetLayouts.push_back(
					(VkDescriptorSetLayout)bindingGroupLayouts.at(i).instance
				);
			
			pipelineLayoutCreateInfo.setLayoutCount = nativeDescriptorSetLayouts.size();
			pipelineLayoutCreateInfo.pSetLayouts = nativeDescriptorSetLayouts.data();
		}

		VkPushConstantRange pushConstantRange = {};
		if (bUsePushConstants == True)
		{
			pushConstantRange.offset = 0;
			pushConstantRange.size = sizeof(SRenderPassPushConstantsLayout);
			pushConstantRange.stageFlags = VK_SHADER_STAGE_VERTEX_BIT;

			pipelineLayoutCreateInfo.pushConstantRangeCount = 1;
			pipelineLayoutCreateInfo.pPushConstantRanges = &pushConstantRange;
		}

		VkPipelineLayout pipelineLayout = VK_NULL_HANDLE;

		VkResult result = vkCreatePipelineLayout(
			_logicalDevice.device,
			&pipelineLayoutCreateInfo,
			nullptr,
			&pipelineLayout
		);

		if (result != VK_SUCCESS)
			Print("[Vulkan]: Error: failed to create pipeline layout");

		VkGraphicsPipelineCreateInfo graphicsPipelineCreateInfo = {};
		graphicsPipelineCreateInfo.sType = VK_STRUCTURE_TYPE_GRAPHICS_PIPELINE_CREATE_INFO;
		graphicsPipelineCreateInfo.stageCount = stageCount;
		graphicsPipelineCreateInfo.pStages = stageCreateInfos.data();
		graphicsPipelineCreateInfo.pVertexInputState = &vertexInputStateCreateInfo;
		graphicsPipelineCreateInfo.pInputAssemblyState = &inputAssemblyStateCreateInfo;
		graphicsPipelineCreateInfo.pTessellationState = nullptr;
		graphicsPipelineCreateInfo.pViewportState = &viewportStateCreateInfo;
		graphicsPipelineCreateInfo.pRasterizationState = &rasterizationStateCreateInfo;
		graphicsPipelineCreateInfo.pMultisampleState = &multisampleStateCreateInfo;

		VkPipelineDepthStencilStateCreateInfo depthStencilStateCreateInfo = {};

		const boolean bHasDepth = renderTarget->get().depth_attachment.IsValid();

		if (bHasDepth == True)
		{
			depthStencilStateCreateInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_DEPTH_STENCIL_STATE_CREATE_INFO;
			depthStencilStateCreateInfo.depthTestEnable = VK_TRUE;
			depthStencilStateCreateInfo.depthWriteEnable = VK_TRUE;
			depthStencilStateCreateInfo.depthCompareOp = VK_COMPARE_OP_LESS;
			depthStencilStateCreateInfo.depthBoundsTestEnable = VK_FALSE;
			depthStencilStateCreateInfo.minDepthBounds = 0.0f;
			depthStencilStateCreateInfo.maxDepthBounds = 1.0f;
			depthStencilStateCreateInfo.stencilTestEnable = VK_FALSE;
			depthStencilStateCreateInfo.front = {};
			depthStencilStateCreateInfo.back = {};

			graphicsPipelineCreateInfo.pDepthStencilState = &depthStencilStateCreateInfo;
		}

		graphicsPipelineCreateInfo.pColorBlendState = &colorBlendStateCreateInfo;
		graphicsPipelineCreateInfo.pDynamicState = nullptr;
		graphicsPipelineCreateInfo.layout = pipelineLayout;
		graphicsPipelineCreateInfo.renderPass = (VkRenderPass)renderPass->get().instance;
		graphicsPipelineCreateInfo.subpass = 0;
		graphicsPipelineCreateInfo.basePipelineHandle = VK_NULL_HANDLE;
		graphicsPipelineCreateInfo.basePipelineIndex = -1;

		vkCreateGraphicsPipelines(
			_logicalDevice.device,
			VK_NULL_HANDLE,
			1,
			&graphicsPipelineCreateInfo,
			nullptr,
			&pipeline
		);

		return SGPUPipelineResource(
			(qword)pipeline,
			0,
			ERenderPipelineBindPoint::Graphics,
			(qword)pipelineLayout,
			bindingGroupLayouts
		);
	}
	else if (bIsCompute == True)
	{
		VkPipelineLayout pipeline_layout;
		VkPipelineLayoutCreateInfo pipeline_layout_create_info = {};

		{
			pipeline_layout_create_info.sType = VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO;

			std::vector<VkDescriptorSetLayout> native_descriptor_set_layouts;

			if (bindingGroupLayouts.size())
			{
				for (usize i = 0; i < bindingGroupLayouts.size(); ++i)
					native_descriptor_set_layouts.push_back(
						(VkDescriptorSetLayout)bindingGroupLayouts.at(i).instance
					);

				pipeline_layout_create_info.setLayoutCount = native_descriptor_set_layouts.size();
				pipeline_layout_create_info.pSetLayouts = native_descriptor_set_layouts.data();
			}

			VkPushConstantRange pushConstantRange = {};

			if (bUsePushConstants == True)
			{
				pushConstantRange.offset = 0;
				pushConstantRange.size = sizeof(SRenderPassPushConstantsLayout);
				pushConstantRange.stageFlags = VK_SHADER_STAGE_VERTEX_BIT;

				pipeline_layout_create_info.pushConstantRangeCount = 1;
				pipeline_layout_create_info.pPushConstantRanges = &pushConstantRange;
			}

			vkCreatePipelineLayout(
				_logicalDevice.device,
				&pipeline_layout_create_info,
				nullptr,
				&pipeline_layout
			);
		}

		VkPipeline pipeline;
		VkComputePipelineCreateInfo compute_pipeline_create_info = {};

		{
			compute_pipeline_create_info.sType = VK_STRUCTURE_TYPE_COMPUTE_PIPELINE_CREATE_INFO;
			compute_pipeline_create_info.stage = stageCreateInfos.at(0);
			compute_pipeline_create_info.layout = pipeline_layout;

			vkCreateComputePipelines(
				_logicalDevice.device,
				VK_NULL_HANDLE,
				1,
				&compute_pipeline_create_info,
				nullptr,
				&pipeline
			);
		}

		{
			return SGPUPipelineResource(
				(qword)pipeline,
				0,
				ERenderPipelineBindPoint::Compute,
				(qword)pipeline_layout,
				bindingGroupLayouts
			);
		}
	}

	return SGPUPipelineResource::Invalid();
}

void triton::BGraphicsBackend2Vulkan::DestroyPipeline(SGPUPipelineResource& pipeline)
{
	if (pipeline.IsValid())
	{
		vkDestroyPipeline(
			_logicalDevice.device,
			(VkPipeline)pipeline.instance,
			nullptr
		);

		pipeline.Invalidate();
	}
}

triton::SGPUBindingGroupLayoutResource triton::BGraphicsBackend2Vulkan::CreateBindingGroupLayout(
	const std::vector<SRenderBindingGroupBinding>& bindings
)
{
	const usize bindingCount = bindings.size();

	if (!bindingCount)
		return SGPUBindingGroupLayoutResource::Invalid();

	std::vector<VkDescriptorSetLayoutBinding> nativeBindings(bindingCount);

	for (usize i = 0; i < bindingCount; ++i)
	{
		nativeBindings[i].binding = bindings[i].index;

		if (bindings[i].type == ERenderBindingGroupBindingType::StorageBuffer)
			nativeBindings[i].descriptorType = VK_DESCRIPTOR_TYPE_STORAGE_BUFFER;
		else if (bindings[i].type == ERenderBindingGroupBindingType::TextureSampler)
			nativeBindings[i].descriptorType = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;

		nativeBindings[i].descriptorCount = 1;

		if ((dword)bindings[i].shaderStageMask & (dword)EShaderStageBit::Vertex)
			nativeBindings[i].stageFlags |= VK_SHADER_STAGE_VERTEX_BIT;
		if ((dword)bindings[i].shaderStageMask & (dword)EShaderStageBit::Pixel)
			nativeBindings[i].stageFlags |= VK_SHADER_STAGE_FRAGMENT_BIT;
	}

	VkDescriptorSetLayoutCreateInfo descriptorSetLayoutInfo = {};
	descriptorSetLayoutInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_CREATE_INFO;
	descriptorSetLayoutInfo.bindingCount = nativeBindings.size();
	descriptorSetLayoutInfo.pBindings = nativeBindings.data();

	VkDescriptorSetLayout descriptorSetLayout = VK_NULL_HANDLE;

	VkResult result = vkCreateDescriptorSetLayout(
		_logicalDevice.device,
		&descriptorSetLayoutInfo,
		nullptr,
		&descriptorSetLayout
	);

	if (result != VK_SUCCESS)
		Print("[Vulkan]: Error: failed to create descriptor set layout");

	return SGPUBindingGroupLayoutResource(
		(qword)descriptorSetLayout,
		0,
		bindings
	);
}

void triton::BGraphicsBackend2Vulkan::DestroyBindingGroupLayout(SGPUBindingGroupLayoutResource& bindingGroupLayout)
{
	if (bindingGroupLayout.IsValid() == True)
	{
		vkDestroyDescriptorSetLayout(
			_logicalDevice.device,
			(VkDescriptorSetLayout)bindingGroupLayout.instance,
			nullptr
		);

		bindingGroupLayout.Invalidate();
	}
}

triton::SGPUBindingGroupResource triton::BGraphicsBackend2Vulkan::CreateBindingGroup(
	s32 index,
	const SGPUBindingGroupLayoutResource& bindingGroupLayout,
	const std::vector<SRenderBindingGroupBinding>& buffersToBind,
	const std::vector<SRenderBindingGroupBinding>& texturesToBind
)
{
	const auto& bindingGroupBindings = bindingGroupLayout.bindings;
	const usize bindingCount = bindingGroupBindings.size();

	if (!bindingCount)
		return SGPUBindingGroupResource::Invalid();

	VkDescriptorSetLayout descriptorSetLayout = (VkDescriptorSetLayout)bindingGroupLayout.instance;

	VkDescriptorSetAllocateInfo descriptorSetAllocInfo = {};
	descriptorSetAllocInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_ALLOCATE_INFO;
	descriptorSetAllocInfo.descriptorPool = _descriptorPool;
	descriptorSetAllocInfo.descriptorSetCount = 1;
	descriptorSetAllocInfo.pSetLayouts = &descriptorSetLayout;

	VkDescriptorSet descriptorSet = VK_NULL_HANDLE;

	VkResult result = vkAllocateDescriptorSets(
		_logicalDevice.device,
		&descriptorSetAllocInfo,
		&descriptorSet
	);

	if (result != VK_SUCCESS)
		Print("[Vulkan]: Error: failed to allocate descriptor set");

	std::vector<VkWriteDescriptorSet> nativeWriteDescriptorSetElements(bindingCount);
	std::vector<VkDescriptorBufferInfo> nativeDescriptorBufferInfos(bindingCount);
	std::vector<VkDescriptorImageInfo> nativeDescriptorImageInfos(bindingCount);

	for (usize i = 0; i < bindingCount; ++i)
	{
		const SRenderBindingGroupBinding& bindingGroupBinding = bindingGroupBindings[i];

		if (bindingGroupBinding.type == ERenderBindingGroupBindingType::StorageBuffer)
		{
			const SGPUBufferResource* bufferToBind = nullptr;
			
			for (usize j = 0; j < buffersToBind.size(); ++j)
				if (buffersToBind[j].index == bindingGroupBinding.index)
					bufferToBind = (const SGPUBufferResource*)buffersToBind[j].resource;

			nativeDescriptorBufferInfos[i].buffer = (VkBuffer)bufferToBind->instance;
			nativeDescriptorBufferInfos[i].offset = 0;
			nativeDescriptorBufferInfos[i].range = VK_WHOLE_SIZE;

			VkWriteDescriptorSet writeDescriptorSet = {};
			writeDescriptorSet.sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
			writeDescriptorSet.dstSet = descriptorSet;
			writeDescriptorSet.dstBinding = bindingGroupBinding.index;
			writeDescriptorSet.dstArrayElement = 0;
			writeDescriptorSet.descriptorType = VK_DESCRIPTOR_TYPE_STORAGE_BUFFER;
			writeDescriptorSet.descriptorCount = 1;
			writeDescriptorSet.pBufferInfo = &nativeDescriptorBufferInfos[i];

			nativeWriteDescriptorSetElements[i] = writeDescriptorSet;
		}
		else if (bindingGroupBinding.type == ERenderBindingGroupBindingType::TextureSampler)
		{
			const SGPUTextureResource* textureToBind = nullptr;

			for (usize j = 0; j < texturesToBind.size(); ++j)
				if (texturesToBind[j].index == bindingGroupBinding.index)
					textureToBind = (const SGPUTextureResource*)texturesToBind[j].resource;

			nativeDescriptorImageInfos[i].imageLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;
			nativeDescriptorImageInfos[i].imageView = (VkImageView)textureToBind->view;
			nativeDescriptorImageInfos[i].sampler = (VkSampler)textureToBind->sampler;

			VkWriteDescriptorSet writeDescriptorSet = {};
			writeDescriptorSet.sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
			writeDescriptorSet.dstSet = descriptorSet;
			writeDescriptorSet.dstBinding = bindingGroupBinding.index;
			writeDescriptorSet.dstArrayElement = 0;
			writeDescriptorSet.descriptorType = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
			writeDescriptorSet.descriptorCount = 1;
			writeDescriptorSet.pImageInfo = &nativeDescriptorImageInfos[i];

			nativeWriteDescriptorSetElements[i] = writeDescriptorSet;
		}
	}

	vkUpdateDescriptorSets(
		_logicalDevice.device,
		nativeWriteDescriptorSetElements.size(),
		nativeWriteDescriptorSetElements.data(),
		0,
		nullptr
	);

	return SGPUBindingGroupResource(
		(qword)descriptorSet,
		0,
		(qword)_descriptorPool,
		index
	);
}

void triton::BGraphicsBackend2Vulkan::DestroyBindingGroup(SGPUBindingGroupResource& bindingGroup)
{
	if (bindingGroup.IsValid() == True)
	{
		VkDescriptorSet descriptorSet = (VkDescriptorSet)bindingGroup.instance;
		vkFreeDescriptorSets(
			_logicalDevice.device,
			(VkDescriptorPool)bindingGroup.pool,
			1,
			&descriptorSet
		);

		bindingGroup.Invalidate();
	}
}

void triton::BGraphicsBackend2Vulkan::DestroyRenderTarget(SGPURenderTargetResource& renderTarget)
{
	if (renderTarget.IsValid())
		renderTarget.Invalidate();
}

triton::SGPURenderPassResource triton::BGraphicsBackend2Vulkan::CreateRenderPass(
	SGPURenderTargetResource& renderTarget,
	boolean bClearRenderTarget,
	const cVector4& clearColor,
	types::f32 clearDepth,
	const std::vector<ERenderResourceUsage>& srcAttachmentsUsage,
	const std::vector<ERenderResourceUsage>& dstAttachmentsUsage
)
{
	const usize colorAttachmentCount = renderTarget.color_attachments.size();
	const boolean hasDepth = renderTarget.depth_attachment.IsValid();
	const usize depthAttachmentCount = hasDepth == True ? 1 : 0;

	const usize attachmentCount =  colorAttachmentCount + depthAttachmentCount;

	std::vector<VkAttachmentDescription> attachments(attachmentCount);

	std::vector<VkAttachmentReference> colorReferences(colorAttachmentCount);
	VkAttachmentReference depthReference = {};

	VkAttachmentLoadOp loadOp = VK_ATTACHMENT_LOAD_OP_NONE;
	if (bClearRenderTarget == True)
		loadOp = VK_ATTACHMENT_LOAD_OP_CLEAR;
	else if (bClearRenderTarget == False)
		loadOp = VK_ATTACHMENT_LOAD_OP_LOAD;

	for (usize i = 0; i < colorAttachmentCount; ++i)
	{
		attachments[i].format = TextureFormatToNative(
			renderTarget.color_attachments[i].format
		);
		attachments[i].samples = VK_SAMPLE_COUNT_1_BIT;
		attachments[i].loadOp = loadOp;
		attachments[i].storeOp = VK_ATTACHMENT_STORE_OP_STORE;
		attachments[i].stencilLoadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE;
		attachments[i].stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
		attachments[i].initialLayout = AttachmentUsageToNativeLayout(
			srcAttachmentsUsage[i]
		);
		attachments[i].finalLayout = AttachmentUsageToNativeLayout(
			dstAttachmentsUsage[i]
		);

		colorReferences[i].attachment = i;
		colorReferences[i].layout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;
	}

	if (hasDepth)
	{
		attachments[colorAttachmentCount].format = TextureFormatToNative(
			renderTarget.depth_attachment.format
		);
		attachments[colorAttachmentCount].samples = VK_SAMPLE_COUNT_1_BIT;
		attachments[colorAttachmentCount].loadOp = loadOp;
		attachments[colorAttachmentCount].storeOp = VK_ATTACHMENT_STORE_OP_STORE;
		attachments[colorAttachmentCount].stencilLoadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE;
		attachments[colorAttachmentCount].stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
		attachments[colorAttachmentCount].initialLayout = AttachmentUsageToNativeLayout(
			srcAttachmentsUsage[colorAttachmentCount]
		);
		attachments[colorAttachmentCount].finalLayout = AttachmentUsageToNativeLayout(
			dstAttachmentsUsage[colorAttachmentCount]
		);

		depthReference.attachment = colorAttachmentCount;
		depthReference.layout = VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL;
	}

	VkSubpassDescription subpass = {};

	subpass.pipelineBindPoint = VK_PIPELINE_BIND_POINT_GRAPHICS;
	subpass.colorAttachmentCount = (uint32_t)colorReferences.size();
	subpass.pColorAttachments = colorReferences.data();

	if (hasDepth)
		subpass.pDepthStencilAttachment = &depthReference;

	std::vector<VkSubpassDependency> dependencies;

	for (usize i = 0; i < attachmentCount; ++i)
	{
		VkSubpassDependency inDependency = {};
		VkSubpassDependency outDependency = {};

		inDependency.srcSubpass = VK_SUBPASS_EXTERNAL;
		inDependency.dstSubpass = 0;

		outDependency.srcSubpass = 0;
		outDependency.dstSubpass = VK_SUBPASS_EXTERNAL;

		boolean bUseOutDependency = True;

		if (srcAttachmentsUsage[i] == ERenderResourceUsage::Unknown)
		{
			if (dstAttachmentsUsage[i] == ERenderResourceUsage::PixelShaderRead)
			{
				inDependency.srcStageMask = VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT;
				inDependency.srcAccessMask = VK_ACCESS_NONE;
			}
			else if (dstAttachmentsUsage[i] == ERenderResourceUsage::Present)
			{
				inDependency.srcStageMask = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;
				inDependency.srcAccessMask = VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT;
			}
		}
		else if (srcAttachmentsUsage[i] == ERenderResourceUsage::ColorAttachment)
		{
		}
		else if (srcAttachmentsUsage[i] == ERenderResourceUsage::DepthAttachment)
		{
		}
		else if (srcAttachmentsUsage[i] == ERenderResourceUsage::VertexShaderRead)
		{
		}
		else if (srcAttachmentsUsage[i] == ERenderResourceUsage::PixelShaderRead)
		{
		}
		else if (srcAttachmentsUsage[i] == ERenderResourceUsage::Present)
		{
		}

		if (dstAttachmentsUsage[i] == ERenderResourceUsage::Unknown)
		{
		}
		else if (dstAttachmentsUsage[i] == ERenderResourceUsage::ColorAttachment)
		{
		}
		else if (dstAttachmentsUsage[i] == ERenderResourceUsage::DepthAttachment)
		{
			inDependency.dstStageMask =
				VK_PIPELINE_STAGE_EARLY_FRAGMENT_TESTS_BIT |
				VK_PIPELINE_STAGE_LATE_FRAGMENT_TESTS_BIT;
			inDependency.dstAccessMask =
				VK_ACCESS_DEPTH_STENCIL_ATTACHMENT_WRITE_BIT |
				VK_ACCESS_DEPTH_STENCIL_ATTACHMENT_READ_BIT;

			outDependency.srcStageMask =
				VK_PIPELINE_STAGE_EARLY_FRAGMENT_TESTS_BIT |
				VK_PIPELINE_STAGE_LATE_FRAGMENT_TESTS_BIT;
			outDependency.srcAccessMask =
				VK_ACCESS_DEPTH_STENCIL_ATTACHMENT_WRITE_BIT |
				VK_ACCESS_DEPTH_STENCIL_ATTACHMENT_READ_BIT;
			outDependency.dstStageMask = VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT;
			outDependency.dstAccessMask = VK_ACCESS_SHADER_READ_BIT;
		}
		else if (dstAttachmentsUsage[i] == ERenderResourceUsage::VertexShaderRead)
		{
		}
		else if (dstAttachmentsUsage[i] == ERenderResourceUsage::PixelShaderRead)
		{
			inDependency.dstStageMask =
				VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;
			inDependency.dstAccessMask =
				VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT |
				VK_ACCESS_COLOR_ATTACHMENT_READ_BIT;

			outDependency.srcStageMask =
				VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;
			outDependency.srcAccessMask =
				VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT |
				VK_ACCESS_COLOR_ATTACHMENT_READ_BIT;
			outDependency.dstStageMask = VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT;
			outDependency.dstAccessMask = VK_ACCESS_SHADER_READ_BIT;
		}
		else if (dstAttachmentsUsage[i] == ERenderResourceUsage::Present)
		{
			inDependency.dstStageMask = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;
			inDependency.dstAccessMask = VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT;

			bUseOutDependency = False;
		}

		dependencies.push_back(inDependency);
		if (bUseOutDependency == True)
			dependencies.push_back(outDependency);
	}

	/*if (dstAttachmentsUsage[0] == ERenderResourceUsage::PixelShaderRead)
	{
		dependencies[0].srcSubpass = VK_SUBPASS_EXTERNAL;
		dependencies[0].dstSubpass = 0;
		dependencies[0].srcStageMask = VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT;
		dependencies[0].srcAccessMask = 0;
		dependencies[0].dstStageMask =
			VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT |
			VK_PIPELINE_STAGE_EARLY_FRAGMENT_TESTS_BIT |
			VK_PIPELINE_STAGE_LATE_FRAGMENT_TESTS_BIT;
		dependencies[0].dstAccessMask =
			VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT |
			VK_ACCESS_COLOR_ATTACHMENT_READ_BIT |
			VK_ACCESS_DEPTH_STENCIL_ATTACHMENT_WRITE_BIT |
			VK_ACCESS_DEPTH_STENCIL_ATTACHMENT_READ_BIT;
		dependencies[0].dependencyFlags = VK_DEPENDENCY_BY_REGION_BIT;

		dependencies[1].srcSubpass = 0;
		dependencies[1].dstSubpass = VK_SUBPASS_EXTERNAL;
		dependencies[1].srcStageMask =
			VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT |
			VK_PIPELINE_STAGE_EARLY_FRAGMENT_TESTS_BIT |
			VK_PIPELINE_STAGE_LATE_FRAGMENT_TESTS_BIT;
		dependencies[1].srcAccessMask =
			VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT |
			VK_ACCESS_COLOR_ATTACHMENT_READ_BIT |
			VK_ACCESS_DEPTH_STENCIL_ATTACHMENT_WRITE_BIT |
			VK_ACCESS_DEPTH_STENCIL_ATTACHMENT_READ_BIT;
		dependencies[1].dstStageMask = VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT;
		dependencies[1].dstAccessMask = VK_ACCESS_MEMORY_READ_BIT;
		dependencies[1].dependencyFlags = VK_DEPENDENCY_BY_REGION_BIT;
	}
	else if (dstAttachmentsUsage[0] == ERenderResourceUsage::Present)
	{
		dependencies[0].srcSubpass = VK_SUBPASS_EXTERNAL;
		dependencies[0].dstSubpass = 0;
		dependencies[0].srcStageMask = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;
		dependencies[0].srcAccessMask = VK_ACCESS_NONE;
		dependencies[0].dstStageMask = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;
		dependencies[0].dstAccessMask = VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT;
	}*/

	VkRenderPassCreateInfo createInfo = {};
	createInfo.sType = VK_STRUCTURE_TYPE_RENDER_PASS_CREATE_INFO;
	createInfo.attachmentCount = attachments.size();
	createInfo.pAttachments = attachments.data();
	createInfo.subpassCount = 1;
	createInfo.pSubpasses = &subpass;
	createInfo.dependencyCount = dependencies.size();
	createInfo.pDependencies = dependencies.data();

	VkRenderPass renderPass = VK_NULL_HANDLE;

	VkResult result = vkCreateRenderPass(
		_logicalDevice.device,
		&createInfo,
		nullptr,
		&renderPass
	);

	if (result != VK_SUCCESS)
		Print("[Vulkan]: Error: failed to create render pass");

	std::vector<VkImageView> framebufferAttachments(colorAttachmentCount + depthAttachmentCount);
	for (usize i = 0; i < colorAttachmentCount; i++)
		framebufferAttachments[i] = (VkImageView)renderTarget.color_attachments[i].view;
	
	if (hasDepth == True)
		framebufferAttachments[colorAttachmentCount] = (VkImageView)renderTarget.depth_attachment.view;

	VkFramebufferCreateInfo framebufferCreateInfo = {};
	framebufferCreateInfo.sType = VK_STRUCTURE_TYPE_FRAMEBUFFER_CREATE_INFO;
	framebufferCreateInfo.renderPass = renderPass;
	framebufferCreateInfo.attachmentCount = (uint32_t)attachments.size();
	framebufferCreateInfo.pAttachments = framebufferAttachments.data();
	framebufferCreateInfo.width = renderTarget.color_attachments[0].size.GetX();
	framebufferCreateInfo.height = renderTarget.color_attachments[0].size.GetY();
	framebufferCreateInfo.layers = 1;

	VkFramebuffer framebuffer = VK_NULL_HANDLE;

	result = vkCreateFramebuffer(
		_logicalDevice.device,
		&framebufferCreateInfo,
		nullptr,
		&framebuffer
	);

	if (result != VK_SUCCESS)
		Print("[Vulkan]: Error: failed to create render pass framebuffer");

	return SGPURenderPassResource(
		(types::qword)renderPass,
		0,
		(types::qword)framebuffer,
		bClearRenderTarget,
		clearColor,
		clearDepth
	);
}

void triton::BGraphicsBackend2Vulkan::DestroyRenderPass(SGPURenderPassResource& renderPass)
{
	if (renderPass.IsValid() == True)
	{
		vkDestroyRenderPass(
			_logicalDevice.device,
			(VkRenderPass)renderPass.instance,
			nullptr
		);

		renderPass.Invalidate();
	}
}

triton::SGPUShaderResource triton::BGraphicsBackend2Vulkan::CreateShader(
	dword stageMask,
	const SShaderBytecodeFiles& bytecodeFiles
)
{
	if (!stageMask ||
		(!bytecodeFiles.vertexFilePath.has_value() &&
		!bytecodeFiles.pixelFilePath.has_value() &&
		!bytecodeFiles.tessellationControlFilePath.has_value() &&
		!bytecodeFiles.tessellationEvaluationFilePath.has_value() &&
		!bytecodeFiles.computeFilePath.has_value()))
		return SGPUShaderResource::Invalid();

	if (stageMask & (dword)EShaderStageBit::Vertex &&
		stageMask & (dword)EShaderStageBit::Pixel &&
		bytecodeFiles.vertexFilePath.has_value() &&
		bytecodeFiles.pixelFilePath.has_value())
	{
		SMemAlloc vertexShaderByteCode;
		const usize vertexShaderByteCodeSize = _context->GetSubsystem<XFileSystem>()->BinaryFileToArray(
			bytecodeFiles.vertexFilePath.value().string(),
			vertexShaderByteCode,
			0
		);

		SMemAlloc pixelShaderByteCode;
		const usize pixelShaderByteCodeSize = _context->GetSubsystem<XFileSystem>()->BinaryFileToArray(
			bytecodeFiles.pixelFilePath.value().string(),
			pixelShaderByteCode,
			0
		);

		VkResult result;

		VkShaderModuleCreateInfo vertexShaderModuleCreateInfo = {};
		vertexShaderModuleCreateInfo.sType = VK_STRUCTURE_TYPE_SHADER_MODULE_CREATE_INFO;
		vertexShaderModuleCreateInfo.codeSize = vertexShaderByteCodeSize;
		vertexShaderModuleCreateInfo.pCode = (uint32_t*)vertexShaderByteCode.mem;

		VkShaderModule vertexShaderModule = VK_NULL_HANDLE;

		result = vkCreateShaderModule(
			_logicalDevice.device,
			&vertexShaderModuleCreateInfo,
			nullptr,
			&vertexShaderModule
		);

		if (result != VK_SUCCESS)
			Print("[Vulkan]: Error: failed to create vertex shader");

		VkShaderModuleCreateInfo pixelShaderModuleCreateInfo = {};
		pixelShaderModuleCreateInfo.sType = VK_STRUCTURE_TYPE_SHADER_MODULE_CREATE_INFO;
		pixelShaderModuleCreateInfo.codeSize = pixelShaderByteCodeSize;
		pixelShaderModuleCreateInfo.pCode = (uint32_t*)pixelShaderByteCode.mem;

		VkShaderModule pixelShaderModule = VK_NULL_HANDLE;

		result = vkCreateShaderModule(
			_logicalDevice.device,
			&pixelShaderModuleCreateInfo,
			nullptr,
			&pixelShaderModule
		);

		if (result != VK_SUCCESS)
			Print("[Vulkan]: Error: failed to create pixel shader");

		_context->GetSubsystem<XFileSystem>()->ReleaseBinaryFileArray(vertexShaderByteCode);
		_context->GetSubsystem<XFileSystem>()->ReleaseBinaryFileArray(pixelShaderByteCode);

		std::vector<qword> modules(2);
		modules[0] = (qword)vertexShaderModule;
		modules[1] = (qword)pixelShaderModule;

		return SGPUShaderResource(
			0,
			0,
			stageMask,
			modules
		);
	}
	else if (stageMask & (dword)EShaderStageBit::Compute && bytecodeFiles.computeFilePath.has_value())
	{
		SMemAlloc compute_shader_bytecode;

		const usize compute_shader_bytecode_size = _context->GetSubsystem<XFileSystem>()->BinaryFileToArray(
			bytecodeFiles.computeFilePath.value().string(),
			compute_shader_bytecode,
			0
		);

		VkResult result;

		VkShaderModuleCreateInfo compute_shader_module_create_info = {};
		compute_shader_module_create_info.sType = VK_STRUCTURE_TYPE_SHADER_MODULE_CREATE_INFO;
		compute_shader_module_create_info.codeSize = compute_shader_bytecode_size;
		compute_shader_module_create_info.pCode = (uint32_t*)compute_shader_bytecode.mem;

		VkShaderModule compute_shader_module = VK_NULL_HANDLE;

		result = vkCreateShaderModule(
			_logicalDevice.device,
			&compute_shader_module_create_info,
			nullptr,
			&compute_shader_module
		);

		if (result != VK_SUCCESS)
			Print("[Vulkan]: Error: failed to create compute shader");

		_context->GetSubsystem<XFileSystem>()->ReleaseBinaryFileArray(compute_shader_bytecode);

		std::vector<qword> modules(1);
		modules[0] = (qword)compute_shader_module;

		return SGPUShaderResource(
			0,
			0,
			stageMask,
			modules
		);
	}

	return SGPUShaderResource::Invalid();
}

void triton::BGraphicsBackend2Vulkan::DestroyShader(SGPUShaderResource& shader)
{
	if (shader.IsValid())
	{
		for (auto& module : shader.modules)
			vkDestroyShaderModule(
				_logicalDevice.device,
				(VkShaderModule)module,
				nullptr
			);

		shader.Invalidate();
	}
}

void triton::BGraphicsBackend2Vulkan::Dispatch(
	const SGPUPipelineResource& pipeline,
	const std::vector<SGPUBindingGroupResource>& binding_groups,
	const cVector3& work_group_count
)
{
	vkCmdBindPipeline(
		_frameCommandBuffers[_currentFrame],
		VK_PIPELINE_BIND_POINT_COMPUTE,
		(VkPipeline)pipeline.instance
	);

	std::vector<VkDescriptorSet> native_descriptor_sets;

	for (usize i = 0; i < binding_groups.size(); ++i)
		native_descriptor_sets.push_back((VkDescriptorSet)binding_groups.at(i).instance);

	const usize first_set_index = binding_groups.at(0).index;

	vkCmdBindDescriptorSets(
		_frameCommandBuffers[_currentFrame],
		VK_PIPELINE_BIND_POINT_COMPUTE,
		(VkPipelineLayout)pipeline.layout,
		0,
		native_descriptor_sets.size(),
		native_descriptor_sets.data(),
		0,
		nullptr
	);

	vkCmdDispatch(
		_frameCommandBuffers[_currentFrame],
		work_group_count.GetX(),
		work_group_count.GetY(),
		work_group_count.GetZ()
	);
}

void triton::BGraphicsBackend2Vulkan::ResetCommandBuffer(
	ERenderCommandBufferType command_buffer,
	ERenderCommandBufferResetOption reset_option
)
{
	VkCommandBuffer cmd_buff = VK_NULL_HANDLE;

	if (command_buffer == ERenderCommandBufferType::Frame)
		cmd_buff = _frameCommandBuffers[_currentFrame];
	else if (command_buffer == ERenderCommandBufferType::Staging)
		cmd_buff = _stagingCommandBuffers[_currentFrame];

	if (reset_option == ERenderCommandBufferResetOption::Clear)
	{
		vkResetCommandBuffer(
			cmd_buff,
			0
		);
	}
	else if (reset_option == ERenderCommandBufferResetOption::Begin)
	{
		VkCommandBufferBeginInfo beginInfo = {};
		beginInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;

		vkBeginCommandBuffer(
			cmd_buff,
			&beginInfo
		);
	}
	else if (reset_option == ERenderCommandBufferResetOption::End)
	{
		vkEndCommandBuffer(cmd_buff);
	}
}

void triton::BGraphicsBackend2Vulkan::AddCommandToBuffer(
	ENativeRenderCommand command,
	const void* commandArgA,
	const void* commandArgB
)
{
	if (command == ENativeRenderCommand::Unknown)
		return;

	if (command == ENativeRenderCommand::BeginRenderPass)
	{
		SGPURenderPassResource& renderPass = *(SGPURenderPassResource*)commandArgA;
		SGPURenderTargetResource& renderTarget = *(SGPURenderTargetResource*)commandArgB;

		VkRenderPassBeginInfo renderPassBeginInfo = {};
		renderPassBeginInfo.sType = VK_STRUCTURE_TYPE_RENDER_PASS_BEGIN_INFO;
		renderPassBeginInfo.renderPass = (VkRenderPass)renderPass.instance;
		renderPassBeginInfo.framebuffer = (VkFramebuffer)renderPass.framebuffer;
		renderPassBeginInfo.renderArea.offset.x = 0;
		renderPassBeginInfo.renderArea.offset.y = 0;
		renderPassBeginInfo.renderArea.extent.width = (uint32_t)renderTarget.color_attachments[0].size.GetX();
		renderPassBeginInfo.renderArea.extent.height = (uint32_t)renderTarget.color_attachments[0].size.GetY();
		
		VkClearValue clearValues[2] = {};
		clearValues[0].color = {
			renderPass.clear_color.GetX(),
			renderPass.clear_color.GetY(),
			renderPass.clear_color.GetZ(),
			renderPass.clear_color.GetW()
		};

		clearValues[1].depthStencil = {
			renderPass.clear_depth,
			0
		};

		if (True)
		{
			if (renderTarget.depth_attachment.IsValid() == True)
				renderPassBeginInfo.clearValueCount = 2;
			else
				renderPassBeginInfo.clearValueCount = 1;
			renderPassBeginInfo.pClearValues = &clearValues[0];
		}

		vkCmdBeginRenderPass(
			_frameCommandBuffers[_currentFrame],
			&renderPassBeginInfo,
			VK_SUBPASS_CONTENTS_INLINE
		);
	}
	else if (command == ENativeRenderCommand::BindPipeline)
	{
		vkCmdBindPipeline(
			_frameCommandBuffers[_currentFrame],
			PipelineBindPointToNative(((SGPUPipelineResource*)commandArgA)->bind_point),
			(VkPipeline)((SGPUPipelineResource*)commandArgA)->instance
		);
	}
	else if (command == ENativeRenderCommand::Draw)
	{
		const SNativeCommandDrawInfo& drawInfo = *(SNativeCommandDrawInfo*)commandArgA;
		SGPUPipelineResource& pipeline = *(SGPUPipelineResource*)commandArgB;

		vkCmdPushConstants(
			_frameCommandBuffers[_currentFrame],
			(VkPipelineLayout)pipeline.layout,
			VK_SHADER_STAGE_VERTEX_BIT,
			offsetof(SRenderPassPushConstantsLayout, instanceMotionType),
			4,
			&drawInfo.instanceMotionType
		);

		vkCmdDrawIndexed(
			_frameCommandBuffers[_currentFrame],
			drawInfo.indexCount,
			drawInfo.instanceCount,
			drawInfo.firstIndex,
			drawInfo.baseVertex,
			drawInfo.firstInstance
		);
	}
	else if (command == ENativeRenderCommand::EndRenderPass)
	{
		vkCmdEndRenderPass(_frameCommandBuffers[_currentFrame]);
	}
	else if (command == ENativeRenderCommand::SetScissor)
	{
		SViewport viewport = *(SViewport*)commandArgA;

		VkRect2D nativeScissor;
		nativeScissor.extent.width = viewport.rect.GetZ();
		nativeScissor.extent.height = viewport.rect.GetW();

		vkCmdSetScissor(_frameCommandBuffers[_currentFrame], 0, 1, &nativeScissor);
	}
	else if (command == ENativeRenderCommand::SetViewport)
	{
		SViewport viewport = *(SViewport*)commandArgA;

		VkViewport nativeViewport = ViewportToNative(viewport);

		vkCmdSetViewport(_frameCommandBuffers[_currentFrame], 0, 1, &nativeViewport);
	}
	else if (command == ENativeRenderCommand::BindVertexBuffer)
	{
		const SGPUBufferResource& vertexBuffer = *(SGPUBufferResource*)commandArgA;

		VkBuffer nativeVertexBuffer = (VkBuffer)vertexBuffer.instance;
		VkDeviceSize offset = 0;

		vkCmdBindVertexBuffers(
			_frameCommandBuffers[_currentFrame],
			0,
			1,
			&nativeVertexBuffer,
			&offset
		);
	}
	else if (command == ENativeRenderCommand::BindIndexBuffer)
	{
		SGPUBufferResource& indexBuffer = *(SGPUBufferResource*)commandArgA;

		VkBuffer nativeIndexBuffer = (VkBuffer)indexBuffer.instance;

		vkCmdBindIndexBuffer(
			_frameCommandBuffers[_currentFrame],
			nativeIndexBuffer,
			0,
			VK_INDEX_TYPE_UINT32
		);
	}
	else if (command == ENativeRenderCommand::PushConstants)
	{
		SRenderPassPushConstantsLayout& pushConstants = *(SRenderPassPushConstantsLayout*)commandArgA;
		SGPUPipelineResource& pipeline = *(SGPUPipelineResource*)commandArgB;

		vkCmdPushConstants(
			_frameCommandBuffers[_currentFrame],
			(VkPipelineLayout)pipeline.layout,
			VK_SHADER_STAGE_VERTEX_BIT,
			0,
			sizeof(SRenderPassPushConstantsLayout),
			&pushConstants
		);
	}
	else if (command == ENativeRenderCommand::BindDescriptorSets)
	{
		std::vector<SGPUBindingGroupResource>& bindingGroups = *(std::vector<SGPUBindingGroupResource>*)commandArgA;
		
		if (!bindingGroups.size())
			return;
		
		SGPUPipelineResource& pipeline = *(SGPUPipelineResource*)commandArgB;

		std::vector<VkDescriptorSet> nativeDescriptorSets;
		
		for (usize i = 0; i < bindingGroups.size(); ++i)
			nativeDescriptorSets.push_back((VkDescriptorSet)bindingGroups[i].instance);

		const usize firstSetIndex = bindingGroups[0].index;

		vkCmdBindDescriptorSets(
			_frameCommandBuffers[_currentFrame],
			VK_PIPELINE_BIND_POINT_GRAPHICS,
			(VkPipelineLayout)pipeline.layout,
			firstSetIndex,
			nativeDescriptorSets.size(),
			nativeDescriptorSets.data(),
			0,
			nullptr
		);
	}
}

void triton::BGraphicsBackend2Vulkan::BeginFrame()
{
	vkWaitForFences(
		_logicalDevice.device,
		1,
		&_swapchainFences[_currentFrame].fence,
		VK_TRUE,
		UINT64_MAX
	);

	ResetCommandBuffer(ERenderCommandBufferType::Frame, ERenderCommandBufferResetOption::Clear);
	ResetCommandBuffer(ERenderCommandBufferType::Frame, ERenderCommandBufferResetOption::Begin);
}

void triton::BGraphicsBackend2Vulkan::EndFrame()
{
	VkResult result;

	uint32_t imageIndex = 0;

	result = vkAcquireNextImageKHR(
		_logicalDevice.device,
		_swapchain.swapchain,
		UINT64_MAX,
		_swapchainImageAvailableSemaphores[_currentFrame].semaphore,
		VK_NULL_HANDLE,
		&imageIndex
	);

	if (result != VK_SUCCESS)
		Print("[Vulkan]: Error: failed to acquire swapchain image");

	vkResetFences(
		_logicalDevice.device,
		1,
		&_swapchainFences[_currentFrame].fence
	);

	VkClearValue clearValue = {};
	clearValue.color = {
		_swapchainRenderPasses[imageIndex].clear_color.GetX(),
		_swapchainRenderPasses[imageIndex].clear_color.GetY(),
		_swapchainRenderPasses[imageIndex].clear_color.GetZ(),
		_swapchainRenderPasses[imageIndex].clear_color.GetW()
	};

	VkRenderPassBeginInfo renderPassBeginInfo = {};
	renderPassBeginInfo.sType = VK_STRUCTURE_TYPE_RENDER_PASS_BEGIN_INFO;
	renderPassBeginInfo.renderPass = (VkRenderPass)_swapchainRenderPasses[imageIndex].instance;
	renderPassBeginInfo.framebuffer = (VkFramebuffer)_swapchainRenderPasses[imageIndex].framebuffer;
	renderPassBeginInfo.renderArea.offset = { 0, 0 };
	renderPassBeginInfo.renderArea.extent.width = _swapchain.size.GetX();
	renderPassBeginInfo.renderArea.extent.height = _swapchain.size.GetY();
	renderPassBeginInfo.clearValueCount = 1;
	renderPassBeginInfo.pClearValues = &clearValue;

	vkCmdBeginRenderPass(
		_frameCommandBuffers[_currentFrame],
		&renderPassBeginInfo,
		VK_SUBPASS_CONTENTS_INLINE
	);

	vkCmdBindPipeline(
		_frameCommandBuffers[_currentFrame],
		VK_PIPELINE_BIND_POINT_GRAPHICS,
		(VkPipeline)_swapchainPipelines[imageIndex].instance
	);

	VkDescriptorSet nativeDescriptorSet =
		(VkDescriptorSet)_swapchainBindingGroups[imageIndex].instance;

	vkCmdBindDescriptorSets(
		_frameCommandBuffers[_currentFrame],
		VK_PIPELINE_BIND_POINT_GRAPHICS,
		(VkPipelineLayout)_swapchainPipelines[imageIndex].layout,
		0,
		1,
		&nativeDescriptorSet,
		0,
		nullptr
	);

	vkCmdDraw(
		_frameCommandBuffers[_currentFrame],
		4,
		1,
		0,
		0
	);

	vkCmdEndRenderPass(
		_frameCommandBuffers[_currentFrame]
	);

	ResetCommandBuffer(ERenderCommandBufferType::Frame, ERenderCommandBufferResetOption::End);

	VkPipelineStageFlags waitStage = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;

	VkSubmitInfo submitInfo = {};
	submitInfo.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO;
	submitInfo.waitSemaphoreCount = 1;
	submitInfo.pWaitSemaphores = &_swapchainImageAvailableSemaphores[_currentFrame].semaphore;
	submitInfo.pWaitDstStageMask = &waitStage;
	submitInfo.commandBufferCount = 1;
	submitInfo.pCommandBuffers = &_frameCommandBuffers[_currentFrame];
	submitInfo.signalSemaphoreCount = 1;
	submitInfo.pSignalSemaphores = &_swapchainRenderFinishedSemaphores[imageIndex].semaphore;

	result = vkQueueSubmit(
		_graphicsTransferPresentQueue.queue,
		1,
		&submitInfo,
		_swapchainFences[_currentFrame].fence
	);

	if (result != VK_SUCCESS)
		Print("[Vulkan]: Error: failed to submit commands to the graphics queue");

	VkPresentInfoKHR presentInfo = {};
	presentInfo.sType = VK_STRUCTURE_TYPE_PRESENT_INFO_KHR;
	presentInfo.waitSemaphoreCount = 1;
	presentInfo.pWaitSemaphores = &_swapchainRenderFinishedSemaphores[imageIndex].semaphore;
	presentInfo.swapchainCount = 1;
	presentInfo.pSwapchains = &_swapchain.swapchain;
	presentInfo.pImageIndices = &imageIndex;

	result = vkQueuePresentKHR(
		_graphicsTransferPresentQueue.queue,
		&presentInfo
	);

	if (result != VK_SUCCESS)
		Print("[Vulkan]: Error: failed to present frame");

	_currentFrame = (_currentFrame + 1) % _framesInFlight;
}

void triton::BGraphicsBackend2Vulkan::ExecuteImageBarrier(
	ERenderCommandBufferType command_buffer,
	const SGPUTextureResource& texture,
	ERenderPipelineStageBit src_stage_mask,
	ERenderPipelineStageBit dst_stage_mask,
	ERenderResourceAccessBit src_access_mask,
	ERenderResourceAccessBit dst_access_mask,
	ERenderImageAspectBit aspect_mask,
	ERenderImageLayout src_layout,
	ERenderImageLayout dst_layout
)
{
	VkPipelineStageFlags native_src_stage_mask = PipelineStageToNative(src_stage_mask);
	VkPipelineStageFlags native_dst_stage_mask = PipelineStageToNative(dst_stage_mask);
	VkAccessFlags native_src_access_mask = AccessToNative(src_access_mask);
	VkAccessFlags native_dst_access_mask = AccessToNative(dst_access_mask);
	VkImageAspectFlags native_aspect_mask = ImageAspectToNative(aspect_mask);
	VkImageLayout native_src_layout = ImageLayoutToNative(src_layout);
	VkImageLayout native_dst_layout = ImageLayoutToNative(dst_layout);

	VkImageSubresourceRange imageSubresource = {};
	imageSubresource.aspectMask = native_aspect_mask;
	imageSubresource.baseArrayLayer = 0;
	imageSubresource.baseMipLevel = 0;
	imageSubresource.layerCount = 1;
	imageSubresource.levelCount = 1;

	VkImageMemoryBarrier barrier = {};
	barrier.sType = VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER;
	barrier.image = (VkImage)texture.instance;
	barrier.subresourceRange = imageSubresource;
	barrier.srcAccessMask = native_src_access_mask;
	barrier.dstAccessMask = native_dst_access_mask;
	barrier.srcQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
	barrier.dstQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
	barrier.oldLayout = native_src_layout;
	barrier.newLayout = native_dst_layout;

	VkCommandBuffer cmd_buff = VK_NULL_HANDLE;

	if (command_buffer == ERenderCommandBufferType::Frame)
		cmd_buff = _frameCommandBuffers[_currentFrame];
	else if (command_buffer == ERenderCommandBufferType::Staging)
		cmd_buff = _stagingCommandBuffers[_currentFrame];

	vkCmdPipelineBarrier(
		cmd_buff,
		native_src_stage_mask,
		native_dst_stage_mask,
		0,
		0,
		nullptr,
		0,
		nullptr,
		1,
		&barrier
	);
}

void triton::BGraphicsBackend2Vulkan::ExecuteBufferBarrier(
	ERenderCommandBufferType command_buffer,
	const SGPUBufferResource& buffer,
	ERenderPipelineStageBit src_stage_mask,
	ERenderPipelineStageBit dst_stage_mask,
	ERenderResourceAccessBit src_access_mask,
	ERenderResourceAccessBit dst_access_mask
)
{
	VkPipelineStageFlags native_src_stage_mask = PipelineStageToNative(src_stage_mask);
	VkPipelineStageFlags native_dst_stage_mask = PipelineStageToNative(dst_stage_mask);
	VkAccessFlags native_src_access_mask = AccessToNative(src_access_mask);
	VkAccessFlags native_dst_access_mask = AccessToNative(dst_access_mask);

	VkBufferMemoryBarrier barrier = {};
	barrier.sType = VK_STRUCTURE_TYPE_BUFFER_MEMORY_BARRIER;
	barrier.buffer = (VkBuffer)buffer.instance;
	barrier.offset = 0;
	barrier.size = buffer.byte_size;
	barrier.srcAccessMask = native_src_access_mask;
	barrier.dstAccessMask = native_dst_access_mask;
	barrier.srcQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
	barrier.dstQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;

	VkCommandBuffer cmd_buff = VK_NULL_HANDLE;

	if (command_buffer == ERenderCommandBufferType::Frame)
		cmd_buff = _frameCommandBuffers[_currentFrame];
	else if (command_buffer == ERenderCommandBufferType::Staging)
		cmd_buff = _stagingCommandBuffers[_currentFrame];

	vkCmdPipelineBarrier(
		cmd_buff,
		native_src_stage_mask,
		native_dst_stage_mask,
		0,
		0,
		nullptr,
		1,
		&barrier,
		0,
		nullptr
	);
}

void triton::BGraphicsBackend2Vulkan::CopyBufferToImage(
	ERenderCommandBufferType command_buffer,
	const SGPUBufferResource& buffer,
	const SGPUTextureResource& texture,
	ERenderImageAspectBit aspect_mask,
	const cVector3& offset,
	const cVector3& size
)
{
	VkAccessFlags native_aspect_mask = 0;

	if ((dword)aspect_mask & (dword)ERenderImageAspectBit::Color)
		native_aspect_mask |= VK_IMAGE_ASPECT_COLOR_BIT;
	if ((dword)aspect_mask & (dword)ERenderImageAspectBit::Depth)
		native_aspect_mask |= VK_IMAGE_ASPECT_DEPTH_BIT;

	VkImageSubresourceLayers imageSubresourceLayers = {};
	imageSubresourceLayers.aspectMask = native_aspect_mask;
	imageSubresourceLayers.baseArrayLayer = 0;
	imageSubresourceLayers.layerCount = 1;
	imageSubresourceLayers.mipLevel = 0;

	VkBufferImageCopy region = {};
	region.bufferOffset = 0;
	region.bufferRowLength = 0;
	region.bufferImageHeight = 0;
	region.imageOffset = {
		(int32_t)offset.GetX(), (int32_t)offset.GetY(), (int32_t)offset.GetZ()
	};
	region.imageExtent = {
		(uint32_t)size.GetX(), (uint32_t)size.GetY(), (uint32_t)size.GetZ()
	};
	region.imageSubresource = imageSubresourceLayers;

	VkCommandBuffer cmd_buff = VK_NULL_HANDLE;

	if (command_buffer == ERenderCommandBufferType::Frame)
		cmd_buff = _frameCommandBuffers[_currentFrame];
	else if (command_buffer == ERenderCommandBufferType::Staging)
		cmd_buff = _stagingCommandBuffers[_currentFrame];

	vkCmdCopyBufferToImage(
		cmd_buff,
		(VkBuffer)buffer.instance,
		(VkImage)texture.instance,
		VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL,
		1,
		&region
	);
}

void triton::BGraphicsBackend2Vulkan::CopyBufferToBuffer(
	ERenderCommandBufferType command_buffer,
	const SGPUBufferResource& src_buffer,
	const SGPUBufferResource& dst_buffer,
	usize src_offset,
	usize dst_offset,
	usize size
)
{
	VkBufferCopy region = {};
	region.srcOffset = src_offset;
	region.dstOffset = dst_offset;
	region.size = size;

	VkCommandBuffer cmd_buff = VK_NULL_HANDLE;

	if (command_buffer == ERenderCommandBufferType::Frame)
		cmd_buff = _frameCommandBuffers[_currentFrame];
	else if (command_buffer == ERenderCommandBufferType::Staging)
		cmd_buff = _stagingCommandBuffers[_currentFrame];

	vkCmdCopyBuffer(
		cmd_buff,
		(VkBuffer)src_buffer.instance,
		(VkBuffer)dst_buffer.instance,
		1,
		&region
	);
}

void triton::BGraphicsBackend2Vulkan::SubmitCommands(ERenderCommandBufferType command_buffer)
{
	VkCommandBuffer cmd_buff = VK_NULL_HANDLE;
	VkFence fence = VK_NULL_HANDLE;

	if (command_buffer == ERenderCommandBufferType::Frame)
	{
		cmd_buff = _frameCommandBuffers[_currentFrame];
		fence = _swapchainFences[_currentFrame].fence;
	}
	else if (command_buffer == ERenderCommandBufferType::Staging)
	{
		cmd_buff = _stagingCommandBuffers[_currentFrame];
		fence = _stagingFences[_currentFrame].fence;
	}

	VkPipelineStageFlags waitStage = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;

	VkSubmitInfo submitInfo = {};
	submitInfo.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO;
	submitInfo.waitSemaphoreCount = 0;
	submitInfo.pWaitSemaphores = nullptr;
	submitInfo.pWaitDstStageMask = nullptr;
	submitInfo.commandBufferCount = 1;
	submitInfo.pCommandBuffers = &cmd_buff;
	submitInfo.signalSemaphoreCount = 0;
	submitInfo.pSignalSemaphores = nullptr;

	VkResult result = vkQueueSubmit(
		_graphicsTransferPresentQueue.queue,
		1,
		&submitInfo,
		fence
	);

	if (result != VK_SUCCESS)
		Print("[Vulkan]: Error: failed to submit commands to the queue");
}

void triton::BGraphicsBackend2Vulkan::WaitForFence(ERenderCommandBufferType command_buffer)
{
	VkFence fence = VK_NULL_HANDLE;

	if (command_buffer == ERenderCommandBufferType::Frame)
		fence = _swapchainFences[_currentFrame].fence;
	else if (command_buffer == ERenderCommandBufferType::Staging)
		fence = _stagingFences[_currentFrame].fence;

	vkWaitForFences(
		_logicalDevice.device,
		1,
		&fence,
		VK_TRUE,
		UINT64_MAX
	);
}

void triton::BGraphicsBackend2Vulkan::ResetFence(ERenderCommandBufferType command_buffer)
{
	VkFence fence = VK_NULL_HANDLE;

	if (command_buffer == ERenderCommandBufferType::Frame)
		fence = _swapchainFences[_currentFrame].fence;
	else if (command_buffer == ERenderCommandBufferType::Staging)
		fence = _stagingFences[_currentFrame].fence;

	vkResetFences(
		_logicalDevice.device,
		1,
		&fence
	);
}

void triton::BGraphicsBackend2Vulkan::CreateInstance(
	boolean bEnableDebugging,
	const std::vector<const char*> extensions
)
{
	if (bEnableDebugging == True &&
		!CheckValidationLayerSupport())
	{
		Print("Error: Vulkan validation layer is not available");
		return;
	}

	std::vector<const char*> validExtensions;
	for (usize i = 0; i < extensions.size(); i++)
		if (IsZeroTerminated(extensions[i], 1024))
			validExtensions.push_back(extensions[i]);

	if (bEnableDebugging == True)
		validExtensions.push_back(VK_EXT_DEBUG_UTILS_EXTENSION_NAME);

	VkApplicationInfo applicationInfo = {};
	applicationInfo.sType = VK_STRUCTURE_TYPE_APPLICATION_INFO;
	applicationInfo.pApplicationName = "TritonEngineApplication";
	applicationInfo.applicationVersion = VK_MAKE_VERSION(1, 0, 0);
	applicationInfo.pEngineName = "TritonEngine";
	applicationInfo.engineVersion = VK_MAKE_VERSION(1, 0, 0);
	applicationInfo.apiVersion = VK_MAKE_VERSION(1, 0, 66);

	std::vector<VkValidationFeatureEnableEXT> featuresEnabled;
	if (bEnableDebugging == True)
	{
		featuresEnabled.resize(3);

		//VK_VALIDATION_FEATURE_ENABLE_GPU_ASSISTED_EXT,
		//VK_VALIDATION_FEATURE_ENABLE_GPU_ASSISTED_RESERVE_BINDING_SLOT_EXT,
		featuresEnabled[0] = VK_VALIDATION_FEATURE_ENABLE_BEST_PRACTICES_EXT;
		featuresEnabled[1] = VK_VALIDATION_FEATURE_ENABLE_SYNCHRONIZATION_VALIDATION_EXT;
		featuresEnabled[2] = VK_VALIDATION_FEATURE_ENABLE_DEBUG_PRINTF_EXT;
	};

	VkValidationFeaturesEXT validationFeatures = {};
	validationFeatures.sType = VK_STRUCTURE_TYPE_VALIDATION_FEATURES_EXT;
	validationFeatures.enabledValidationFeatureCount = featuresEnabled.size();
	validationFeatures.pEnabledValidationFeatures = featuresEnabled.data();

	VkInstanceCreateInfo createInfo = {};
	createInfo.sType = VK_STRUCTURE_TYPE_INSTANCE_CREATE_INFO;
	createInfo.pApplicationInfo = &applicationInfo;
	createInfo.enabledExtensionCount = (uint32_t)(validExtensions.size());
	createInfo.ppEnabledExtensionNames = validExtensions.data();

	if (bEnableDebugging == True)
	{
		createInfo.enabledLayerCount = 1;
		createInfo.ppEnabledLayerNames = &kValidationLayer;
		createInfo.pNext = &validationFeatures;
	}

	VkResult result = vkCreateInstance(
		&createInfo,
		nullptr,
		&_instance.instance
	);

	if (result != VK_SUCCESS)
	{
		Print("Error: failed to create Vulkan instance");
		return;
	}

	if (bEnableDebugging == True)
		CreateDebugMessenger();
}

void triton::BGraphicsBackend2Vulkan::DestroyInstance()
{
	DestroyDebugMessenger();
	
	if (_instance.instance != VK_NULL_HANDLE)
	{
		vkDestroyInstance(
			_instance.instance,
			nullptr
		);
	}
}

void triton::BGraphicsBackend2Vulkan::CreateDebugMessenger()
{
	VkDebugUtilsMessengerCreateInfoEXT debugCreateInfo = {};
	debugCreateInfo.sType = VK_STRUCTURE_TYPE_DEBUG_UTILS_MESSENGER_CREATE_INFO_EXT;
	debugCreateInfo.messageSeverity =
		VK_DEBUG_UTILS_MESSAGE_SEVERITY_VERBOSE_BIT_EXT |
		VK_DEBUG_UTILS_MESSAGE_SEVERITY_INFO_BIT_EXT |
		VK_DEBUG_UTILS_MESSAGE_SEVERITY_WARNING_BIT_EXT |
		VK_DEBUG_UTILS_MESSAGE_SEVERITY_ERROR_BIT_EXT;
	debugCreateInfo.messageType =
		VK_DEBUG_UTILS_MESSAGE_TYPE_GENERAL_BIT_EXT |
		VK_DEBUG_UTILS_MESSAGE_TYPE_VALIDATION_BIT_EXT |
		VK_DEBUG_UTILS_MESSAGE_TYPE_PERFORMANCE_BIT_EXT;
	debugCreateInfo.pfnUserCallback =
		DebugCallback;

	PFN_vkCreateDebugUtilsMessengerEXT vkCreateDebugUtilsMessenger = VK_NULL_HANDLE;
	vkCreateDebugUtilsMessenger = (PFN_vkCreateDebugUtilsMessengerEXT)vkGetInstanceProcAddr(
		_instance.instance,
		"vkCreateDebugUtilsMessengerEXT"
	);

	VkResult result = vkCreateDebugUtilsMessenger(
		_instance.instance,
		&debugCreateInfo,
		nullptr,
		&_instance.debugMessenger
	);

	if (result != VK_SUCCESS)
		Print("Error: failed to create Vulkan debug messenger");
}

void triton::BGraphicsBackend2Vulkan::DestroyDebugMessenger()
{
	if (_instance.debugMessenger != VK_NULL_HANDLE)
	{
		PFN_vkDestroyDebugUtilsMessengerEXT vkDestroyDebugUtilsMessenger = VK_NULL_HANDLE;
		vkDestroyDebugUtilsMessenger = (PFN_vkDestroyDebugUtilsMessengerEXT)vkGetInstanceProcAddr(
			_instance.instance,
			"vkDestroyDebugUtilsMessengerEXT"
		);

		vkDestroyDebugUtilsMessenger(
			_instance.instance,
			_instance.debugMessenger,
			nullptr
		);
	}
}

void triton::BGraphicsBackend2Vulkan::CreateSurface(SWindowResource& window)
{
	void* surfaceRaw = _context->GetBackend<IInputBackend>()->CreateBackendWindowVulkanSurface(
		window,
		(void*)&_instance.instance
	);

	if (!surfaceRaw)
	{
		Print("Error: failed to create Vulkan surface");
		return;
	}

	_surface.surface = *((VkSurfaceKHR*)surfaceRaw);
}

void triton::BGraphicsBackend2Vulkan::DestroySurface()
{
	_context->GetBackend<IInputBackend>()->DestroyBackendWindowVulkanSurface(
		(void*)&_instance.instance,
		(void*)&_surface.surface
	);
}

void triton::BGraphicsBackend2Vulkan::PickPhysicalDevice(EGraphicsAdapterType deviceType)
{
	VkPhysicalDevice physicalDevice = VK_NULL_HANDLE;

	uint32_t deviceCount = 0;
	VkResult result = vkEnumeratePhysicalDevices(
		_instance.instance,
		&deviceCount,
		nullptr
	);

	if (result != VK_SUCCESS || deviceCount == 0)
	{
		Print("Error: no Vulkan physical devices found");
		return;
	}

	std::vector<VkPhysicalDevice> devices(deviceCount);
	result = vkEnumeratePhysicalDevices(
		_instance.instance,
		&deviceCount,
		devices.data()
	);

	if (result != VK_SUCCESS)
	{
		Print("Error: failed to enumerate Vulkan physical devices");
		return;
	}

	for (VkPhysicalDevice device : devices)
	{
		VkPhysicalDeviceProperties properties = {};
		vkGetPhysicalDeviceProperties(
			device,
			&properties
		);

		if (deviceType == EGraphicsAdapterType::Discrete &&
			properties.deviceType ==
			VK_PHYSICAL_DEVICE_TYPE_DISCRETE_GPU)
		{
			physicalDevice = device;
			break;
		}
		else if (deviceType == EGraphicsAdapterType::Integrated &&
			properties.deviceType ==
			VK_PHYSICAL_DEVICE_TYPE_INTEGRATED_GPU)
		{
			physicalDevice = device;
			break;
		}
	}

	if (physicalDevice == VK_NULL_HANDLE)
		physicalDevice = devices[0];

	VkPhysicalDeviceProperties properties = {};
	vkGetPhysicalDeviceProperties(
		physicalDevice,
		&properties
	);

	Print("[Vulkan] Picked device: " + std::string(properties.deviceName));

	uint32_t version = properties.apiVersion;
	uint32_t major = VK_VERSION_MAJOR(version);
	uint32_t minor = VK_VERSION_MINOR(version);
	uint32_t patch = VK_VERSION_PATCH(version);
	Print(
		"[Vulkan] API version: " +
		std::to_string(major) + "." +
		std::to_string(minor) + "." +
		std::to_string(patch)
	);

	EGraphicsAdapterType finalDeviceType = EGraphicsAdapterType::Unknown;

	if (properties.deviceType == VK_PHYSICAL_DEVICE_TYPE_DISCRETE_GPU)
		finalDeviceType = EGraphicsAdapterType::Discrete;
	else if (properties.deviceType == VK_PHYSICAL_DEVICE_TYPE_INTEGRATED_GPU)
		finalDeviceType = EGraphicsAdapterType::Integrated;
	
	VkPhysicalDeviceFeatures features = {};
	vkGetPhysicalDeviceFeatures(
		physicalDevice,
		&features
	);

	VkPhysicalDeviceFeatures2 features2 = {};
	features2.sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_FEATURES_2;
	VkPhysicalDeviceVulkan13Features featuresVulkan13 = {};
	featuresVulkan13.sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_VULKAN_1_3_FEATURES;
	features2.pNext = &featuresVulkan13;
	vkGetPhysicalDeviceFeatures2(
		physicalDevice,
		&features2
	);

	uint32_t queueCount = 0;
	vkGetPhysicalDeviceQueueFamilyProperties(
		physicalDevice,
		&queueCount,
		nullptr
	);
	std::vector<VkQueueFamilyProperties> queueFamilyProperties(queueCount);
	vkGetPhysicalDeviceQueueFamilyProperties(
		physicalDevice,
		&queueCount,
		queueFamilyProperties.data()
	);

	VkSurfaceCapabilitiesKHR surfaceCapabilities = {};
	vkGetPhysicalDeviceSurfaceCapabilitiesKHR(
		physicalDevice,
		_surface.surface,
		&surfaceCapabilities
	);

	_physicalDevice = SPhysicalDevice(
		finalDeviceType,
		physicalDevice,
		properties,
		features,
		features2,
		featuresVulkan13,
		queueFamilyProperties,
		surfaceCapabilities
	);

	CheckPhysicalDeviceProperties();
	CheckPhysicalDeviceFeatures();
	CheckPhysicalDeviceFeaturesVulkan13();
	CheckPhysicalDeviceFormats();
	FindQueueFamilies();
}

void triton::BGraphicsBackend2Vulkan::CheckPhysicalDeviceProperties()
{
	Print(
		"[Vulkan]: Info: max uniform buffer range: " +
		std::to_string(_physicalDevice.properties.limits.maxUniformBufferRange)
	);
	Print(
		"[Vulkan]: Info: max storage buffer range: " +
		std::to_string(_physicalDevice.properties.limits.maxStorageBufferRange)
	);
	Print(
		"[Vulkan]: Info: max memory allocation count: " +
		std::to_string(_physicalDevice.properties.limits.maxMemoryAllocationCount)
	);
	Print(
		"[Vulkan]: Info: max bound descriptor sets: " +
		std::to_string(_physicalDevice.properties.limits.maxBoundDescriptorSets)
	);
	Print(
		"[Vulkan]: Info: max color attachments: " +
		std::to_string(_physicalDevice.properties.limits.maxColorAttachments)
	);
	Print(
		"[Vulkan]: Info: max push constants size: " + 
		std::to_string(_physicalDevice.properties.limits.maxPushConstantsSize)
	);
	Print(
		"[Vulkan]: Info: max image dimension 2D: " +
		std::to_string(_physicalDevice.properties.limits.maxImageDimension2D)
	);
	Print(
		"[Vulkan]: Info: max image dimension 3D: " +
		std::to_string(_physicalDevice.properties.limits.maxImageDimension3D)
	);
}

void triton::BGraphicsBackend2Vulkan::CheckPhysicalDeviceFeatures()
{
	if (_physicalDevice.features.multiDrawIndirect == VK_FALSE)
		Print("Error: Vulkan multi draw indirect is not supported");
}

void triton::BGraphicsBackend2Vulkan::CheckPhysicalDeviceFeaturesVulkan13()
{
	if (_physicalDevice.featuresVulkan13.synchronization2 == VK_FALSE)
		Print("Error: Vulkan synchronization2 is not supported");
	if (_physicalDevice.featuresVulkan13.dynamicRendering == VK_FALSE)
		Print("Error: Vulkan dynamicRendering is not supported");
}

void triton::BGraphicsBackend2Vulkan::CheckPhysicalDeviceFormats()
{
	const VkFormat candidates[] =
	{
		VK_FORMAT_D32_SFLOAT
	};

	for (VkFormat format : candidates)
	{
		VkFormatProperties properties{};

		vkGetPhysicalDeviceFormatProperties(
			_physicalDevice.device,
			format,
			&properties
		);

		if (properties.optimalTilingFeatures &
			VK_FORMAT_FEATURE_DEPTH_STENCIL_ATTACHMENT_BIT)
			_physicalDeviceDepthBufferFormat = format;
	}
}

void triton::BGraphicsBackend2Vulkan::FindQueueFamilies()
{
	for (usize i = 0; i < _physicalDevice.queueFamilyProperties.size(); i++)
	{
		VkQueueFamilyProperties queueFamilyProperties = _physicalDevice.queueFamilyProperties.at(i);
		if (queueFamilyProperties.queueCount > 0 &&
			queueFamilyProperties.queueFlags & VkQueueFlagBits::VK_QUEUE_GRAPHICS_BIT)
			_graphicsQueue = SQueue(i, 0, {});
		if (queueFamilyProperties.queueCount > 0 &&
			queueFamilyProperties.queueFlags & VkQueueFlagBits::VK_QUEUE_TRANSFER_BIT)
			_transferQueue = SQueue(i, 0, {});
		if (queueFamilyProperties.queueCount > 0 &&
			queueFamilyProperties.queueFlags & VkQueueFlagBits::VK_QUEUE_COMPUTE_BIT)
			_computeQueue = SQueue(i, 0, {});

		VkBool32 presentSupported = VK_FALSE;
		vkGetPhysicalDeviceSurfaceSupportKHR(
			_physicalDevice.device,
			(uint32_t)(i),
			_surface.surface,
			&presentSupported
		);
		if (presentSupported == VK_TRUE)
			_presentQueue = SQueue(i, 0, {});

		if (queueFamilyProperties.queueCount > 0 &&
			queueFamilyProperties.queueFlags & VkQueueFlagBits::VK_QUEUE_GRAPHICS_BIT &&
			queueFamilyProperties.queueFlags & VkQueueFlagBits::VK_QUEUE_TRANSFER_BIT &&
			presentSupported == VK_TRUE)
		{
			_graphicsTransferPresentQueue = SQueue(i, 0, {});

			Print("[Vulkan]: Info: suitable queue family found");

			return;
		}
		else
		{
			Print("[Vulkan]: Error: suitable queue family was not found");
		}
	}
}

void triton::BGraphicsBackend2Vulkan::CreateLogicalDevice()
{
	std::set<usize> uniqueQueueFamilyIndices = {
		_graphicsQueue.familyIndex,
		_transferQueue.familyIndex,
		_computeQueue.familyIndex,
		_presentQueue.familyIndex,
		_graphicsTransferPresentQueue.familyIndex
	};

	const float priority = 1.0f;

	usize counter = 0;
	std::vector<VkDeviceQueueCreateInfo> queueCreateInfos(uniqueQueueFamilyIndices.size());
	for (auto queueFamilyIndex : uniqueQueueFamilyIndices)
	{
		queueCreateInfos[counter].sType = VK_STRUCTURE_TYPE_DEVICE_QUEUE_CREATE_INFO;
		queueCreateInfos[counter].queueCount = 1;
		queueCreateInfos[counter].queueFamilyIndex = (uint32_t)queueFamilyIndex;
		queueCreateInfos[counter].pQueuePriorities = &priority;
		++counter;
	}

	std::vector<const char*> extensions = {};
	extensions.push_back(VK_KHR_SWAPCHAIN_EXTENSION_NAME);

	VkDeviceCreateInfo createInfo = {};
	createInfo.sType = VK_STRUCTURE_TYPE_DEVICE_CREATE_INFO;
	createInfo.queueCreateInfoCount = (uint32_t)queueCreateInfos.size();
	createInfo.pQueueCreateInfos = queueCreateInfos.data();
	createInfo.enabledExtensionCount = extensions.size();
	createInfo.ppEnabledExtensionNames = extensions.data();

	VkResult result = vkCreateDevice(
		_physicalDevice.device,
		&createInfo,
		nullptr,
		&_logicalDevice.device
	);

	if (result != VK_SUCCESS)
		Print("Error: failed to create Vulkan logical device");

	vkGetDeviceQueue(
		_logicalDevice.device,
		_graphicsQueue.familyIndex,
		_graphicsQueue.queueIndex,
		&_graphicsQueue.queue
	);

	vkGetDeviceQueue(
		_logicalDevice.device,
		_transferQueue.familyIndex,
		_transferQueue.queueIndex,
		&_transferQueue.queue
	);

	vkGetDeviceQueue(
		_logicalDevice.device,
		_computeQueue.familyIndex,
		_computeQueue.queueIndex,
		&_computeQueue.queue
	);

	vkGetDeviceQueue(
		_logicalDevice.device,
		_presentQueue.familyIndex,
		_presentQueue.queueIndex,
		&_presentQueue.queue
	);

	vkGetDeviceQueue(
		_logicalDevice.device,
		_graphicsTransferPresentQueue.familyIndex,
		_graphicsTransferPresentQueue.queueIndex,
		&_graphicsTransferPresentQueue.queue
	);
}

void triton::BGraphicsBackend2Vulkan::DestroyLogicalDevice()
{
	if (_logicalDevice.device != VK_NULL_HANDLE)
		vkDestroyDevice(_logicalDevice.device, nullptr);
}

void triton::BGraphicsBackend2Vulkan::CreateCommandPoolsAndCommandBuffers()
{
	VkResult result;

	VkCommandPoolCreateInfo graphicsCreateInfo = {};
	graphicsCreateInfo.sType = VK_STRUCTURE_TYPE_COMMAND_POOL_CREATE_INFO;
	graphicsCreateInfo.flags = VK_COMMAND_POOL_CREATE_RESET_COMMAND_BUFFER_BIT;
	graphicsCreateInfo.queueFamilyIndex = _graphicsQueue.familyIndex;

	result = vkCreateCommandPool(
		_logicalDevice.device,
		&graphicsCreateInfo,
		nullptr,
		&_graphicsQueue.commandPool
	);

	if (result != VK_SUCCESS)
		Print("[Vulkan]: Error: failed to create graphics command pool");

	VkCommandPoolCreateInfo transferCreateInfo = {};
	transferCreateInfo.sType = VK_STRUCTURE_TYPE_COMMAND_POOL_CREATE_INFO;
	transferCreateInfo.flags = VK_COMMAND_POOL_CREATE_RESET_COMMAND_BUFFER_BIT;
	transferCreateInfo.queueFamilyIndex = _transferQueue.familyIndex;

	result = vkCreateCommandPool(
		_logicalDevice.device,
		&transferCreateInfo,
		nullptr,
		&_transferQueue.commandPool
	);

	if (result != VK_SUCCESS)
		Print("[Vulkan]: Error: failed to create transfer command pool");

	VkCommandPoolCreateInfo computeCreateInfo = {};
	computeCreateInfo.sType = VK_STRUCTURE_TYPE_COMMAND_POOL_CREATE_INFO;
	computeCreateInfo.flags = VK_COMMAND_POOL_CREATE_RESET_COMMAND_BUFFER_BIT;
	computeCreateInfo.queueFamilyIndex = _computeQueue.familyIndex;

	result = vkCreateCommandPool(
		_logicalDevice.device,
		&computeCreateInfo,
		nullptr,
		&_computeQueue.commandPool
	);

	if (result != VK_SUCCESS)
		Print("[Vulkan]: Error: failed to create compute command pool");

	VkCommandPoolCreateInfo graphicsPresentCreateInfo = {};
	graphicsPresentCreateInfo.sType = VK_STRUCTURE_TYPE_COMMAND_POOL_CREATE_INFO;
	graphicsPresentCreateInfo.flags = VK_COMMAND_POOL_CREATE_RESET_COMMAND_BUFFER_BIT;
	graphicsPresentCreateInfo.queueFamilyIndex = _graphicsTransferPresentQueue.familyIndex;

	result = vkCreateCommandPool(
		_logicalDevice.device,
		&graphicsCreateInfo,
		nullptr,
		&_graphicsTransferPresentQueue.commandPool
	);

	if (result != VK_SUCCESS)
		Print("[Vulkan]: Error: failed to create compute command pool");

	VkCommandBufferAllocateInfo graphicsAllocInfo = {};
	graphicsAllocInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO;
	graphicsAllocInfo.commandPool = _graphicsQueue.commandPool;
	graphicsAllocInfo.level = VK_COMMAND_BUFFER_LEVEL_PRIMARY;
	graphicsAllocInfo.commandBufferCount = 1;

	result = vkAllocateCommandBuffers(
		_logicalDevice.device,
		&graphicsAllocInfo,
		&_graphicsQueue.commandBuffer
	);

	if (result != VK_SUCCESS)
		Print("[Vulkan]: Error: failed to create graphics command buffer");

	VkCommandBufferAllocateInfo transferAllocInfo = {};
	transferAllocInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO;
	transferAllocInfo.commandPool = _transferQueue.commandPool;
	transferAllocInfo.level = VK_COMMAND_BUFFER_LEVEL_PRIMARY;
	transferAllocInfo.commandBufferCount = 1;

	result = vkAllocateCommandBuffers(
		_logicalDevice.device,
		&transferAllocInfo,
		&_transferQueue.commandBuffer
	);

	if (result != VK_SUCCESS)
		Print("[Vulkan]: Error: failed to create transfer command buffer");

	VkCommandBufferAllocateInfo computeAllocInfo = {};
	computeAllocInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO;
	computeAllocInfo.commandPool = _computeQueue.commandPool;
	computeAllocInfo.level = VK_COMMAND_BUFFER_LEVEL_PRIMARY;
	computeAllocInfo.commandBufferCount = 1;

	result = vkAllocateCommandBuffers(
		_logicalDevice.device,
		&computeAllocInfo,
		&_computeQueue.commandBuffer
	);

	if (result != VK_SUCCESS)
		Print("[Vulkan]: Error: failed to create compute command buffer");

	VkCommandBufferAllocateInfo frameCommandBufferAllocInfo = {};
	frameCommandBufferAllocInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO;
	frameCommandBufferAllocInfo.commandPool = _graphicsTransferPresentQueue.commandPool;
	frameCommandBufferAllocInfo.level = VK_COMMAND_BUFFER_LEVEL_PRIMARY;
	frameCommandBufferAllocInfo.commandBufferCount = _framesInFlight;

	_frameCommandBuffers.resize(_framesInFlight);

	result = vkAllocateCommandBuffers(
		_logicalDevice.device,
		&frameCommandBufferAllocInfo,
		_frameCommandBuffers.data()
	);

	if (result != VK_SUCCESS)
		Print("[Vulkan]: Error: failed to create frame command buffers");

	VkCommandBufferAllocateInfo stagingCommandBufferAllocInfo = {};
	stagingCommandBufferAllocInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO;
	stagingCommandBufferAllocInfo.commandPool = _graphicsTransferPresentQueue.commandPool;
	stagingCommandBufferAllocInfo.level = VK_COMMAND_BUFFER_LEVEL_PRIMARY;
	stagingCommandBufferAllocInfo.commandBufferCount = _framesInFlight;

	_stagingCommandBuffers.resize(_framesInFlight);

	result = vkAllocateCommandBuffers(
		_logicalDevice.device,
		&stagingCommandBufferAllocInfo,
		_stagingCommandBuffers.data()
	);

	if (result != VK_SUCCESS)
		Print("[Vulkan]: Error: failed to create staging command buffers");
}

void triton::BGraphicsBackend2Vulkan::DestroyCommandPoolsAndCommandBuffers()
{
	vkFreeCommandBuffers(
		_logicalDevice.device,
		_graphicsTransferPresentQueue.commandPool,
		_stagingCommandBuffers.size(),
		_stagingCommandBuffers.data()
	);

	vkFreeCommandBuffers(
		_logicalDevice.device,
		_graphicsTransferPresentQueue.commandPool,
		_frameCommandBuffers.size(),
		_frameCommandBuffers.data()
	);

	vkFreeCommandBuffers(
		_logicalDevice.device,
		_computeQueue.commandPool,
		1,
		&_computeQueue.commandBuffer
	);

	vkFreeCommandBuffers(
		_logicalDevice.device,
		_transferQueue.commandPool,
		1,
		&_transferQueue.commandBuffer
	);

	vkFreeCommandBuffers(
		_logicalDevice.device,
		_graphicsQueue.commandPool,
		1,
		&_graphicsQueue.commandBuffer
	);

	vkDestroyCommandPool(
		_logicalDevice.device,
		_graphicsTransferPresentQueue.commandPool,
		nullptr
	);

	vkDestroyCommandPool(
		_logicalDevice.device,
		_computeQueue.commandPool,
		nullptr
	);

	vkDestroyCommandPool(
		_logicalDevice.device,
		_transferQueue.commandPool,
		nullptr
	);

	vkDestroyCommandPool(
		_logicalDevice.device,
		_graphicsQueue.commandPool,
		nullptr
	);
}

void triton::BGraphicsBackend2Vulkan::CreateSwapchain(const cVector2& size, usize framesInFlight)
{
	VkResult result;

	VkSurfaceCapabilitiesKHR surfaceCapabilities = {};
	result = vkGetPhysicalDeviceSurfaceCapabilitiesKHR(
		_physicalDevice.device,
		_surface.surface,
		&surfaceCapabilities
	);

	if (result != VK_SUCCESS)
		Print("[Vulkan]: Error: failed to query surface capabilities");

	uint32_t formatCount = 0;
	std::vector<VkSurfaceFormatKHR> formats = {};

	result = vkGetPhysicalDeviceSurfaceFormatsKHR(
		_physicalDevice.device,
		_surface.surface,
		&formatCount,
		nullptr
	);

	if (result != VK_SUCCESS)
		Print("[Vulkan]: Error: failed to query surface format count");

	if (formatCount != 0)
	{
		formats.resize(formatCount);

		result = vkGetPhysicalDeviceSurfaceFormatsKHR(
			_physicalDevice.device,
			_surface.surface,
			&formatCount,
			formats.data()
		);

		if (result != VK_SUCCESS)
			Print("[Vulkan]: Error: failed to query surface formats");
	}

	uint32_t presentModeCount = 0;
	std::vector<VkPresentModeKHR> presentModes = {};

	result = vkGetPhysicalDeviceSurfacePresentModesKHR(
		_physicalDevice.device,
		_surface.surface,
		&presentModeCount,
		nullptr
	);

	if (result != VK_SUCCESS)
		Print("[Vulkan]: Error: failed to query surface present mode count");

	if (presentModeCount != 0)
	{
		presentModes.resize(presentModeCount);

		result = vkGetPhysicalDeviceSurfacePresentModesKHR(
			_physicalDevice.device,
			_surface.surface,
			&presentModeCount,
			presentModes.data()
		);

		if (result != VK_SUCCESS)
			Print("[Vulkan]: Error: failed to query surface present modes");
	}

	if (formats.empty() || presentModes.empty())
		Print("[Vulkan]: Error: inadequate swapchain support");

	VkSurfaceFormatKHR swapchainSurfaceFormat = ChooseSwapchainSurfaceFormat(formats);

	VkPresentModeKHR swapchainPresentMode = ChooseSwapchainPresentMode(presentModes);

	VkExtent2D swapchainExtent = ChooseSwapchainExtent(surfaceCapabilities, size);

	uint32_t swapchainImageCount = 3;

	if (surfaceCapabilities.maxImageCount > 0 &&
		swapchainImageCount > surfaceCapabilities.maxImageCount)
		swapchainImageCount = surfaceCapabilities.maxImageCount;

	VkSwapchainCreateInfoKHR swapchainCreateInfo = {};
	swapchainCreateInfo.sType = VK_STRUCTURE_TYPE_SWAPCHAIN_CREATE_INFO_KHR;
	swapchainCreateInfo.surface = _surface.surface;
	swapchainCreateInfo.minImageCount = swapchainImageCount;
	swapchainCreateInfo.imageFormat = swapchainSurfaceFormat.format;
	swapchainCreateInfo.imageColorSpace = swapchainSurfaceFormat.colorSpace;
	swapchainCreateInfo.imageExtent = swapchainExtent;
	swapchainCreateInfo.imageArrayLayers = 1;
	swapchainCreateInfo.imageUsage = VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT;

	/*const uint32_t queueFamilyIndices[] = {
		_graphicsQueue.familyIndex,
		_presentQueue.familyIndex
	};

	if (_graphicsQueue.familyIndex != _presentQueue.familyIndex)
	{
		swapchainCreateInfo.imageSharingMode = VK_SHARING_MODE_CONCURRENT;
		swapchainCreateInfo.queueFamilyIndexCount = 2;
		swapchainCreateInfo.pQueueFamilyIndices = queueFamilyIndices;
	}
	else
	{
		swapchainCreateInfo.imageSharingMode = VK_SHARING_MODE_EXCLUSIVE;
		swapchainCreateInfo.queueFamilyIndexCount = 0;
		swapchainCreateInfo.pQueueFamilyIndices = nullptr;
	}*/

	swapchainCreateInfo.imageSharingMode = VK_SHARING_MODE_EXCLUSIVE;
	swapchainCreateInfo.queueFamilyIndexCount = 0;
	swapchainCreateInfo.pQueueFamilyIndices = nullptr;

	swapchainCreateInfo.preTransform = surfaceCapabilities.currentTransform;
	swapchainCreateInfo.compositeAlpha = VK_COMPOSITE_ALPHA_OPAQUE_BIT_KHR;
	swapchainCreateInfo.presentMode = swapchainPresentMode;
	swapchainCreateInfo.clipped = VK_TRUE;
	swapchainCreateInfo.oldSwapchain = VK_NULL_HANDLE;

	result = vkCreateSwapchainKHR(
		_logicalDevice.device,
		&swapchainCreateInfo,
		nullptr,
		&_swapchain.swapchain
	);

	if (result != VK_SUCCESS)
		Print("[Vulkan]: Error: failed to create swapchain");

	_swapchain.size = size;
	_swapchain.format = swapchainSurfaceFormat.format;

	_swapchainBindingGroupLayout = CreateBindingGroupLayout(
		{
			SRenderBindingGroupBinding(
				0,
				ERenderBindingGroupBindingType::TextureSampler,
				EShaderStageBit::Pixel,
				nullptr
			)
		}
	);

	GetSwapchainImages();

	CreateSwapchainRenderTargets();

	CreateSwapchainRenderPasses();

	CreateSwapchainShader();

	CreateSwapchainSemaphoresAndFences();
}

void triton::BGraphicsBackend2Vulkan::DestroySwapchain()
{
	DestroySwapchainSemaphoresAndFences();

	DestroySwapchainShader();

	DestroySwapchainRenderTargets();

	DestroySwapchainRenderPasses();

	for (auto& image : _swapchain.images)
		if (image.view != VK_NULL_HANDLE)
			vkDestroyImageView(
				_logicalDevice.device,
				image.view,
				nullptr
			);

	vkDestroySwapchainKHR(
		_logicalDevice.device,
		_swapchain.swapchain,
		nullptr
	);

	DestroyBindingGroupLayout(_swapchainBindingGroupLayout);
}

void triton::BGraphicsBackend2Vulkan::CreateStagingBuffer(usize byte_size)
{
	_staging_buffer = CreateBuffer(EGPUBufferType::Staging, byte_size);
}

void triton::BGraphicsBackend2Vulkan::DestroyStagingBuffer()
{
	DestroyBuffer(_staging_buffer);
}

void triton::BGraphicsBackend2Vulkan::CreateDescriptorPool()
{
	const SCapabilities& caps = _context->GetSubsystem<XEngine>()->GetApplication()->GetCapabilities();

	VkDescriptorPoolSize sizes[] =
	{
		{ VK_DESCRIPTOR_TYPE_STORAGE_BUFFER, caps.maxStorageBufferBindingCount },
		{ VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER, caps.maxTextureSamplerBindingCount }
	};

	VkDescriptorPoolCreateInfo poolInfo = {};
	poolInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_POOL_CREATE_INFO;
	poolInfo.flags = VK_DESCRIPTOR_POOL_CREATE_FREE_DESCRIPTOR_SET_BIT;
	poolInfo.poolSizeCount = std::size(sizes);
	poolInfo.pPoolSizes = &sizes[0];
	poolInfo.maxSets = caps.maxBindingGroupCount;

	VkResult result = vkCreateDescriptorPool(
		_logicalDevice.device,
		&poolInfo,
		nullptr,
		&_descriptorPool
	);

	if (result != VK_SUCCESS)
		Print("[Vulkan]: Error: failed to create descriptor pool");
}

void triton::BGraphicsBackend2Vulkan::DestroyDescriptorPool()
{
	if (_descriptorPool != VK_NULL_HANDLE)
		vkDestroyDescriptorPool(
			_logicalDevice.device,
			_descriptorPool,
			nullptr
		);
}

VkSurfaceFormatKHR triton::BGraphicsBackend2Vulkan::ChooseSwapchainSurfaceFormat(
	const std::vector<VkSurfaceFormatKHR>& formats
)
{
	for (const auto& availableFormat : formats)
		if (availableFormat.format == VK_FORMAT_B8G8R8A8_SRGB &&
			availableFormat.colorSpace == VK_COLOR_SPACE_SRGB_NONLINEAR_KHR)
			return availableFormat;

	return formats[0];
}

VkPresentModeKHR triton::BGraphicsBackend2Vulkan::ChooseSwapchainPresentMode(
	const std::vector<VkPresentModeKHR>& presentModes
)
{
	for (const auto& availablePresentMode : presentModes)
		if (availablePresentMode == VK_PRESENT_MODE_MAILBOX_KHR)
			return availablePresentMode;

	return VK_PRESENT_MODE_FIFO_KHR;
}

VkExtent2D triton::BGraphicsBackend2Vulkan::ChooseSwapchainExtent(
	VkSurfaceCapabilitiesKHR capabilities,
	const cVector2& size
)
{
	if (capabilities.currentExtent.width != UINT32_MAX)
	{
		return capabilities.currentExtent;
	}
	else
	{
		VkExtent2D actualExtent = {
			(uint32_t)size.GetX(),
			(uint32_t)size.GetY(),
		};

		actualExtent.width = std::clamp(
			actualExtent.width,
			capabilities.minImageExtent.width,
			capabilities.maxImageExtent.width
		);
		actualExtent.height = std::clamp(
			actualExtent.height,
			capabilities.minImageExtent.height,
			capabilities.maxImageExtent.height
		);

		return actualExtent;
	}
}

void triton::BGraphicsBackend2Vulkan::GetSwapchainImages()
{
	VkResult result;

	uint32_t imageCount = 0;

	result = vkGetSwapchainImagesKHR(
		_logicalDevice.device,
		_swapchain.swapchain,
		&imageCount,
		nullptr
	);

	if (result != VK_SUCCESS)
		Print("[Vulkan]: Error: failed to query swapchain image count");

	_swapchain.images.reserve(imageCount);

	std::vector<VkImage> images(imageCount);

	result = vkGetSwapchainImagesKHR(
		_logicalDevice.device,
		_swapchain.swapchain,
		&imageCount,
		images.data()
	);

	for (usize i = 0; i < imageCount; i++)
		_swapchain.images.push_back(
			SImageWithView(
				_swapchain.format,
				images[i],
				VK_NULL_HANDLE
			)
		);

	if (result != VK_SUCCESS)
		Print("[Vulkan]: Error: failed to query swapchain images");

	for (auto& image : _swapchain.images)
	{
		VkImageViewCreateInfo createInfo = {};
		createInfo.sType = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO;

		createInfo.image = image.image;
		createInfo.viewType = VK_IMAGE_VIEW_TYPE_2D;
		createInfo.format = _swapchain.format;

		createInfo.components.r = VK_COMPONENT_SWIZZLE_IDENTITY;
		createInfo.components.g = VK_COMPONENT_SWIZZLE_IDENTITY;
		createInfo.components.b = VK_COMPONENT_SWIZZLE_IDENTITY;
		createInfo.components.a = VK_COMPONENT_SWIZZLE_IDENTITY;

		createInfo.subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;

		createInfo.subresourceRange.baseMipLevel = 0;
		createInfo.subresourceRange.levelCount = 1;
		createInfo.subresourceRange.baseArrayLayer = 0;
		createInfo.subresourceRange.layerCount = 1;

		VkResult result = vkCreateImageView(
			_logicalDevice.device,
			&createInfo,
			nullptr,
			&image.view
		);

		if (result != VK_SUCCESS)
			Print("[Vulkan]: Error: failed to create swapchain image views");
	}
}

void triton::BGraphicsBackend2Vulkan::CreateSwapchainRenderTargets()
{
	const usize renderTargetCount = _swapchain.images.size();

	_swapchainRenderTargets.reserve(renderTargetCount);

	for (usize i = 0; i < renderTargetCount; i++)
	{
		SGPUTextureResource colorAttachment = SGPUTextureResource(
			(qword)_swapchain.images[i].image,
			(qword)_swapchain.images[i].view,
			0,
			0,
			ETextureFormat::BGRA8_SRGB,
			(dword)ETextureUsageBit::ColorAttachment,
			ETextureDimension::Texture2D,
			cVector3(_swapchain.size.GetX(), _swapchain.size.GetY(), 0.0f),
			0
		);

		const std::vector<SGPUTextureResource> colorAttachmentVec = { colorAttachment };

		_swapchainRenderTargets.push_back(SGPURenderTargetResource(
			(cpuword)1,
			0,
			colorAttachmentVec,
			SGPUTextureResource::Invalid()
		));
	}
}

void triton::BGraphicsBackend2Vulkan::DestroySwapchainRenderTargets()
{
	for (auto& renderTarget : _swapchainRenderTargets)
		renderTarget.Invalidate();
}

void triton::BGraphicsBackend2Vulkan::CreateSwapchainRenderPasses()
{
	_swapchainRenderPasses.reserve(_swapchainRenderTargets.size());
	for (auto& renderTarget : _swapchainRenderTargets)
		_swapchainRenderPasses.push_back(
			CreateRenderPass(
				renderTarget,
				True,
				cVector4(1.0f),
				1.0f,
				{ ERenderResourceUsage::Unknown },
				{ ERenderResourceUsage::Present }
			)
		);
}

void triton::BGraphicsBackend2Vulkan::DestroySwapchainRenderPasses()
{
	for (auto& renderPass : _swapchainRenderPasses)
		DestroyRenderPass(renderPass);
}

void triton::BGraphicsBackend2Vulkan::CreateSwapchainShader()
{
	SShaderBytecodeFiles bytecodeFiles;
	bytecodeFiles.vertexFilePath =
		"C:/My/My_Projects_Programming/TritonEngine/runtime/data/shaders/builtin/passthrough_final.vert.spv";
	bytecodeFiles.pixelFilePath =
		"C:/My/My_Projects_Programming/TritonEngine/runtime/data/shaders/builtin/passthrough_final.frag.spv";
	
	_swapchainShader = CreateShader(
		(dword)EShaderStageBit::Vertex | (dword)EShaderStageBit::Pixel,
		bytecodeFiles
	);
}

void triton::BGraphicsBackend2Vulkan::DestroySwapchainShader()
{
	if (_swapchainShader.IsValid() == True)
	{
		DestroyShader(_swapchainShader);
		
		_swapchainShader.Invalidate();
	}
}

void triton::BGraphicsBackend2Vulkan::CreateSwapchainSemaphoresAndFences()
{
	VkResult result;

	VkSemaphoreCreateInfo semaphoreCreateInfo = {};
	semaphoreCreateInfo.sType = VK_STRUCTURE_TYPE_SEMAPHORE_CREATE_INFO;

	_swapchainImageAvailableSemaphores.resize(_framesInFlight);

	for (usize i = 0; i < _framesInFlight; i++)
	{
		result = vkCreateSemaphore(
			_logicalDevice.device,
			&semaphoreCreateInfo,
			nullptr,
			&_swapchainImageAvailableSemaphores[i].semaphore
		);

		if (result != VK_SUCCESS)
			Print("[Vulkan]: Error: failed to create swapchain semaphore");
	}

	const usize renderFinishedSemaphoreCount = _swapchain.images.size();
	_swapchainRenderFinishedSemaphores.resize(renderFinishedSemaphoreCount);

	for (usize i = 0; i < renderFinishedSemaphoreCount; i++)
	{
		result = vkCreateSemaphore(
			_logicalDevice.device,
			&semaphoreCreateInfo,
			nullptr,
			&_swapchainRenderFinishedSemaphores[i].semaphore
		);

		if (result != VK_SUCCESS)
			Print("[Vulkan]: Error: failed to create swapchain semaphore");
	}

	VkFenceCreateInfo fenceCreateInfo = {};
	fenceCreateInfo.sType = VK_STRUCTURE_TYPE_FENCE_CREATE_INFO;
	fenceCreateInfo.flags = VK_FENCE_CREATE_SIGNALED_BIT;

	_swapchainFences.resize(_framesInFlight);

	for (usize i = 0; i < _framesInFlight; i++)
	{
		result = vkCreateFence(
			_logicalDevice.device,
			&fenceCreateInfo,
			nullptr,
			&_swapchainFences[i].fence
		);

		if (result != VK_SUCCESS)
			Print("[Vulkan]: Error: failed to create swapchain fence");
	}
}

void triton::BGraphicsBackend2Vulkan::DestroySwapchainSemaphoresAndFences()
{
	for (auto& fence : _swapchainFences)
		vkDestroyFence(
			_logicalDevice.device,
			fence.fence,
			nullptr
		);

	for (auto& semaphore : _swapchainRenderFinishedSemaphores)
		vkDestroySemaphore(
			_logicalDevice.device,
			semaphore.semaphore,
			nullptr
		);

	for (auto& semaphore : _swapchainImageAvailableSemaphores)
		vkDestroySemaphore(
			_logicalDevice.device,
			semaphore.semaphore,
			nullptr
		);
}

void triton::BGraphicsBackend2Vulkan::CreateStagingFence()
{
	VkFenceCreateInfo fenceCreateInfo = {};
	fenceCreateInfo.sType = VK_STRUCTURE_TYPE_FENCE_CREATE_INFO;
	fenceCreateInfo.flags = VK_FENCE_CREATE_SIGNALED_BIT;

	_stagingFences.resize(_framesInFlight);

	VkResult result;

	for (usize i = 0; i < _framesInFlight; i++)
	{
		result = vkCreateFence(
			_logicalDevice.device,
			&fenceCreateInfo,
			nullptr,
			&_stagingFences[i].fence
		);

		if (result != VK_SUCCESS)
			Print("[Vulkan]: Error: failed to create staging fence");
	}
}

void triton::BGraphicsBackend2Vulkan::DestroyStagingFence()
{
	for (auto& fence : _stagingFences)
		vkDestroyFence(
			_logicalDevice.device,
			fence.fence,
			nullptr
		);
}

usize triton::BGraphicsBackend2Vulkan::FindProperMemoryTypeIndex(
	VkPhysicalDeviceMemoryProperties memoryProperties,
	VkMemoryRequirements requirements,
	dword requiredMemoryPropertyBits
)
{
	uint32_t memoryTypeIndex = UINT32_MAX;

	for (usize i = 0; i < memoryProperties.memoryTypeCount; ++i)
	{
		const bool typeSupported = (requirements.memoryTypeBits & (1u << i)) != 0;

		const bool propertiesSupported =
			(memoryProperties.memoryTypes[i].propertyFlags & requiredMemoryPropertyBits)
			== requiredMemoryPropertyBits;

		if (typeSupported && propertiesSupported)
			return i;
	}

	Print("[Vulkan]: Info: selected memory type index: 0");

	return 0;
}

usize triton::BGraphicsBackend2Vulkan::TextureFormatToChannelCount(ETextureFormat textureFormat)
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

VkDeviceMemory triton::BGraphicsBackend2Vulkan::AllocateDeviceMemory(
	VkMemoryRequirements requirements,
	dword requiredMemoryPropertyBits
)
{
	VkPhysicalDeviceMemoryProperties memoryProperties = {};
	vkGetPhysicalDeviceMemoryProperties(
		_physicalDevice.device,
		&memoryProperties
	);

	usize memoryTypeIndex = FindProperMemoryTypeIndex(
		memoryProperties,
		requirements,
		requiredMemoryPropertyBits
	);

	VkMemoryAllocateInfo memoryAllocateInfo = {};
	memoryAllocateInfo.sType = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO;
	memoryAllocateInfo.allocationSize = requirements.size;
	memoryAllocateInfo.memoryTypeIndex = memoryTypeIndex;

	VkDeviceMemory memory = VK_NULL_HANDLE;

	VkResult result = vkAllocateMemory(
		_logicalDevice.device,
		&memoryAllocateInfo,
		nullptr,
		&memory
	);

	if (result != VK_SUCCESS)
		Print("[Vulkan]: Error: failed to allocate device memory");

	return memory;
}

VkFormat triton::BGraphicsBackend2Vulkan::TextureFormatToNative(ETextureFormat textureFormat)
{
	if (textureFormat == ETextureFormat::R8)
		return VK_FORMAT_R8_UNORM;
	else if (textureFormat == ETextureFormat::RGBA8)
		return VK_FORMAT_R8G8B8A8_UNORM;
	else if (textureFormat == ETextureFormat::RGBA8_SRGB ||
		textureFormat == ETextureFormat::RGBA8_SRGB_Mips)
		return VK_FORMAT_R8G8B8A8_SRGB;
	else if (textureFormat == ETextureFormat::BGRA8_SRGB)
		return VK_FORMAT_B8G8R8A8_SRGB;
	else if (textureFormat == ETextureFormat::DepthStencil)
		return _physicalDeviceDepthBufferFormat;

	return VK_FORMAT_UNDEFINED;
}

VkImageUsageFlags triton::BGraphicsBackend2Vulkan::TextureUsageToNative(dword textureUsageMask)
{
	VkImageUsageFlags flags = 0;

	if (textureUsageMask & (dword)ETextureUsageBit::Sampled)
		flags |= VK_IMAGE_USAGE_SAMPLED_BIT | VK_IMAGE_USAGE_TRANSFER_DST_BIT;
	if (textureUsageMask & (dword)ETextureUsageBit::ColorAttachment)
		flags |= VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT;
	if (textureUsageMask & (dword)ETextureUsageBit::DepthStencilAttachment)
		flags |= VK_IMAGE_USAGE_DEPTH_STENCIL_ATTACHMENT_BIT;

	return flags;
}

VkImageType triton::BGraphicsBackend2Vulkan::TextureDimensionToNative(ETextureDimension textureDimension)
{
	VkImageType type = VK_IMAGE_TYPE_2D;

	return type;
}

VkImageLayout triton::BGraphicsBackend2Vulkan::AttachmentUsageToNativeLayout(ERenderResourceUsage attachmentUsage)
{
	if (attachmentUsage == ERenderResourceUsage::Unknown)
		return VK_IMAGE_LAYOUT_UNDEFINED;
	else if (attachmentUsage == ERenderResourceUsage::ColorAttachment)
		return VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;
	else if (attachmentUsage == ERenderResourceUsage::DepthAttachment)
		return VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL;
	else if (attachmentUsage == ERenderResourceUsage::VertexShaderRead)
		return VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;
	else if (attachmentUsage == ERenderResourceUsage::PixelShaderRead)
		return VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;
	else if (attachmentUsage == ERenderResourceUsage::Present)
		return VK_IMAGE_LAYOUT_PRESENT_SRC_KHR;

	return VK_IMAGE_LAYOUT_UNDEFINED;
}

VkPipelineBindPoint triton::BGraphicsBackend2Vulkan::PipelineBindPointToNative(ERenderPipelineBindPoint bindPoint)
{
	return VK_PIPELINE_BIND_POINT_GRAPHICS;
}

VkViewport triton::BGraphicsBackend2Vulkan::ViewportToNative(const SViewport& viewport)
{
	VkViewport nativeViewport = {};
	nativeViewport.x = viewport.rect.GetX();
	nativeViewport.y = viewport.rect.GetY();
	nativeViewport.width = viewport.rect.GetZ();
	nativeViewport.height = viewport.rect.GetW();
	nativeViewport.minDepth = 0.0f;
	nativeViewport.maxDepth = 1.0f;

	return nativeViewport;
}

VkPrimitiveTopology triton::BGraphicsBackend2Vulkan::PrimitiveTopologyToNative(ERenderPrimitiveTopology primitiveTopology)
{
	if (primitiveTopology == ERenderPrimitiveTopology::TriangleStrip)
		return VK_PRIMITIVE_TOPOLOGY_TRIANGLE_STRIP;

	return VK_PRIMITIVE_TOPOLOGY_TRIANGLE_LIST;
}

VkBufferUsageFlags triton::BGraphicsBackend2Vulkan::BufferTypeToNative(EGPUBufferType bufferType)
{
	if (bufferType == EGPUBufferType::Vertex)
		return VK_BUFFER_USAGE_VERTEX_BUFFER_BIT | VK_BUFFER_USAGE_TRANSFER_DST_BIT;
	else if (bufferType == EGPUBufferType::Index)
		return VK_BUFFER_USAGE_INDEX_BUFFER_BIT | VK_BUFFER_USAGE_TRANSFER_DST_BIT;
	else if (bufferType == EGPUBufferType::Uniform)
		return VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT | VK_BUFFER_USAGE_TRANSFER_DST_BIT;
	else if (bufferType == EGPUBufferType::Storage)
		return VK_BUFFER_USAGE_STORAGE_BUFFER_BIT | VK_BUFFER_USAGE_TRANSFER_DST_BIT;
	else if (bufferType == EGPUBufferType::Staging)
		return VK_BUFFER_USAGE_TRANSFER_SRC_BIT;

	return 0;
}

triton::ERenderImageAspectBit triton::BGraphicsBackend2Vulkan::TextureFormatToAspect(ETextureFormat format)
{
	ERenderImageAspectBit aspect_mask = ERenderImageAspectBit::Unknown;

	if (format == ETextureFormat::DepthStencil)
		aspect_mask = ERenderImageAspectBit::Depth;
	else
		aspect_mask = ERenderImageAspectBit::Color;

	return aspect_mask;
}

VkPipelineStageFlags triton::BGraphicsBackend2Vulkan::PipelineStageToNative(ERenderPipelineStageBit stage_mask)
{
	VkPipelineStageFlags native_stage_mask = VK_PIPELINE_STAGE_NONE;

	if ((dword)stage_mask & (dword)ERenderPipelineStageBit::TopOfPipe)
		native_stage_mask |= VK_PIPELINE_STAGE_TOP_OF_PIPE_BIT;
	if ((dword)stage_mask & (dword)ERenderPipelineStageBit::Vertex)
		native_stage_mask |= VK_PIPELINE_STAGE_VERTEX_SHADER_BIT;
	if ((dword)stage_mask & (dword)ERenderPipelineStageBit::VertexInput)
		native_stage_mask |= VK_PIPELINE_STAGE_VERTEX_INPUT_BIT;
	if ((dword)stage_mask & (dword)ERenderPipelineStageBit::Pixel)
		native_stage_mask |= VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT;
	if ((dword)stage_mask & (dword)ERenderPipelineStageBit::Compute)
		native_stage_mask |= VK_PIPELINE_STAGE_COMPUTE_SHADER_BIT;
	if ((dword)stage_mask & (dword)ERenderPipelineStageBit::Transfer)
		native_stage_mask |= VK_PIPELINE_STAGE_TRANSFER_BIT;

	return native_stage_mask;
}

VkAccessFlags triton::BGraphicsBackend2Vulkan::AccessToNative(ERenderResourceAccessBit access_mask)
{
	VkAccessFlags native_access_mask = 0;

	if ((dword)access_mask & (dword)ERenderResourceAccessBit::None)
		native_access_mask |= VK_ACCESS_NONE;
	if ((dword)access_mask & (dword)ERenderResourceAccessBit::IndexRead)
		native_access_mask |= VK_ACCESS_INDEX_READ_BIT;
	if ((dword)access_mask & (dword)ERenderResourceAccessBit::ShaderRead)
		native_access_mask |= VK_ACCESS_SHADER_READ_BIT;
	if ((dword)access_mask & (dword)ERenderResourceAccessBit::ShaderWrite)
		native_access_mask |= VK_ACCESS_SHADER_WRITE_BIT;
	if ((dword)access_mask & (dword)ERenderResourceAccessBit::TransferWrite)
		native_access_mask |= VK_ACCESS_TRANSFER_WRITE_BIT;

	return native_access_mask;
}

VkImageAspectFlags triton::BGraphicsBackend2Vulkan::ImageAspectToNative(ERenderImageAspectBit aspect_mask)
{
	VkImageAspectFlags native_aspect_mask = 0;

	if ((dword)aspect_mask & (dword)ERenderImageAspectBit::Color)
		native_aspect_mask |= VK_IMAGE_ASPECT_COLOR_BIT;
	if ((dword)aspect_mask & (dword)ERenderImageAspectBit::Depth)
		native_aspect_mask |= VK_IMAGE_ASPECT_DEPTH_BIT;

	return native_aspect_mask;
}

VkImageLayout triton::BGraphicsBackend2Vulkan::ImageLayoutToNative(ERenderImageLayout layout)
{
	VkImageLayout native_layout = VK_IMAGE_LAYOUT_UNDEFINED;

	if (layout == ERenderImageLayout::ShaderRead)
		native_layout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;
	else if (layout == ERenderImageLayout::TransferDst)
		native_layout = VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL;

	return native_layout;
}

std::string triton::BGraphicsBackend2Vulkan::ShaderSourceInclude(
	const std::string& shaderSource,
	const std::string& includeStr
)
{
	return includeStr + std::string("\n\n") + shaderSource;
}