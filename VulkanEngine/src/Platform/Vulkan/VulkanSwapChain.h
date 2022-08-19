#pragma once

#include "Platform/Vulkan/VulkanDevice.h"
#include "Platform/Vulkan/VulkanRenderPass.h"
#include "Platform/Vulkan/VulkanImage.h"

#include <GLFW/glfw3.h>

namespace VE
{
	class VulkanSwapChain : public ReferenceCount
	{
	public:
		VulkanSwapChain() = default;

		void Init( const Ref<VulkanLogicalDevice>& logicalDevice );
		void CreateSurface( GLFWwindow* window );
		void Create( uint32_t* width, uint32_t* height, bool vsync );
		void Recreate( uint32_t width, uint32_t height );

		bool BeginFrame();
		bool EndFrame();

		void CleanUp();

		uint32_t GetImageCount() const
		{
			return m_ImageCount;
		}

		uint32_t GetWidth() const
		{
			return m_Width;
		}
		uint32_t GetHeight() const
		{
			return m_Height;
		}

		Ref<VulkanRenderPass> GetRenderPass()
		{
			return m_RenderPass;
		}

		uint32_t GetCurrentFrameIndex() const
		{
			return m_CurrentFrameIndex;
		}
		uint32_t GetCurrentImageIndex() const
		{
			return m_CurrentImageIndex;
		}

		VkFramebuffer GetFramebuffer( uint32_t index )
		{
			VE_ASSERT( index < m_SwapChainFramebuffers.size() );
			return m_SwapChainFramebuffers[ index ];
		}
		VkFramebuffer GetCurrentFramebuffer()
		{
			return GetFramebuffer( m_CurrentImageIndex );
		}

	private:
		struct SwapChainSupportDetails
		{
			VkSurfaceCapabilitiesKHR Capabilities;
			std::vector<VkSurfaceFormatKHR> Formats;
			std::vector<VkPresentModeKHR> PresentModes;
		};

		SwapChainSupportDetails QuerySwapChainSupport( VkPhysicalDevice device );
		VkSurfaceFormatKHR ChooseSwapSurfaceFormat( const std::vector<VkSurfaceFormatKHR>& formats );
		VkPresentModeKHR ChooseSwapPresentMode( const std::vector<VkPresentModeKHR>& presentModes );
		VkExtent2D ChooseSwapExtent( const VkSurfaceCapabilitiesKHR& capabilities );

		bool AcquireNextImageIndex();
		void Present();

		void CreateSwapChain( uint32_t* width, uint32_t* height, bool vsync );
		void CreateImageViews();
		void CreateRenderPass();
		void CreateFramebuffers();
		void CreateCommandBuffers();
		void CreateSyncObjects();
		void CleanUpSwapChain( bool shutdown = false );

	private:
		Ref<VulkanLogicalDevice> m_LogicalDevice;

		VkSurfaceKHR m_Surface;
		uint32_t m_Width = 0, m_Height = 0;
		bool m_VSync = false;
		bool m_IsRecreating = false;

		VkSwapchainKHR m_SwapChain = nullptr;
		uint32_t m_ImageCount = 0;
		std::vector<VkImage> m_SwapChainImages;
		VkFormat m_SwapChainImageFormat;

		Ref<VulkanRenderPass> m_RenderPass;
		std::vector<VkFramebuffer> m_SwapChainFramebuffers;

		std::vector<Ref<VulkanImage>> m_RenderImages;

		//Ref<VulkanImage> m_DepthAttachment;

		std::vector<VkSemaphore> m_WaitSemaphores;
		std::vector<VkSemaphore> m_SignalSemaphores;
		std::vector<VkFence> m_WaitInFlightFences;
		std::vector<VkFence> m_ImageInFlightFences;

		uint32_t m_CurrentFrameIndex = 0;
		uint32_t m_CurrentImageIndex = 0;
	};
}
