#include "vepch.h"
#include "Platform/Vulkan/VulkanCommandBuffer.h"

#include "Platform/Vulkan/VulkanGraphicsContext.h"

namespace VE
{
	VulkanCommandBuffer::VulkanCommandBuffer( const CommandBufferSpecification& specification )
		: m_Specification( specification )
	{
	}

	VulkanCommandBuffer::~VulkanCommandBuffer()
	{
		Free();
	}

	void VulkanCommandBuffer::Allocate()
	{
		VkCommandBufferAllocateInfo allocateInfo = { VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO };
		allocateInfo.commandPool = m_Specification.CommandPool;
		allocateInfo.level = m_Specification.Primary ? VK_COMMAND_BUFFER_LEVEL_PRIMARY : VK_COMMAND_BUFFER_LEVEL_SECONDARY;
		allocateInfo.commandBufferCount = 1;
		allocateInfo.pNext = 0;

		m_State = CommandBufferState::NotAllocated;
		const auto device = VulkanGraphicsContext::GetCurrentDevice()->GetVulkanLogicalDevice();
		VK_CHECK_RESULT( vkAllocateCommandBuffers( device, &allocateInfo, &m_CommandBuffer ) );
		m_State = CommandBufferState::Ready;
	}

	void VulkanCommandBuffer::Free()
	{
		if ( m_CommandBuffer == VK_NULL_HANDLE )
			return;

		const auto device = VulkanGraphicsContext::GetCurrentDevice()->GetVulkanLogicalDevice();
		vkFreeCommandBuffers( device, m_Specification.CommandPool, 1, &m_CommandBuffer );

		m_CommandBuffer = VK_NULL_HANDLE;
		m_State = CommandBufferState::NotAllocated;
	}

	void VulkanCommandBuffer::Begin( bool singleUse /*= false*/, bool renderPassContinue /*= false*/, bool simultaneousUse /*= false*/ )
	{
		VkCommandBufferBeginInfo beginInfo = {};
		beginInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;

		beginInfo.flags = 0;
		if ( singleUse )
			beginInfo.flags |= VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT;
		if ( renderPassContinue )
			beginInfo.flags |= VK_COMMAND_BUFFER_USAGE_RENDER_PASS_CONTINUE_BIT;
		if ( simultaneousUse )
			beginInfo.flags |= VK_COMMAND_BUFFER_USAGE_SIMULTANEOUS_USE_BIT;


		VK_CHECK_RESULT( vkBeginCommandBuffer( m_CommandBuffer, &beginInfo ) );
		m_State = CommandBufferState::Recording;
	}

	void VulkanCommandBuffer::End()
	{
		VK_CHECK_RESULT( vkEndCommandBuffer( m_CommandBuffer ) );
		m_State = CommandBufferState::RecordingEnded;
	}

	void VulkanCommandBuffer::UpdateSubmitted()
	{
		m_State = CommandBufferState::Recording;
	}

	void VulkanCommandBuffer::Reset()
	{
		m_State = CommandBufferState::Ready;
	}

	void VulkanCommandBuffer::AllocateAndBeginSingleUse()
	{
		Allocate();
		Begin( true, false, false );
	}

	void VulkanCommandBuffer::EndSingleUse( VkQueue queue )
	{
		// End the command buffer.
		End();

		// Submit the queue
		VkSubmitInfo submitInfo = { VK_STRUCTURE_TYPE_SUBMIT_INFO };
		submitInfo.commandBufferCount = 1;
		submitInfo.pCommandBuffers = &m_CommandBuffer;
		VK_CHECK_RESULT( vkQueueSubmit( queue, 1, &submitInfo, 0 ) );

		// Wait for it to finish
		VK_CHECK_RESULT( vkQueueWaitIdle( queue ) );

		// Free the command buffer.
		Free();
	}
}