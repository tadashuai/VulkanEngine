#pragma once

#include "vk_mem_alloc.h"

namespace VE
{
	class VulkanAllocator
	{
	public:
		VulkanAllocator() = default;
		VulkanAllocator( const std::string& tag );
		~VulkanAllocator() = default;

		static void Init();
		static void Shutdown();

		VmaAllocation AllocateBuffer( VkBufferCreateInfo bufferCreateInfo, VmaMemoryUsage usage, VkBuffer& outBuffer );
		VmaAllocation AllocateImage( VkImageCreateInfo imageCreateInfo, VmaMemoryUsage usage, VkImage& outImage );
		void DestroyBuffer( VkBuffer buffer, VmaAllocation allocation );
		void DestroyImage( VkImage image, VmaAllocation allocation );

		template<typename T>
		T* MapMemory( VmaAllocation allocation )
		{
			T* mappedMemory;
			vmaMapMemory( VulkanAllocator::GetVMAAllocator(), allocation, ( void** )&mappedMemory );
			return mappedMemory;
		}
		void UnmapMemory( VmaAllocation allocation );

		static VmaAllocator& GetVMAAllocator();

	private:
		std::string m_Tag;
	};
}
