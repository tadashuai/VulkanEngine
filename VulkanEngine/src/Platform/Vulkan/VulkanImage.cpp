#include "vepch.h"
#include "Platform/Vulkan/VulkanImage.h"

#include "Platform/Vulkan/VulkanGraphicsContext.h"

namespace VE
{
	VulkanImage::VulkanImage( const ImageSpecification& specification )
		: m_Specification( specification )
	{
		VE_ASSERT( m_Specification.Width > 0 && m_Specification.Height > 0 );
	}

	VulkanImage::~VulkanImage()
	{
		Release();
	}

	void VulkanImage::Create()
	{
		VE_ASSERT( m_Specification.Width > 0 && m_Specification.Height > 0 );

		VkImageCreateInfo imageCreateInfo = {};
		imageCreateInfo.sType = VK_STRUCTURE_TYPE_IMAGE_CREATE_INFO;
		imageCreateInfo.imageType = VK_IMAGE_TYPE_2D;
		imageCreateInfo.extent.width = m_Specification.Width;
		imageCreateInfo.extent.height = m_Specification.Height;
		imageCreateInfo.extent.depth = 1;                                 // TODO: Support configurable depth.
		imageCreateInfo.mipLevels = m_Specification.Mips;                 // TODO: Support mip mapping
		imageCreateInfo.arrayLayers = m_Specification.Layers;             // TODO: Support number of layers in the image.
		imageCreateInfo.format = m_Specification.Format;
		imageCreateInfo.tiling = m_Specification.Tiling;
		imageCreateInfo.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
		imageCreateInfo.usage = m_Specification.Usage;
		imageCreateInfo.samples = VK_SAMPLE_COUNT_1_BIT;                  // TODO: Configurable sample count.
		imageCreateInfo.sharingMode = VK_SHARING_MODE_EXCLUSIVE;          // TODO: Configurable sharing mode.

		VulkanAllocator allocator( "VulkanImage" );
		m_ImageInfo.Allocation = allocator.AllocateImage( imageCreateInfo, VMA_MEMORY_USAGE_GPU_ONLY, m_ImageInfo.Image );

		if ( m_Specification.IsCreateView )
			CreateView();
	}

	void VulkanImage::CreateView()
	{
		VkImageViewCreateInfo imageViewCreateInfo = {};
		imageViewCreateInfo.sType = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO;
		imageViewCreateInfo.image = m_ImageInfo.Image;
		imageViewCreateInfo.viewType = VK_IMAGE_VIEW_TYPE_2D;
		imageViewCreateInfo.format = m_Specification.Format;
		imageViewCreateInfo.subresourceRange.aspectMask = m_Specification.AspectMask;
		imageViewCreateInfo.subresourceRange.baseMipLevel = 0;
		imageViewCreateInfo.subresourceRange.levelCount = 1; // m_Specification.Mips;
		imageViewCreateInfo.subresourceRange.baseArrayLayer = 0;
		imageViewCreateInfo.subresourceRange.layerCount = m_Specification.Layers;

		const auto device = VulkanGraphicsContext::GetCurrentDevice()->GetVulkanLogicalDevice();
		VK_CHECK_RESULT( vkCreateImageView( device, &imageViewCreateInfo, VulkanGraphicsContext::GetAllocator(), &m_ImageInfo.ImageView ) );
	}

	void VulkanImage::Release()
	{
		if ( m_ImageInfo.Image == VK_NULL_HANDLE )
			return;

		const auto device = VulkanGraphicsContext::GetCurrentDevice()->GetVulkanLogicalDevice();
		const auto vkAllocator = VulkanGraphicsContext::GetAllocator();
		if ( m_ImageInfo.ImageView )
			vkDestroyImageView( device, m_ImageInfo.ImageView, vkAllocator );

		VulkanAllocator allocator( "VulkanImage" );
		allocator.DestroyImage( m_ImageInfo.Image, m_ImageInfo.Allocation );

		m_ImageInfo.Image = VK_NULL_HANDLE;
		m_ImageInfo.ImageView = VK_NULL_HANDLE;
		m_ImageInfo.Allocation = VK_NULL_HANDLE;
	}

	void VulkanImage::InsertImageMemoryBarrier( VkCommandBuffer commandBuffer, VkImageLayout oldLayout, VkImageLayout newLayout )
	{
		auto graphicsFamilyIndex = VulkanGraphicsContext::GetCurrentDevice()->GetPhysicalDevice()->GetQueueFamilyIndices().GraphicsFamilyIndex;

		VkImageMemoryBarrier barrier = {};
		barrier.sType = VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER;
		barrier.oldLayout = oldLayout;
		barrier.newLayout = newLayout;
		barrier.srcQueueFamilyIndex = graphicsFamilyIndex;
		barrier.dstQueueFamilyIndex = graphicsFamilyIndex;
		barrier.image = m_ImageInfo.Image;
		barrier.subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT; // m_Specification.AspectMask;
		barrier.subresourceRange.baseMipLevel = 0;
		barrier.subresourceRange.levelCount = 1; // m_Specification.Mips;
		barrier.subresourceRange.baseArrayLayer = 0;
		barrier.subresourceRange.layerCount = m_Specification.Layers;

		VkPipelineStageFlags srcStageMask, dstStageMask;

		// Don't care about the old layout - transition to optimal layout (for the underlying implementation).
		if ( oldLayout == VK_IMAGE_LAYOUT_UNDEFINED && newLayout == VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL )
		{
			barrier.srcAccessMask = 0;
			barrier.dstAccessMask = VK_ACCESS_TRANSFER_WRITE_BIT;

			// Don't care what stage the pipeline is in at the start.
			srcStageMask = VK_PIPELINE_STAGE_TOP_OF_PIPE_BIT;

			// Used for copying
			dstStageMask = VK_PIPELINE_STAGE_TRANSFER_BIT;
		}
		else if ( oldLayout == VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL && newLayout == VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL )
		{
			// Transitioning from a transfer destination layout to a shader-readonly layout.
			barrier.srcAccessMask = VK_ACCESS_TRANSFER_WRITE_BIT;
			barrier.dstAccessMask = VK_ACCESS_SHADER_READ_BIT;

			// From a copying stage to...
			srcStageMask = VK_PIPELINE_STAGE_TRANSFER_BIT;

			// The fragment stage.
			dstStageMask = VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT;
		}
		else
		{
			VE_CRITICAL( "unsupported layout transition!" );
			return;
		}

		vkCmdPipelineBarrier( commandBuffer, srcStageMask, dstStageMask, 0, 0, nullptr, 0, nullptr, 1, &barrier );
	}

	void VulkanImage::CopyFromBuffer( VkCommandBuffer commandBuffer, VkBuffer buffer )
	{
		VkBufferImageCopy bufferImageCopyRegion;
		bufferImageCopyRegion.bufferOffset = 0;
		bufferImageCopyRegion.bufferRowLength = 0;
		bufferImageCopyRegion.bufferImageHeight = 0;
		bufferImageCopyRegion.imageSubresource.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT; // m_Specification.AspectMask;
		bufferImageCopyRegion.imageSubresource.mipLevel = 0;
		bufferImageCopyRegion.imageSubresource.baseArrayLayer = 0;
		bufferImageCopyRegion.imageSubresource.layerCount = m_Specification.Layers;
		bufferImageCopyRegion.imageExtent.width = m_Specification.Width;
		bufferImageCopyRegion.imageExtent.height = m_Specification.Height;
		bufferImageCopyRegion.imageExtent.depth = 1;
		vkCmdCopyBufferToImage( commandBuffer, buffer, m_ImageInfo.Image, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL, 1, &bufferImageCopyRegion );
	}

}