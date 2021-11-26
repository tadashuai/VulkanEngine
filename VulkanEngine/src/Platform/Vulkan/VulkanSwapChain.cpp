#include "vepch.h"
#include "Platform/Vulkan/VulkanSwapChain.h"

#include "Renderer/Renderer.h"

namespace VE
{

	void VulkanSwapChain::Init( VkInstance instance, const Ref<VulkanLogicalDevice>& logicalDevice )
	{
		m_Instance = instance;
		m_LogicalDevice = logicalDevice;
	}

	void VulkanSwapChain::CreateSurface( GLFWwindow* window )
	{
		m_Window = window;
		glfwCreateWindowSurface( m_Instance, window, nullptr, &m_Surface );
	}

	void VulkanSwapChain::Create( uint32_t* width, uint32_t* height, bool vsync )
	{
		CreateSwapChain( width, height, vsync );
		CreateImageViews();
		CreateRenderPass();
		CreateFramebuffers();
		CreateCommandPool();
		CreateCommandBuffers();
		CreateSyncObjects();
	}

	void VulkanSwapChain::DrawFrame()
	{
		//auto& queue = Renderer::GetRenderResourceReleaseQueue( m_CurrentBufferIndex );
		//queue.Execute();

		auto logicalDevice = m_LogicalDevice->GetVulkanLogicalDevice();
		auto graphicsQueue = m_LogicalDevice->GetGraphicsQueue();

		vkWaitForFences( logicalDevice, 1, &m_WaitInFlightFences[ m_CurrentBufferIndex ], VK_TRUE, UINT64_MAX );

		VkResult result = vkAcquireNextImageKHR( logicalDevice, m_SwapChain, UINT64_MAX, m_WaitSemaphores[ m_CurrentBufferIndex ], ( VkFence )nullptr, &m_CurrentImageIndex );
		if ( result == VK_ERROR_OUT_OF_DATE_KHR )
		{
			OnResize( m_Width, m_Height );
			return;
		}
		else if ( result != VK_SUCCESS && result != VK_SUBOPTIMAL_KHR )
		{
			VE_ASSERT( false, "failed to acquire swap chain image!" );
		}

		if ( m_ImageInFlightFences[ m_CurrentImageIndex ] != VK_NULL_HANDLE )
		{
			vkWaitForFences( logicalDevice, 1, &m_ImageInFlightFences[ m_CurrentImageIndex ], VK_TRUE, UINT64_MAX );
		}
		m_ImageInFlightFences[ m_CurrentImageIndex ] = m_WaitInFlightFences[ m_CurrentBufferIndex ];

		//Renderer::WaitAndRender();

		VkSubmitInfo submitInfo{};
		submitInfo.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO;

		VkSemaphore waitSemaphores[] = { m_WaitSemaphores[ m_CurrentBufferIndex ] };
		submitInfo.waitSemaphoreCount = 1;
		submitInfo.pWaitSemaphores = waitSemaphores;

		VkPipelineStageFlags waitStages = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;
		submitInfo.pWaitDstStageMask = &waitStages;

		VkSemaphore signalSemaphores[] = { m_SignalSemaphores[ m_CurrentBufferIndex ] };
		submitInfo.signalSemaphoreCount = 1;
		submitInfo.pSignalSemaphores = signalSemaphores;

		submitInfo.commandBufferCount = 1;
		submitInfo.pCommandBuffers = &m_CommandBuffers[ m_CurrentImageIndex ];

		VK_CHECK_RESULT( vkResetFences( logicalDevice, 1, &m_WaitInFlightFences[ m_CurrentBufferIndex ] ) );
		VK_CHECK_RESULT( vkQueueSubmit( graphicsQueue, 1, &submitInfo, m_WaitInFlightFences[ m_CurrentBufferIndex ] ) );

		VkPresentInfoKHR presentInfo = {};
		presentInfo.sType = VK_STRUCTURE_TYPE_PRESENT_INFO_KHR;
		presentInfo.pNext = nullptr;
		presentInfo.waitSemaphoreCount = 1;
		presentInfo.pWaitSemaphores = signalSemaphores;
		presentInfo.swapchainCount = 1;
		presentInfo.pSwapchains = &m_SwapChain;
		presentInfo.pImageIndices = &m_CurrentImageIndex;

		result = vkQueuePresentKHR( graphicsQueue, &presentInfo );

		if ( result == VK_ERROR_OUT_OF_DATE_KHR || result == VK_SUBOPTIMAL_KHR )
		{
			OnResize( m_Width, m_Height );
		}
		else if ( result != VK_SUCCESS )
		{
			VK_CHECK_RESULT( result );
		}

		m_CurrentBufferIndex = ( m_CurrentBufferIndex + 1 ) % MAX_FRAMES_IN_FLIGHT;
		VK_CHECK_RESULT( vkWaitForFences( logicalDevice, 1, &m_WaitInFlightFences[ m_CurrentBufferIndex ], VK_TRUE, UINT64_MAX ) );
	}

	void VulkanSwapChain::OnResize( uint32_t width, uint32_t height )
	{
		auto device = m_LogicalDevice->GetVulkanLogicalDevice();

		glfwWaitEvents();
		vkDeviceWaitIdle( device );

		CleanUpSwapChain();

		CreateSwapChain( &width, &height, m_VSync );
		CreateImageViews();
		CreateRenderPass();
		CreateFramebuffers();
		CreateCommandBuffers();

		m_ImageInFlightFences.resize( m_SwapChainImages.size(), VK_NULL_HANDLE );
	}

	void VulkanSwapChain::CleanUp()
	{
		auto device = m_LogicalDevice->GetVulkanLogicalDevice();

		CleanUpSwapChain();

		for ( size_t i = 0; i < MAX_FRAMES_IN_FLIGHT; i++ )
		{
			vkDestroySemaphore( device, m_SignalSemaphores[ i ], nullptr );
			vkDestroySemaphore( device, m_WaitSemaphores[ i ], nullptr );
			vkDestroyFence( device, m_WaitInFlightFences[ i ], nullptr );
		}

		vkDestroyCommandPool( device, m_CommandPool, nullptr );
		vkDestroySurfaceKHR( m_Instance, m_Surface, nullptr );
	}

	VulkanSwapChain::SwapChainSupportDetails VulkanSwapChain::QuerySwapChainSupport( VkPhysicalDevice device )
	{
		SwapChainSupportDetails details;

		// Capabilities
		VK_CHECK_RESULT( vkGetPhysicalDeviceSurfaceCapabilitiesKHR( device, m_Surface, &details.Capabilities ) );

		// Formats
		uint32_t formatCount;
		VK_CHECK_RESULT( vkGetPhysicalDeviceSurfaceFormatsKHR( device, m_Surface, &formatCount, nullptr ) );

		VE_ASSERT( formatCount > 0 );
		details.Formats.resize( formatCount );
		VK_CHECK_RESULT( vkGetPhysicalDeviceSurfaceFormatsKHR( device, m_Surface, &formatCount, details.Formats.data() ) );

		// Present Modes
		uint32_t presentModeCount;
		VK_CHECK_RESULT( vkGetPhysicalDeviceSurfacePresentModesKHR( device, m_Surface, &presentModeCount, nullptr ) );

		VE_ASSERT( presentModeCount > 0 );
		details.PresentModes.resize( presentModeCount );
		VK_CHECK_RESULT( vkGetPhysicalDeviceSurfacePresentModesKHR( device, m_Surface, &presentModeCount, details.PresentModes.data() ) );

		return details;
	}

	VkSurfaceFormatKHR VulkanSwapChain::ChooseSwapSurfaceFormat( const std::vector<VkSurfaceFormatKHR>& formats )
	{
		if ( formats.size() == 1 && formats[ 0 ].format == VK_FORMAT_UNDEFINED )
		{
			VkSurfaceFormatKHR format{};
			format.format = VK_FORMAT_B8G8R8A8_UNORM;
			format.colorSpace = formats[ 0 ].colorSpace;
			m_SwapChainImageFormat = VK_FORMAT_B8G8R8A8_UNORM;
			return format;
		}
		else
		{
			for ( const auto& availableFormat : formats )
			{
				if ( availableFormat.format == VK_FORMAT_B8G8R8A8_UNORM )
				{
					m_SwapChainImageFormat = VK_FORMAT_B8G8R8A8_UNORM;
					return availableFormat;
				}
			}
		}

		m_SwapChainImageFormat = formats[ 0 ].format;
		return formats[ 0 ];
	}

	VkPresentModeKHR VulkanSwapChain::ChooseSwapPresentMode( const std::vector<VkPresentModeKHR>& presentModes )
	{
		if ( !m_VSync )
		{
			for ( const auto& availablePresentMode : presentModes )
			{
				if ( availablePresentMode == VK_PRESENT_MODE_MAILBOX_KHR )
				{
					return availablePresentMode;
				}
			}
		}

		return VK_PRESENT_MODE_FIFO_KHR;
	}

	VkExtent2D VulkanSwapChain::ChooseSwapExtent( const VkSurfaceCapabilitiesKHR& capabilities )
	{
		if ( capabilities.currentExtent.width != UINT32_MAX )
		{
			m_Width = capabilities.currentExtent.width;
			m_Height = capabilities.currentExtent.height;
			return capabilities.currentExtent;
		}
		else
		{
			VkExtent2D extent = { m_Width, m_Height };
			return extent;
		}
	}

	void VulkanSwapChain::CreateSwapChain( uint32_t* width, uint32_t* height, bool vsync )
	{
		m_VSync = vsync;
		m_Width = *width;
		m_Height = *height;

		auto physicalDevice = m_LogicalDevice->GetPhysicalDevice()->GetVulkanPhysicalDevice();
		auto logicalDevice = m_LogicalDevice->GetVulkanLogicalDevice();

		SwapChainSupportDetails swapChainSupport = QuerySwapChainSupport( physicalDevice );

		VkSurfaceFormatKHR surfaceFormat = ChooseSwapSurfaceFormat( swapChainSupport.Formats );
		VkPresentModeKHR presentMode = ChooseSwapPresentMode( swapChainSupport.PresentModes );
		VkExtent2D extent = ChooseSwapExtent( swapChainSupport.Capabilities );

		uint32_t imageCount = swapChainSupport.Capabilities.minImageCount + 1;
		if ( swapChainSupport.Capabilities.maxImageCount > 0 && imageCount > swapChainSupport.Capabilities.maxImageCount )
		{
			imageCount = swapChainSupport.Capabilities.maxImageCount;
		}

		VkSwapchainCreateInfoKHR createInfo{};
		createInfo.sType = VK_STRUCTURE_TYPE_SWAPCHAIN_CREATE_INFO_KHR;
		createInfo.pNext = nullptr;
		createInfo.surface = m_Surface;

		createInfo.minImageCount = imageCount;
		createInfo.imageFormat = surfaceFormat.format;
		createInfo.imageColorSpace = surfaceFormat.colorSpace;
		createInfo.imageExtent = extent;
		createInfo.imageArrayLayers = 1;
		createInfo.imageUsage = VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT;
		if ( swapChainSupport.Capabilities.supportedUsageFlags & VK_IMAGE_USAGE_TRANSFER_SRC_BIT )
		{
			createInfo.imageUsage |= VK_IMAGE_USAGE_TRANSFER_SRC_BIT;
		}
		if ( swapChainSupport.Capabilities.supportedUsageFlags & VK_IMAGE_USAGE_TRANSFER_DST_BIT )
		{
			createInfo.imageUsage |= VK_IMAGE_USAGE_TRANSFER_DST_BIT;
		}

		createInfo.imageSharingMode = VK_SHARING_MODE_EXCLUSIVE;
		createInfo.queueFamilyIndexCount = 0;
		createInfo.pQueueFamilyIndices = nullptr;

		VkSurfaceTransformFlagsKHR preTransform = swapChainSupport.Capabilities.supportedTransforms & VK_SURFACE_TRANSFORM_IDENTITY_BIT_KHR ? VK_SURFACE_TRANSFORM_IDENTITY_BIT_KHR : swapChainSupport.Capabilities.currentTransform;
		createInfo.preTransform = ( VkSurfaceTransformFlagBitsKHR )preTransform;

		VkCompositeAlphaFlagBitsKHR compositeAlpha = VK_COMPOSITE_ALPHA_OPAQUE_BIT_KHR;
		std::vector<VkCompositeAlphaFlagBitsKHR> compositeAlphaFlags
		{
			VK_COMPOSITE_ALPHA_OPAQUE_BIT_KHR,
			VK_COMPOSITE_ALPHA_PRE_MULTIPLIED_BIT_KHR,
			VK_COMPOSITE_ALPHA_POST_MULTIPLIED_BIT_KHR,
			VK_COMPOSITE_ALPHA_INHERIT_BIT_KHR,
		};
		for ( auto& compositeAlphaFlag : compositeAlphaFlags )
		{
			if ( swapChainSupport.Capabilities.supportedCompositeAlpha & compositeAlphaFlag )
			{
				compositeAlpha = compositeAlphaFlag;
				break;
			};
		}
		createInfo.compositeAlpha = compositeAlpha;
		createInfo.presentMode = presentMode;
		createInfo.clipped = VK_TRUE;

		VK_CHECK_RESULT( vkCreateSwapchainKHR( logicalDevice, &createInfo, nullptr, &m_SwapChain ) );

		VK_CHECK_RESULT( vkGetSwapchainImagesKHR( logicalDevice, m_SwapChain, &m_ImageCount, nullptr ) );
		m_SwapChainImages.resize( m_ImageCount );
		VK_CHECK_RESULT( vkGetSwapchainImagesKHR( logicalDevice, m_SwapChain, &m_ImageCount, m_SwapChainImages.data() ) );
	}

	void VulkanSwapChain::CreateImageViews()
	{
		m_SwapChainBuffers.resize( m_ImageCount );

		for ( size_t i = 0; i < m_SwapChainBuffers.size(); i++ )
		{
			m_SwapChainBuffers[ i ].Image = m_SwapChainImages[ i ];

			VkImageViewCreateInfo createInfo{};
			createInfo.sType = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO;
			createInfo.pNext = nullptr;
			createInfo.image = m_SwapChainBuffers[ i ].Image;
			createInfo.viewType = VK_IMAGE_VIEW_TYPE_2D;
			createInfo.format = m_SwapChainImageFormat;
			createInfo.components.r = VK_COMPONENT_SWIZZLE_R;
			createInfo.components.g = VK_COMPONENT_SWIZZLE_G;
			createInfo.components.b = VK_COMPONENT_SWIZZLE_B;
			createInfo.components.a = VK_COMPONENT_SWIZZLE_A;
			createInfo.subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
			createInfo.subresourceRange.baseMipLevel = 0;
			createInfo.subresourceRange.levelCount = 1;
			createInfo.subresourceRange.baseArrayLayer = 0;
			createInfo.subresourceRange.layerCount = 1;
			createInfo.flags = 0;

			VK_CHECK_RESULT( vkCreateImageView( m_LogicalDevice->GetVulkanLogicalDevice(), &createInfo, nullptr, &m_SwapChainBuffers[ i ].ImageView ) );
		}
	}

	void VulkanSwapChain::CreateRenderPass()
	{
		VkAttachmentDescription colorAttachment{};
		colorAttachment.format = m_SwapChainImageFormat;
		colorAttachment.samples = VK_SAMPLE_COUNT_1_BIT;
		colorAttachment.loadOp = VK_ATTACHMENT_LOAD_OP_CLEAR;
		colorAttachment.storeOp = VK_ATTACHMENT_STORE_OP_STORE;
		colorAttachment.stencilLoadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE;
		colorAttachment.stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
		colorAttachment.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
		colorAttachment.finalLayout = VK_IMAGE_LAYOUT_PRESENT_SRC_KHR;

		VkAttachmentReference colorAttachmentRef{};
		colorAttachmentRef.attachment = 0;
		colorAttachmentRef.layout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;

		VkSubpassDescription subpass{};
		subpass.pipelineBindPoint = VK_PIPELINE_BIND_POINT_GRAPHICS;
		subpass.colorAttachmentCount = 1;
		subpass.pColorAttachments = &colorAttachmentRef;

		VkSubpassDependency dependency{};
		dependency.srcSubpass = VK_SUBPASS_EXTERNAL;
		dependency.dstSubpass = 0;
		dependency.srcStageMask = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;
		dependency.dstStageMask = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;
		dependency.srcAccessMask = 0;
		dependency.dstAccessMask = VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT;

		VkRenderPassCreateInfo renderPassInfo{};
		renderPassInfo.sType = VK_STRUCTURE_TYPE_RENDER_PASS_CREATE_INFO;
		renderPassInfo.attachmentCount = 1;
		renderPassInfo.pAttachments = &colorAttachment;
		renderPassInfo.subpassCount = 1;
		renderPassInfo.pSubpasses = &subpass;
		renderPassInfo.dependencyCount = 1;
		renderPassInfo.pDependencies = &dependency;

		VK_CHECK_RESULT( vkCreateRenderPass( m_LogicalDevice->GetVulkanLogicalDevice(), &renderPassInfo, nullptr, &m_RenderPass ) );
	}

	void VulkanSwapChain::CreateFramebuffers()
	{
		auto logicalDevice = m_LogicalDevice->GetVulkanLogicalDevice();

		m_SwapChainFramebuffers.resize( m_SwapChainBuffers.size() );

		VkImageView attachments[ 1 ];

		VkFramebufferCreateInfo framebufferInfo{};
		framebufferInfo.sType = VK_STRUCTURE_TYPE_FRAMEBUFFER_CREATE_INFO;
		framebufferInfo.pNext = nullptr;
		framebufferInfo.renderPass = m_RenderPass;
		framebufferInfo.attachmentCount = 1;
		framebufferInfo.pAttachments = attachments;
		framebufferInfo.width = m_Width;
		framebufferInfo.height = m_Height;
		framebufferInfo.layers = 1;

		for ( size_t i = 0; i < m_SwapChainBuffers.size(); i++ )
		{
			attachments[ 0 ] = m_SwapChainBuffers[ i ].ImageView;
			VK_CHECK_RESULT( vkCreateFramebuffer( logicalDevice, &framebufferInfo, nullptr, &m_SwapChainFramebuffers[ i ] ) );
		}
	}

	void VulkanSwapChain::CreateCommandPool()
	{
		auto physicalDevice = m_LogicalDevice->GetPhysicalDevice()->GetVulkanPhysicalDevice();

		uint32_t queueFamilyCount;
		vkGetPhysicalDeviceQueueFamilyProperties( physicalDevice, &queueFamilyCount, NULL );
		VE_ASSERT( queueFamilyCount >= 1 );

		std::vector<VkQueueFamilyProperties> queueFamilies( queueFamilyCount );
		vkGetPhysicalDeviceQueueFamilyProperties( physicalDevice, &queueFamilyCount, queueFamilies.data() );

		int i = 0;
		uint32_t graphicsQueueFamilyIndex = UINT32_MAX;
		uint32_t presentQueueFamilyIndex = UINT32_MAX;
		for ( const auto& queueFamily : queueFamilies )
		{
			if ( queueFamily.queueFlags & VK_QUEUE_GRAPHICS_BIT )
				graphicsQueueFamilyIndex = i;

			VkBool32 presentSupport = false;
			vkGetPhysicalDeviceSurfaceSupportKHR( physicalDevice, i, m_Surface, &presentSupport );
			if ( presentSupport )
				presentQueueFamilyIndex = i;

			if ( graphicsQueueFamilyIndex != UINT32_MAX && presentQueueFamilyIndex != UINT32_MAX )
				break;

			i++;
		}

		VE_ASSERT( graphicsQueueFamilyIndex != UINT32_MAX );
		VE_ASSERT( presentQueueFamilyIndex != UINT32_MAX );

		VkCommandPoolCreateInfo createInfo{};
		createInfo.sType = VK_STRUCTURE_TYPE_COMMAND_POOL_CREATE_INFO;
		createInfo.queueFamilyIndex = graphicsQueueFamilyIndex;
		createInfo.flags = VK_COMMAND_POOL_CREATE_TRANSIENT_BIT | VK_COMMAND_POOL_CREATE_RESET_COMMAND_BUFFER_BIT;

		VK_CHECK_RESULT( vkCreateCommandPool( m_LogicalDevice->GetVulkanLogicalDevice(), &createInfo, nullptr, &m_CommandPool ) );
	}

	void VulkanSwapChain::CreateCommandBuffers()
	{
		m_CommandBuffers.resize( m_ImageCount );

		VkCommandBufferAllocateInfo allocInfo{};
		allocInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO;
		allocInfo.commandPool = m_CommandPool;
		allocInfo.level = VK_COMMAND_BUFFER_LEVEL_PRIMARY;
		allocInfo.commandBufferCount = ( uint32_t )m_CommandBuffers.size();

		VK_CHECK_RESULT( vkAllocateCommandBuffers( m_LogicalDevice->GetVulkanLogicalDevice(), &allocInfo, m_CommandBuffers.data() ) );

		//for ( size_t i = 0; i < m_CommandBuffers.size(); i++ )
		//{
		//	VkCommandBufferBeginInfo beginInfo{};
		//	beginInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;

		//	if ( vkBeginCommandBuffer( m_CommandBuffers[ i ], &beginInfo ) != VK_SUCCESS )
		//	{
		//		throw std::runtime_error( "failed to begin recording command buffer!" );
		//	}

		//	VkRenderPassBeginInfo renderPassInfo{};
		//	renderPassInfo.sType = VK_STRUCTURE_TYPE_RENDER_PASS_BEGIN_INFO;
		//	renderPassInfo.renderPass = renderPass;
		//	renderPassInfo.framebuffer = m_SwapChainFramebuffers[ i ];
		//	renderPassInfo.renderArea.offset = { 0, 0 };
		//	renderPassInfo.renderArea.extent = m_SwapChainExtent;

		//	VkClearValue clearColor = { {{0.0f, 0.0f, 0.0f, 1.0f}} };
		//	renderPassInfo.clearValueCount = 1;
		//	renderPassInfo.pClearValues = &clearColor;

		//	vkCmdBeginRenderPass( m_CommandBuffers[ i ], &renderPassInfo, VK_SUBPASS_CONTENTS_INLINE );

		//	vkCmdBindPipeline( m_CommandBuffers[ i ], VK_PIPELINE_BIND_POINT_GRAPHICS, m_GraphicsPipeline );

		//	vkCmdDraw( m_CommandBuffers[ i ], 3, 1, 0, 0 );

		//	vkCmdEndRenderPass( m_CommandBuffers[ i ] );

		//	if ( vkEndCommandBuffer( m_CommandBuffers[ i ] ) != VK_SUCCESS )
		//	{
		//		throw std::runtime_error( "failed to record command buffer!" );
		//	}
		//}
	}

	void VulkanSwapChain::CreateSyncObjects()
	{
		m_WaitSemaphores.resize( MAX_FRAMES_IN_FLIGHT );
		m_SignalSemaphores.resize( MAX_FRAMES_IN_FLIGHT );
		m_WaitInFlightFences.resize( MAX_FRAMES_IN_FLIGHT );
		m_ImageInFlightFences.resize( m_SwapChainImages.size(), VK_NULL_HANDLE );

		VkSemaphoreCreateInfo semaphoreInfo{};
		semaphoreInfo.sType = VK_STRUCTURE_TYPE_SEMAPHORE_CREATE_INFO;

		VkFenceCreateInfo fenceInfo{};
		fenceInfo.sType = VK_STRUCTURE_TYPE_FENCE_CREATE_INFO;
		fenceInfo.flags = VK_FENCE_CREATE_SIGNALED_BIT;

		auto logicalDevice = m_LogicalDevice->GetVulkanLogicalDevice();

		for ( size_t i = 0; i < MAX_FRAMES_IN_FLIGHT; i++ )
		{
			VK_CHECK_RESULT( vkCreateSemaphore( logicalDevice, &semaphoreInfo, nullptr, &m_WaitSemaphores[ i ] ) );
			VK_CHECK_RESULT( vkCreateSemaphore( logicalDevice, &semaphoreInfo, nullptr, &m_SignalSemaphores[ i ] ) );
			VK_CHECK_RESULT( vkCreateFence( logicalDevice, &fenceInfo, nullptr, &m_WaitInFlightFences[ i ] ) );
		}
	}

	void VulkanSwapChain::CleanUpSwapChain()
	{
		auto device = m_LogicalDevice->GetVulkanLogicalDevice();

		for ( auto framebuffer : m_SwapChainFramebuffers )
		{
			vkDestroyFramebuffer( device, framebuffer, nullptr );
		}

		vkFreeCommandBuffers( device, m_CommandPool, static_cast< uint32_t >( m_CommandBuffers.size() ), m_CommandBuffers.data() );

		vkDestroyRenderPass( device, m_RenderPass, nullptr );

		for ( uint32_t i = 0; i < m_ImageCount; i++ )
		{
			vkDestroyImageView( device, m_SwapChainBuffers[ i ].ImageView, nullptr );
		}

		vkDestroySwapchainKHR( device, m_SwapChain, nullptr );
	}

}
