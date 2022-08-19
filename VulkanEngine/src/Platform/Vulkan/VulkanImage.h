#pragma once

#include "Platform/Vulkan/VulkanAllocator.h"
#include "Platform/Vulkan/VulkanCommandBuffer.h"

namespace VE
{
	struct ImageSpecification
	{
		VkFormat Format = VK_FORMAT_R8G8B8A8_UNORM;
		VkImageTiling Tiling = VK_IMAGE_TILING_OPTIMAL;
		VkImageUsageFlags Usage = VK_IMAGE_USAGE_SAMPLED_BIT | VK_IMAGE_USAGE_TRANSFER_SRC_BIT | VK_IMAGE_USAGE_TRANSFER_DST_BIT;
		bool IsCreateView = false;
		VkImageAspectFlags AspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
		uint32_t Width = 1;
		uint32_t Height = 1;
		uint32_t Mips = 4;
		uint32_t Layers = 1;

		std::string Name = "";
	};

	struct VulkanImageInfo
	{
		VkImage Image = VK_NULL_HANDLE;
		VkImageView ImageView = VK_NULL_HANDLE;
		VmaAllocation Allocation = VK_NULL_HANDLE;
	};

	class VulkanImage : public ReferenceCount
	{
	public:
		VulkanImage( const ImageSpecification& specification );
		virtual ~VulkanImage();

		void Create();
		void CreateView();
		void Release();

		void InsertImageMemoryBarrier( Ref<VulkanCommandBuffer> commandBuffer, VkImageLayout oldLayout, VkImageLayout newLayout );
		void CopyFromBuffer( Ref<VulkanCommandBuffer> commandBuffer, VkBuffer buffer );

		uint32_t GetWidth() const
		{
			return m_Specification.Width;
		}
		uint32_t GetHeight() const
		{
			return m_Specification.Height;
		}
		float GetAspectRatio() const
		{
			return static_cast< float >( m_Specification.Width )
				/ static_cast< float >( m_Specification.Height );
		}

		ImageSpecification& GetSpecification()
		{
			return m_Specification;
		}
		const ImageSpecification& GetSpecification() const
		{
			return m_Specification;
		}

		VulkanImageInfo& GetImageInfo()
		{
			return m_ImageInfo;
		}
		const VulkanImageInfo& GetImageInfo() const
		{
			return m_ImageInfo;
		}

		uint64_t GetHash() const
		{
			return ( uint64_t )( m_ImageInfo.Image );
		}

	private:
		ImageSpecification m_Specification;
		VulkanImageInfo m_ImageInfo;
	};
}