// graphics_backend2_vulkan.hpp

#pragma once

#include <vector>
#include <vulkan/vulkan.h>
#include "object.hpp"
#include "graphics_backend2.hpp"
#include "graphics_backend2_vulkan_structs.hpp"
#include "texture_formats.hpp"
#include "render_native_command_struct.hpp"
#include "render_pipeline_stage_bit_enum.hpp"
#include "render_resource_access_bit_enum.hpp"
#include "render_image_aspect_bit_enum.hpp"
#include "render_image_layout_enum.hpp"

namespace triton
{
	class BGraphicsBackend2Vulkan : public IGraphicsBackend2
	{
		SInstance										_instance;
		SSurface										_surface;
		SPhysicalDevice									_physicalDevice = SPhysicalDevice(
			EGraphicsAdapterType::Unknown, VK_NULL_HANDLE, {}, {}, {}, {}, std::vector<VkQueueFamilyProperties>(), {}
		);
		VkFormat										_physicalDeviceDepthBufferFormat = VK_FORMAT_UNDEFINED;
		SQueue											_graphicsQueue = SQueue(0, 0, {});
		SQueue											_transferQueue = SQueue(0, 0, {});
		SQueue											_computeQueue = SQueue(0, 0, {});
		SQueue											_presentQueue = SQueue(0, 0, {});
		SQueue											_graphicsTransferPresentQueue = SQueue(0, 0, {});
		SLogicalDevice									_logicalDevice;
		SGPUBufferResource								_staging_buffer = SGPUBufferResource::Invalid();
		SSwapchain										_swapchain;
		std::vector<SGPURenderTargetResource>			_swapchainRenderTargets;
		std::vector<SGPURenderPassResource>				_swapchainRenderPasses;
		SGPUShaderResource								_swapchainShader = SGPUShaderResource::Invalid();
		std::vector<SGPUPipelineResource>				_swapchainPipelines;
		SGPUBindingGroupLayoutResource					_swapchainBindingGroupLayout = SGPUBindingGroupLayoutResource::Invalid();
		std::vector<SGPUBindingGroupResource>			_swapchainBindingGroups;
		std::vector<SSemaphore>							_swapchainImageAvailableSemaphores;
		std::vector<SSemaphore>							_swapchainRenderFinishedSemaphores;
		std::vector<SFence>								_swapchainFences;
		SGPUTextureResource								_swapchainPresentTexture = SGPUTextureResource::Invalid();
		VkDescriptorPool								_descriptorPool = VK_NULL_HANDLE;
		std::vector<VkCommandBuffer>					_frameCommandBuffers;
		std::vector<VkCommandBuffer>					_stagingCommandBuffers;
		std::vector<SFence>								_stagingFences;
		types::usize									_framesInFlight = 0;
		types::usize									_currentFrame = 0;

	public:
		explicit BGraphicsBackend2Vulkan(cContext* context) : IGraphicsBackend2(context) {}
		~BGraphicsBackend2Vulkan() override = default;

		// Initialization/Shutdown
		void Initialize(
			SWindowResource& window,
			types::boolean bEnableDebugging,
			const std::vector<const char*> extensions,
			EGraphicsAdapterType deviceType,
			types::usize staging_buffer_byte_size,
			const cVector2& swapchainSize,
			types::usize framesInFlight
		) override final;

		void Shutdown() override final;

		void FinalizeSwapchain(const SGPUTextureResource& presentTexture) override final;

		void ReleaseSwapchainResources() override final;

		SGPUTextureResource CreateTexture(
			types::boolean bCreateSampler,
			ETextureFormat format,
			types::dword usageMask,
			ETextureDimension dimension,
			const cVector3& size,
			SGPUTextureResource* resultTexture
		) override final;

		void WriteTexture(
			const SGPUTextureResource& texture,
			const types::u8* data
		) override final;

		void DestroyTexture(SGPUTextureResource& texture) override final;

		SGPUBufferResource CreateBuffer(
			EGPUBufferType type,
			const types::usize byteSize
		) override final;

		void WriteBuffer(
			const SGPUBufferResource& buffer,
			types::usize offset,
			const types::u8* data,
			types::usize data_byte_size
		) override final;

		void DestroyBuffer(SGPUBufferResource& buffer) override final;

		SGPURenderTargetResource CreateRenderTarget(
			const std::vector<SGPUTextureResource>& colorAttachments,
			const SGPUTextureResource& depthAttachment
		) override final;

		void DestroyRenderTarget(SGPURenderTargetResource& renderTarget) override final;

		SGPUPipelineResource CreatePipeline(
			const SGPUShaderResource& shader,
			const std::vector<SGPUBindingGroupLayoutResource>& bindingGroupLayouts,
			types::boolean bUsePushConstants,
			const std::optional<std::reference_wrapper<SViewport>> viewport = std::nullopt,
			std::optional<std::reference_wrapper<SGPURenderTargetResource>> renderTarget = std::nullopt,
			const std::optional<std::reference_wrapper<SGPURenderPassResource>> renderPass = std::nullopt,
			ERenderPrimitiveTopology primitiveTopology = ERenderPrimitiveTopology::Unknown,
			EVertexBufferFormat vertexBufferFormat = EVertexBufferFormat::Unknown
		) override final;

		void DestroyPipeline(SGPUPipelineResource& pipeline) override final;

		SGPUBindingGroupLayoutResource CreateBindingGroupLayout(
			const std::vector<SRenderBindingGroupBinding>& bindings
		) override final;

		void DestroyBindingGroupLayout(SGPUBindingGroupLayoutResource& bindingGroupLayout) override final;

		SGPUBindingGroupResource CreateBindingGroup(
			types::s32 index,
			const SGPUBindingGroupLayoutResource& bindingGroupLayout,
			const std::vector<SRenderBindingGroupBinding>& buffersToBind,
			const std::vector<SRenderBindingGroupBinding>& texturesToBind
		) override final;

		void DestroyBindingGroup(SGPUBindingGroupResource& bindingGroup) override final;

		SGPURenderPassResource CreateRenderPass(
			SGPURenderTargetResource& renderTarget,
			types::boolean bClearRenderTarget,
			const cVector4& clearColor,
			types::f32 clearDepth,
			const std::vector<ERenderResourceUsage>& srcAttachmentsUsage,
			const std::vector<ERenderResourceUsage>& dstAttachmentsUsage
		) override final;

		void DestroyRenderPass(SGPURenderPassResource& renderPass) override final;

		SGPUShaderResource CreateShader(
			types::dword stageMask,
			const SShaderBytecodeFiles& bytecodeFiles
		) override final;

		void DestroyShader(SGPUShaderResource& shader) override final;

		void Dispatch(
			const SGPUPipelineResource& pipeline,
			const std::vector<SGPUBindingGroupResource>& binding_groups,
			const cVector3& work_group_count
		) override final;

		void ResetCommandBuffer(
			ERenderCommandBufferType command_buffer,
			ERenderCommandBufferResetOption reset_option
		) override final;

		void AddCommandToBuffer(
			ENativeRenderCommand command,
			const void* commandArgA,
			const void* commandArgB
		) override final;

		void BeginFrame() override final;

		void EndFrame() override final;

		void ExecuteImageBarrier(
			ERenderCommandBufferType command_buffer,
			const SGPUTextureResource& texture,
			ERenderPipelineStageBit src_stage_mask,
			ERenderPipelineStageBit dst_stage_mask,
			ERenderResourceAccessBit src_access_mask,
			ERenderResourceAccessBit dst_access_mask,
			ERenderImageAspectBit aspect_mask,
			ERenderImageLayout src_layout,
			ERenderImageLayout dst_layout
		);

		void ExecuteBufferBarrier(
			ERenderCommandBufferType command_buffer,
			const SGPUBufferResource& buffer,
			ERenderPipelineStageBit src_stage_mask,
			ERenderPipelineStageBit dst_stage_mask,
			ERenderResourceAccessBit src_access_mask,
			ERenderResourceAccessBit dst_access_mask
		);

		void CopyBufferToImage(
			ERenderCommandBufferType command_buffer,
			const SGPUBufferResource& buffer,
			const SGPUTextureResource& texture,
			ERenderImageAspectBit aspect_mask,
			const cVector3& offset,
			const cVector3& size
		);

		void CopyBufferToBuffer(
			ERenderCommandBufferType command_buffer,
			const SGPUBufferResource& src_buffer,
			const SGPUBufferResource& dst_buffer,
			types::usize src_offset,
			types::usize dst_offset,
			types::usize size
		);

		void SubmitCommands(ERenderCommandBufferType command_buffer);

		void WaitForFence(ERenderCommandBufferType command_buffer);

		void ResetFence(ERenderCommandBufferType command_buffer);
		
	private:
		void CreateInstance(types::boolean bEnableDebugging, const std::vector<const char*> extensions);

		void DestroyInstance();

		void CreateDebugMessenger();

		void DestroyDebugMessenger();

		void CreateSurface(SWindowResource& window);

		void DestroySurface();

		void PickPhysicalDevice(EGraphicsAdapterType deviceType);

		void CheckPhysicalDeviceProperties();

		void CheckPhysicalDeviceFeatures();

		void CheckPhysicalDeviceFeaturesVulkan13();

		void CheckPhysicalDeviceFormats();

		void FindQueueFamilies();

		void CreateLogicalDevice();

		void DestroyLogicalDevice();

		void CreateCommandPoolsAndCommandBuffers();

		void DestroyCommandPoolsAndCommandBuffers();

		void CreateSwapchain(const cVector2& size, types::usize framesInFlight);

		void DestroySwapchain();

		void CreateStagingBuffer(types::usize byte_size);

		void DestroyStagingBuffer();

		void CreateDescriptorPool();

		void DestroyDescriptorPool();

		VkSurfaceFormatKHR ChooseSwapchainSurfaceFormat(const std::vector<VkSurfaceFormatKHR>& formats);

		VkPresentModeKHR ChooseSwapchainPresentMode(const std::vector<VkPresentModeKHR>& presentModes);

		VkExtent2D ChooseSwapchainExtent(
			VkSurfaceCapabilitiesKHR capabilities,
			const cVector2& size
		);

		void GetSwapchainImages();

		void CreateSwapchainRenderTargets();

		void DestroySwapchainRenderTargets();

		void CreateSwapchainRenderPasses();

		void DestroySwapchainRenderPasses();

		void CreateSwapchainShader();

		void DestroySwapchainShader();

		void CreateSwapchainSemaphoresAndFences();

		void DestroySwapchainSemaphoresAndFences();

		void CreateStagingFence();

		void DestroyStagingFence();

		types::usize FindProperMemoryTypeIndex(
			VkPhysicalDeviceMemoryProperties memoryProperties,
			VkMemoryRequirements requirements,
			types::dword requiredMemoryPropertyBits
		);
		
		types::usize TextureFormatToChannelCount(ETextureFormat textureFormat);

		VkDeviceMemory AllocateDeviceMemory(VkMemoryRequirements requirements, types::dword requiredMemoryPropertyBits);

		VkFormat TextureFormatToNative(ETextureFormat textureFormat);

		VkImageUsageFlags TextureUsageToNative(types::dword textureUsageMask);

		VkImageType TextureDimensionToNative(ETextureDimension textureDimension);

		VkImageLayout AttachmentUsageToNativeLayout(ERenderResourceUsage attachmentUsage);

		VkPipelineBindPoint PipelineBindPointToNative(ERenderPipelineBindPoint bindPoint);

		VkViewport ViewportToNative(const SViewport& viewport);

		VkPrimitiveTopology PrimitiveTopologyToNative(ERenderPrimitiveTopology primitiveTopology);

		VkBufferUsageFlags BufferTypeToNative(EGPUBufferType bufferType);

		ERenderImageAspectBit TextureFormatToAspect(ETextureFormat format);

		VkPipelineStageFlags PipelineStageToNative(ERenderPipelineStageBit stage_mask);

		VkAccessFlags AccessToNative(ERenderResourceAccessBit access_mask);

		VkImageAspectFlags ImageAspectToNative(ERenderImageAspectBit aspect_mask);

		VkImageLayout ImageLayoutToNative(ERenderImageLayout layout);

		std::string ShaderSourceInclude(const std::string& shaderSource, const std::string& includeStr);
	};
}