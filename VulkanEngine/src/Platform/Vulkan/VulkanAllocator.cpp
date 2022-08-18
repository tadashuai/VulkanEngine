#include "vepch.h"
#include "Platform/Vulkan/VulkanAllocator.h"

#include "Platform/Vulkan/VulkanGraphicsContext.h"

namespace VE
{
	static VmaAllocator s_Allocator;

	VulkanAllocator::VulkanAllocator( const std::string& tag )
		: m_Tag( tag )
	{
	}

	void VulkanAllocator::Init()
	{
		auto device = VulkanGraphicsContext::GetCurrentDevice();

		VmaAllocatorCreateInfo allocatorInfo = {};
		allocatorInfo.vulkanApiVersion = VK_API_VERSION_1_2;
		allocatorInfo.physicalDevice = device->GetPhysicalDevice()->GetVulkanPhysicalDevice();
		allocatorInfo.device = device->GetVulkanLogicalDevice();
		allocatorInfo.instance = VulkanGraphicsContext::GetVulkanInstance();

		vmaCreateAllocator( &allocatorInfo, &s_Allocator );
	}

	void VulkanAllocator::Shutdown()
	{
		vmaDestroyAllocator( s_Allocator );
	}

	VmaAllocation VulkanAllocator::AllocateBuffer( VkBufferCreateInfo bufferCreateInfo, VmaMemoryUsage usage, VkBuffer& outBuffer )
	{
		VmaAllocationCreateInfo createInfo = {};
		createInfo.usage = usage;

		VmaAllocation allocation;
		vmaCreateBuffer( s_Allocator, &bufferCreateInfo, &createInfo, &outBuffer, &allocation, nullptr );

		return allocation;
	}

	VmaAllocation VulkanAllocator::AllocateImage( VkImageCreateInfo imageCreateInfo, VmaMemoryUsage usage, VkImage& outImage )
	{
		VmaAllocationCreateInfo createInfo = {};
		createInfo.usage = usage;

		VmaAllocation allocation;
		vmaCreateImage( s_Allocator, &imageCreateInfo, &createInfo, &outImage, &allocation, nullptr );

		return allocation;
	}

	void VulkanAllocator::DestroyBuffer( VkBuffer buffer, VmaAllocation allocation )
	{
		VE_ASSERT( buffer );
		VE_ASSERT( allocation );
		vmaDestroyBuffer( s_Allocator, buffer, allocation );
	}

	void VulkanAllocator::DestroyImage( VkImage image, VmaAllocation allocation )
	{
		VE_ASSERT( image );
		VE_ASSERT( allocation );
		vmaDestroyImage( s_Allocator, image, allocation );
	}

	void VulkanAllocator::UnmapMemory( VmaAllocation allocation )
	{
		vmaUnmapMemory( s_Allocator, allocation );
	}

	VmaAllocator& VulkanAllocator::GetVMAAllocator()
	{
		return s_Allocator;
	}
}
