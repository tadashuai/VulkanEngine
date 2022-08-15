#include "vepch.h"
#include "Platform/Vulkan/VulkanRenderPass.h"

#include "Platform/Vulkan/VulkanGraphicsContext.h"

namespace VE
{
	VulkanRenderPass::VulkanRenderPass( const RenderPassSpecification& specification )
		: m_Specification( specification )
	{
		Create();
	}

	VulkanRenderPass::~VulkanRenderPass()
	{
		Destroy();
	}

	void VulkanRenderPass::Create()
	{
		auto device = VulkanGraphicsContext::GetCurrentDevice();

		// Main subpass
		VkSubpassDescription subpass{};
		subpass.pipelineBindPoint = VK_PIPELINE_BIND_POINT_GRAPHICS;

		// Attachments
		std::vector<VkAttachmentDescription> attachments;

		// Color Attachment
		VkAttachmentDescription& colorAttachment = attachments.emplace_back();
		colorAttachment.format = m_Specification.Format;
		colorAttachment.samples = VK_SAMPLE_COUNT_1_BIT;
		colorAttachment.loadOp = HasClearFlag( RenderPassClearFlags_ColourBuffer ) ? VK_ATTACHMENT_LOAD_OP_CLEAR : VK_ATTACHMENT_LOAD_OP_LOAD;
		colorAttachment.storeOp = VK_ATTACHMENT_STORE_OP_STORE;
		colorAttachment.stencilLoadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE;
		colorAttachment.stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
		colorAttachment.initialLayout = m_Specification.HasPrevRenderPass ? VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL : VK_IMAGE_LAYOUT_UNDEFINED;
		colorAttachment.finalLayout = m_Specification.HasNextRenderPass ? VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL : VK_IMAGE_LAYOUT_PRESENT_SRC_KHR;
		colorAttachment.flags = 0;

		VkAttachmentReference colorAttachmentRef{};
		colorAttachmentRef.attachment = 0;	// Attachment description array index
		colorAttachmentRef.layout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;

		subpass.colorAttachmentCount = 1;
		subpass.pColorAttachments = &colorAttachmentRef;

		// Depth Attachment
		bool hasDepthBuffer = HasClearFlag( RenderPassClearFlags_DepthBuffer );
		if ( hasDepthBuffer )
		{
			VkAttachmentDescription& depthAttachment = attachments.emplace_back();
			depthAttachment.format = device->GetPhysicalDevice()->GetDepthFormat();
			depthAttachment.samples = VK_SAMPLE_COUNT_1_BIT;
			depthAttachment.loadOp = hasDepthBuffer ? VK_ATTACHMENT_LOAD_OP_CLEAR : VK_ATTACHMENT_LOAD_OP_LOAD;
			depthAttachment.storeOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
			depthAttachment.stencilLoadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE;
			depthAttachment.stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
			depthAttachment.initialLayout = m_Specification.HasPrevRenderPass ? VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL : VK_IMAGE_LAYOUT_UNDEFINED;
			depthAttachment.finalLayout = m_Specification.HasNextRenderPass ? VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL : VK_IMAGE_LAYOUT_DEPTH_STENCIL_READ_ONLY_OPTIMAL;

			VkAttachmentReference depthAttachmentRef{};
			depthAttachmentRef.attachment = 1;
			depthAttachmentRef.layout = VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL;

			subpass.pDepthStencilAttachment = &depthAttachmentRef;
		}
		else
		{
			subpass.pDepthStencilAttachment = nullptr;
		}

		// TODO: other attachment types (inputs, resolve, preserve)

		// Input from a shader
		subpass.inputAttachmentCount = 0;
		subpass.pInputAttachments = 0;

		// Attachments used for multi sampling color attachments
		subpass.pResolveAttachments = 0;

		// Attachments not used in this	subpass, but must be preserved for the next
		subpass.preserveAttachmentCount = 0;
		subpass.pPreserveAttachments = 0;

		// Render pass dependencies. TODO: make this configurable
		VkSubpassDependency dependency{};
		dependency.srcSubpass = VK_SUBPASS_EXTERNAL;
		dependency.dstSubpass = 0;
		dependency.srcStageMask = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;
		dependency.dstStageMask = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;
		dependency.srcAccessMask = 0;
		dependency.dstAccessMask = VK_ACCESS_COLOR_ATTACHMENT_READ_BIT | VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT;
		dependency.dependencyFlags = 0;

		m_AttachmentCount = static_cast< uint32_t >( attachments.size() );

		// Render pass create
		VkRenderPassCreateInfo renderPassInfo{};
		renderPassInfo.sType = VK_STRUCTURE_TYPE_RENDER_PASS_CREATE_INFO;
		renderPassInfo.attachmentCount = m_AttachmentCount;
		renderPassInfo.pAttachments = attachments.data();
		renderPassInfo.subpassCount = 1;
		renderPassInfo.pSubpasses = &subpass;
		renderPassInfo.dependencyCount = 1;
		renderPassInfo.pDependencies = &dependency;
		renderPassInfo.pNext = 0;
		renderPassInfo.flags = 0;

		VK_CHECK_RESULT( vkCreateRenderPass( device->GetVulkanLogicalDevice(), &renderPassInfo, VulkanGraphicsContext::GetAllocator(), &m_RenderPass ) );
	}

	void VulkanRenderPass::Destroy()
	{
		if ( m_RenderPass )
		{
			auto device = VulkanGraphicsContext::GetCurrentDevice()->GetVulkanLogicalDevice();
			vkDestroyRenderPass( device, m_RenderPass, VulkanGraphicsContext::GetAllocator() );
			m_RenderPass = nullptr;
			m_AttachmentCount = 0;
		}
	}
}