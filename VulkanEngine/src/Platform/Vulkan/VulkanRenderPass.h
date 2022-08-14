#pragma once

#include "Core/Ref.h"
#include "Platform/Vulkan/Vulkan.h"

#include <glm/glm.hpp>

namespace VE
{
	typedef uint8_t RenderPassClearFlag;
	enum RenderPassClearFlags
	{
		RenderPassClearFlags_None = 0,
		RenderPassClearFlags_ColourBuffer = BIT( 0 ),
		RenderPassClearFlags_DepthBuffer = BIT( 1 ),
		RenderPassClearFlags_StencilBuffer = BIT( 2 )
	};

	struct RenderPassSpecification
	{
		VkFormat Format;
		glm::vec4 RenderArea = glm::vec4( 0.0f );
		glm::vec4 ClearColor = glm::vec4( 0.192f, 0.3f, 0.475f, 1.0f );
		float Depth = 1.0f;
		uint32_t Stencil = 0;
		RenderPassClearFlag ClearFlags = 0;
		bool HasPrevRenderPass = false;
		bool HasNextRenderPass = false;
		std::string DebugName;
	};

	enum class RenderPassState
	{
		Ready = 1,
		Recording,
		InRenderPass,
		RecordingEnded,
		Submitted,
		NotAllocated
	};

	class VulkanRenderPass : public ReferenceCount
	{
	public:
		VulkanRenderPass( const RenderPassSpecification& specification );
		virtual ~VulkanRenderPass();

		void Create();
		void Destroy();
		//void Begin( Ref<VulkanCommandBuffer> commandBuffer, Ref<VulkanFramebuffer> framebuffer );
		//void End( Ref<VulkanCommandBuffer> commandBuffer );

		bool HasClearFlag( RenderPassClearFlag clearFlag ) const
		{
			return m_Specification.ClearFlags & clearFlag;
		}

		VkRenderPass GetVulkanRenderPass()
		{
			return m_RenderPass;
		}
		uint32_t GetAttachmentCount()
		{
			return m_AttachmentCount;
		}

		RenderPassSpecification& GetSpecification()
		{
			return m_Specification;
		}
		const RenderPassSpecification& GetSpecification() const
		{
			return m_Specification;
		}

	private:
		VkRenderPass m_RenderPass;
		RenderPassSpecification m_Specification;
		RenderPassState m_State;

		uint32_t m_AttachmentCount = 0;
	};
}