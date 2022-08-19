#pragma once

#include "Platform/Vulkan/Vulkan.h"

namespace VE
{
	enum class CommandBufferState
	{
		Ready = 1,
		Recording,
		InRenderPass,
		RecordingEnded,
		Submitted,
		NotAllocated
	};

	struct CommandBufferSpecification
	{
		VkCommandPool CommandPool = VK_NULL_HANDLE;
		bool Primary = false;
	};

	class VulkanCommandBuffer : public ReferenceCount
	{
	public:
		VulkanCommandBuffer( const CommandBufferSpecification& specification );
		virtual ~VulkanCommandBuffer();

		void Allocate();
		void Free();

		void Begin( bool singleUse = false, bool renderPassContinue = false, bool simultaneousUse = false );
		void End();

		void UpdateSubmitted();
		void Reset();

		void AllocateAndBeginSingleUse();
		void EndSingleUse( VkQueue queue );

		VkCommandBuffer GetVulkanCommandBuffer()
		{
			return m_CommandBuffer;
		}

	public:
		CommandBufferState m_State = CommandBufferState::NotAllocated;

	private:
		CommandBufferSpecification m_Specification;
		VkCommandBuffer m_CommandBuffer = VK_NULL_HANDLE;

		friend class VulkanSwapChain;
	};
}