#include "vepch.h"
#include "Platform/Vulkan/VulkanSwapChain.h"

#include "Platform/Vulkan/VulkanGraphicsContext.h"

#include "Renderer/Renderer.h"

namespace VE
{
	void VulkanSwapChain::Init( const Ref<VulkanLogicalDevice>& logicalDevice )
	{
		m_LogicalDevice = logicalDevice;
	}

	void VulkanSwapChain::CreateSurface( GLFWwindow* window )
	{
		glfwCreateWindowSurface( VulkanGraphicsContext::GetVulkanInstance(), window, VulkanGraphicsContext::GetAllocator(), &m_Surface );

		auto physicalDevice = m_LogicalDevice->GetPhysicalDevice()->GetVulkanPhysicalDevice();

		uint32_t queueFamilyCount;
		vkGetPhysicalDeviceQueueFamilyProperties( physicalDevice, &queueFamilyCount, nullptr );
		VE_ASSERT( queueFamilyCount >= 1 );

		std::vector<VkQueueFamilyProperties> queueFamilies( queueFamilyCount );
		vkGetPhysicalDeviceQueueFamilyProperties( physicalDevice, &queueFamilyCount, queueFamilies.data() );

		int i = 0;
		uint32_t presentQueueFamilyIndex = -1;
		for ( const auto& queueFamily : queueFamilies )
		{
			VkBool32 presentSupport = false;
			vkGetPhysicalDeviceSurfaceSupportKHR( physicalDevice, i, m_Surface, &presentSupport );
			if ( presentSupport )
				presentQueueFamilyIndex = i;

			if ( presentQueueFamilyIndex != -1 )
				break;

			i++;
		}
		VE_ASSERT( presentQueueFamilyIndex != -1 );
		m_LogicalDevice->SetPresentFamilyIndex( presentQueueFamilyIndex );
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

	void VulkanSwapChain::Recreate( uint32_t width, uint32_t height )
	{
		// If already being recreating, do not try again.
		if ( m_IsRecreating )
		{
			VE_INFO( "VulkanSwapChain::Recreate called when already recreating. Booting." );
			return;
		}

		// Mark as recreating if the dimensions are valid.
		m_IsRecreating = true;

		auto device = m_LogicalDevice->GetVulkanLogicalDevice();

		// Wait for any operations to complete.
		glfwWaitEvents();
		vkDeviceWaitIdle( device );

		// Clear these out just in case.
		m_ImageInFlightFences.resize( m_ImageCount, VK_NULL_HANDLE );

		CleanUpSwapChain();

		CreateSwapChain( &width, &height, m_VSync );
		CreateImageViews();
		CreateRenderPass();
		CreateFramebuffers();
		CreateCommandBuffers();

		// Clear the recreating flag.
		m_IsRecreating = false;
	}

	bool VulkanSwapChain::BeginFrame()
	{
		auto logicalDevice = m_LogicalDevice->GetVulkanLogicalDevice();

		// Check if recreating swap chain and boot out.
		if ( m_IsRecreating )
		{
			VkResult result = vkDeviceWaitIdle( logicalDevice );
			if ( !VK_RESULT_IS_SUCCESS( result ) )
			{
				VE_ERROR( "VulkanSwapChain::BeginFrame vkDeviceWaitIdle (1) failed: {0}", VKResultToString( result ) );
				return false;
			}
			VE_INFO( "Recreating swapchain, booting." );
			return false;
		}

		// Wait for the execution of the current frame to complete. The fence being free will allow this one to move on.
		VkResult result = vkWaitForFences( logicalDevice, 1, &m_WaitInFlightFences[ m_CurrentFrameIndex ], VK_TRUE, UINT64_MAX );
		if ( !VK_RESULT_IS_SUCCESS( result ) )
		{
			VE_WARN( "In-flight fence wait failure!" );
			return false;
		}

		// Wait for release command queue execute
		//Renderer::WaitReleaseCommandQueueExecute( m_CurrentFrameIndex );

		// Acquire the next image from the swap chain. Pass along the semaphore that should signaled when this completes.
		// This same semaphore will later be waited on by the queue submission to ensure this image is available.
		if ( !AcquireNextImageIndex() )
		{
			return false;
		}

		m_RenderPass->GetSpecification().RenderArea.z = ( float )m_Width;
		m_RenderPass->GetSpecification().RenderArea.w = ( float )m_Height;

		return true;
	}

	bool VulkanSwapChain::EndFrame()
	{
		auto logicalDevice = m_LogicalDevice->GetVulkanLogicalDevice();
		auto graphicsQueue = m_LogicalDevice->GetGraphicsQueue();

		// Make sure the previous frame is not using this image (i.e. its fence is being waited on)
		if ( m_ImageInFlightFences[ m_CurrentImageIndex ] != VK_NULL_HANDLE )
		{
			VK_CHECK_RESULT( vkWaitForFences( logicalDevice, 1, &m_ImageInFlightFences[ m_CurrentImageIndex ], VK_TRUE, UINT64_MAX ) );
		}

		// Mark the image fence as in-use by this frame.
		m_ImageInFlightFences[ m_CurrentImageIndex ] = m_WaitInFlightFences[ m_CurrentFrameIndex ];

		// Reset the fence for use on the next frame.
		VK_CHECK_RESULT( vkResetFences( logicalDevice, 1, &m_WaitInFlightFences[ m_CurrentFrameIndex ] ) );

		// Submit the queue and wait for the operation to complete.
		// Begin queue submission
		VkSubmitInfo submitInfo{};
		submitInfo.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO;

		// Wait semaphore ensures that the operation cannot begin until the image is available.
		VkSemaphore waitSemaphores[] = { m_WaitSemaphores[ m_CurrentFrameIndex ] };
		submitInfo.waitSemaphoreCount = 1;
		submitInfo.pWaitSemaphores = waitSemaphores;

		// Each semaphore waits on the corresponding pipeline stage to complete. 1:1 ratio.
		// VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT prevents subsequent color attachment
		// writes from executing until the semaphore signals (i.e. one frame is presented at a time)
		VkPipelineStageFlags waitStages = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;
		submitInfo.pWaitDstStageMask = &waitStages;

		// The semaphore(s) to be signaled when the queue is complete.
		VkSemaphore signalSemaphores[] = { m_SignalSemaphores[ m_CurrentFrameIndex ] };
		submitInfo.signalSemaphoreCount = 1;
		submitInfo.pSignalSemaphores = signalSemaphores;

		// Command buffer(s) to be executed.
		submitInfo.commandBufferCount = 1;
		submitInfo.pCommandBuffers = &m_CommandBuffers[ m_CurrentImageIndex ];

		VkResult result = vkQueueSubmit( graphicsQueue, 1, &submitInfo, m_WaitInFlightFences[ m_CurrentFrameIndex ] );
		if ( result != VK_SUCCESS )
		{
			VE_ERROR( "vkQueueSubmit failed with result: {0}", VKResultToString( result ) );
			return false;
		}
		// End queue submission

		Present();

		return true;
	}

	void VulkanSwapChain::CleanUp()
	{
		auto device = m_LogicalDevice->GetVulkanLogicalDevice();

		glfwWaitEvents();
		vkDeviceWaitIdle( device );

		CleanUpSwapChain( true );
		m_RenderPass = nullptr;

		auto frames = Renderer::GetConfig().MaxFramesInFlight;
		for ( size_t i = 0; i < frames; i++ )
		{
			if ( m_SignalSemaphores[ i ] != VK_NULL_HANDLE )
			{
				vkDestroySemaphore( device, m_SignalSemaphores[ i ], VulkanGraphicsContext::GetAllocator() );
				m_SignalSemaphores[ i ] = VK_NULL_HANDLE;
			}
			if ( m_WaitSemaphores[ i ] != VK_NULL_HANDLE )
			{
				vkDestroySemaphore( device, m_WaitSemaphores[ i ], VulkanGraphicsContext::GetAllocator() );
				m_WaitSemaphores[ i ] = VK_NULL_HANDLE;
			}
			vkDestroyFence( device, m_WaitInFlightFences[ i ], VulkanGraphicsContext::GetAllocator() );
		}

		m_SignalSemaphores.clear();
		m_WaitSemaphores.clear();
		m_WaitInFlightFences.clear();
		m_ImageInFlightFences.clear();

		vkDestroyCommandPool( device, m_CommandPool, VulkanGraphicsContext::GetAllocator() );

		vkDestroySurfaceKHR( VulkanGraphicsContext::GetVulkanInstance(), m_Surface, VulkanGraphicsContext::GetAllocator() );
		m_Surface = nullptr;
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
				if ( availableFormat.format == VK_FORMAT_B8G8R8A8_UNORM && availableFormat.colorSpace == VK_COLOR_SPACE_SRGB_NONLINEAR_KHR ) // TODO: Check if colorSpace is right or not
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

	bool VulkanSwapChain::AcquireNextImageIndex()
	{
		auto logicalDevice = m_LogicalDevice->GetVulkanLogicalDevice();

		VkResult result = vkAcquireNextImageKHR( logicalDevice, m_SwapChain, UINT64_MAX, m_WaitSemaphores[ m_CurrentFrameIndex ], VK_NULL_HANDLE, &m_CurrentImageIndex );
		if ( result == VK_ERROR_OUT_OF_DATE_KHR )
		{
			Recreate( m_Width, m_Height );
			return false;
		}
		else if ( result != VK_SUCCESS && result != VK_SUBOPTIMAL_KHR )
		{
			VE_ASSERT( false, "failed to acquire swap chain image!" );
			return false;
		}
		return true;
	}

	void VulkanSwapChain::Present()
	{
		auto presentQueue = m_LogicalDevice->GetPresentQueue();

		VkPresentInfoKHR presentInfo = {};
		presentInfo.sType = VK_STRUCTURE_TYPE_PRESENT_INFO_KHR;
		presentInfo.pNext = nullptr;
		presentInfo.waitSemaphoreCount = 1;

		VkSemaphore signalSemaphores[] = { m_SignalSemaphores[ m_CurrentFrameIndex ] };
		presentInfo.pWaitSemaphores = signalSemaphores;

		presentInfo.swapchainCount = 1;
		presentInfo.pSwapchains = &m_SwapChain;
		presentInfo.pImageIndices = &m_CurrentImageIndex;
		presentInfo.pResults = nullptr;

		VkResult result = vkQueuePresentKHR( presentQueue, &presentInfo );
		if ( result == VK_ERROR_OUT_OF_DATE_KHR || result == VK_SUBOPTIMAL_KHR )
		{
			Recreate( m_Width, m_Height );
		}
		else if ( result != VK_SUCCESS )
		{
			VE_ASSERT( false, "Failed to present swap chain image!" );
		}

		m_CurrentFrameIndex = ( m_CurrentFrameIndex + 1 ) % Renderer::GetConfig().MaxFramesInFlight;
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
		VE_INFO( "Swapchain created preliminarily successfully." );

		VK_CHECK_RESULT( vkGetSwapchainImagesKHR( logicalDevice, m_SwapChain, &m_ImageCount, nullptr ) );
		m_SwapChainImages.resize( m_ImageCount );
		VK_CHECK_RESULT( vkGetSwapchainImagesKHR( logicalDevice, m_SwapChain, &m_ImageCount, m_SwapChainImages.data() ) );
	}

	void VulkanSwapChain::CreateImageViews()
	{
		m_RenderImages.resize( m_ImageCount );
		ImageSpecification imageSpecification = {};

		for ( size_t i = 0; i < m_RenderImages.size(); i++ )
		{
			imageSpecification.Name = "__Swapchain_Render_Image_" + std::to_string( i ) + "__";
			m_RenderImages[ i ] = Ref<VulkanImage>::Create( imageSpecification );
			auto& imageInfo = m_RenderImages[ i ]->GetImageInfo();
			imageInfo.Image = m_SwapChainImages[ i ];

			VkImageViewCreateInfo createInfo{};
			createInfo.sType = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO;
			createInfo.pNext = nullptr;
			createInfo.image = m_SwapChainImages[ i ];
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

			VK_CHECK_RESULT( vkCreateImageView( m_LogicalDevice->GetVulkanLogicalDevice(), &createInfo, VulkanGraphicsContext::GetAllocator(), &imageInfo.ImageView ) );
		}

		// Create depth attachment
		//ImageSpecification imageSpec;
		//imageSpec.Format = ImageFormat::DEPTH24STENCIL8;
		//imageSpec.Usage = ImageUsage::Attachment;
		//imageSpec.Width = m_Width;
		//imageSpec.Height = m_Height;
		//m_DepthAttachment = Ref<VulkanImage2D>::Create( imageSpec );
		//m_DepthAttachment->RecreateImmediate();
	}

	void VulkanSwapChain::CreateRenderPass()
	{
		// If m_RenderPass exists, do not recreate
		if ( m_RenderPass )
		{
			m_RenderPass->GetSpecification().RenderArea.x = 0.0f;
			m_RenderPass->GetSpecification().RenderArea.y = 0.0f;
			m_RenderPass->GetSpecification().RenderArea.z = static_cast< float >( m_Width );
			m_RenderPass->GetSpecification().RenderArea.w = static_cast< float >( m_Height );
		}
		else
		{
			RenderPassSpecification specification;
			specification.Format = m_SwapChainImageFormat;
			specification.RenderArea.x = 0.0f;
			specification.RenderArea.y = 0.0f;
			specification.RenderArea.z = ( float )m_Width;
			specification.RenderArea.w = ( float )m_Height;
			specification.ClearFlags = RenderPassClearFlags_ColourBuffer;
			specification.HasPrevRenderPass = false;
			specification.HasNextRenderPass = false;
			specification.DebugName = "SwapChain-RenderPass";
			m_RenderPass = Ref<VulkanRenderPass>::Create( specification );
		}
	}

	void VulkanSwapChain::CreateFramebuffers()
	{
		auto logicalDevice = m_LogicalDevice->GetVulkanLogicalDevice();

		m_SwapChainFramebuffers.resize( m_ImageCount );

		VkImageView attachments[ 1 ];

		VkFramebufferCreateInfo framebufferInfo{};
		framebufferInfo.sType = VK_STRUCTURE_TYPE_FRAMEBUFFER_CREATE_INFO;
		framebufferInfo.pNext = nullptr;
		framebufferInfo.renderPass = m_RenderPass->GetVulkanRenderPass();
		framebufferInfo.attachmentCount = m_RenderPass->GetAttachmentCount();
		framebufferInfo.pAttachments = attachments;
		framebufferInfo.width = m_Width;
		framebufferInfo.height = m_Height;
		framebufferInfo.layers = 1;

		for ( size_t i = 0; i < m_SwapChainFramebuffers.size(); i++ )
		{
			attachments[ 0 ] = m_RenderImages[ i ]->GetImageInfo().ImageView;
			VK_CHECK_RESULT( vkCreateFramebuffer( logicalDevice, &framebufferInfo, VulkanGraphicsContext::GetAllocator(), &m_SwapChainFramebuffers[ i ] ) );
		}
	}

	void VulkanSwapChain::CreateCommandPool()
	{
		VkCommandPoolCreateInfo createInfo{};
		createInfo.sType = VK_STRUCTURE_TYPE_COMMAND_POOL_CREATE_INFO;
		createInfo.queueFamilyIndex = m_LogicalDevice->GetPhysicalDevice()->GetQueueFamilyIndices().GraphicsFamilyIndex;
		createInfo.flags = VK_COMMAND_POOL_CREATE_TRANSIENT_BIT | VK_COMMAND_POOL_CREATE_RESET_COMMAND_BUFFER_BIT;

		VK_CHECK_RESULT( vkCreateCommandPool( m_LogicalDevice->GetVulkanLogicalDevice(), &createInfo, VulkanGraphicsContext::GetAllocator(), &m_CommandPool ) );
	}

	void VulkanSwapChain::CreateCommandBuffers()
	{
		m_CommandBuffers.resize( m_ImageCount );

		VkCommandBufferAllocateInfo allocInfo{};
		allocInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO;
		allocInfo.commandPool = m_CommandPool;
		allocInfo.level = VK_COMMAND_BUFFER_LEVEL_PRIMARY;
		allocInfo.commandBufferCount = m_ImageCount;

		VK_CHECK_RESULT( vkAllocateCommandBuffers( m_LogicalDevice->GetVulkanLogicalDevice(), &allocInfo, m_CommandBuffers.data() ) );
	}

	void VulkanSwapChain::CreateSyncObjects()
	{
		auto frames = Renderer::GetConfig().MaxFramesInFlight;
		m_WaitSemaphores.resize( frames );
		m_SignalSemaphores.resize( frames );
		m_WaitInFlightFences.resize( frames );
		m_ImageInFlightFences.resize( m_ImageCount, VK_NULL_HANDLE );

		VkSemaphoreCreateInfo semaphoreInfo{};
		semaphoreInfo.sType = VK_STRUCTURE_TYPE_SEMAPHORE_CREATE_INFO;

		VkFenceCreateInfo fenceInfo{};
		fenceInfo.sType = VK_STRUCTURE_TYPE_FENCE_CREATE_INFO;
		fenceInfo.flags = VK_FENCE_CREATE_SIGNALED_BIT;

		auto logicalDevice = m_LogicalDevice->GetVulkanLogicalDevice();

		for ( size_t i = 0; i < frames; i++ )
		{
			VK_CHECK_RESULT( vkCreateSemaphore( logicalDevice, &semaphoreInfo, VulkanGraphicsContext::GetAllocator(), &m_WaitSemaphores[ i ] ) );
			VK_CHECK_RESULT( vkCreateSemaphore( logicalDevice, &semaphoreInfo, VulkanGraphicsContext::GetAllocator(), &m_SignalSemaphores[ i ] ) );
			VK_CHECK_RESULT( vkCreateFence( logicalDevice, &fenceInfo, VulkanGraphicsContext::GetAllocator(), &m_WaitInFlightFences[ i ] ) );
		}
	}

	void VulkanSwapChain::CleanUpSwapChain( bool shutdown )
	{
		auto device = m_LogicalDevice->GetVulkanLogicalDevice();

		for ( auto framebuffer : m_SwapChainFramebuffers )
		{
			vkDestroyFramebuffer( device, framebuffer, VulkanGraphicsContext::GetAllocator() );
		}
		m_SwapChainFramebuffers.clear();

		vkFreeCommandBuffers( device, m_CommandPool, m_ImageCount, m_CommandBuffers.data() );
		m_CommandBuffers.clear();

		//if ( shutdown )
		//{
		//	m_DepthAttachment->ReleaseImmediate();
		//}
		//else
		//{
		//	m_DepthAttachment->Release(); // Renderer Shutdown before this.
		//}
		//m_DepthAttachment = nullptr;

		m_RenderPass = nullptr;

		for ( uint32_t i = 0; i < m_ImageCount; i++ )
		{
			vkDestroyImageView( device, m_RenderImages[ i ]->GetImageInfo().ImageView, VulkanGraphicsContext::GetAllocator() );
			m_RenderImages[ i ]->GetImageInfo().ImageView = VK_NULL_HANDLE;
			m_RenderImages[ i ]->GetImageInfo().Image = VK_NULL_HANDLE;
		}
		m_RenderImages.clear();

		vkDestroySwapchainKHR( device, m_SwapChain, VulkanGraphicsContext::GetAllocator() );
	}
}
