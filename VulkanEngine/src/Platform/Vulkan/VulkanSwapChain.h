#pragma once

#include "Platform/Vulkan/Vulkan.h"
#include "Platform/Vulkan/VulkanDevice.h"

#include <GLFW/glfw3.h>

namespace VE
{
	class VulkanSwapChain
	{
	public:
		VulkanSwapChain() = default;

		void Init( VkInstance instance, const Ref<VulkanLogicalDevice>& logicalDevice );
		void CreateSurface( GLFWwindow* window );
		void Create( uint32_t* width, uint32_t* height, bool vsync );

		void DrawFrame();

		void OnResize( uint32_t width, uint32_t height );

		void CleanUp();

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

		void CreateSwapChain( uint32_t* width, uint32_t* height, bool vsync );
		void CreateImageViews();
		void CreateRenderPass();
		void CreateFramebuffers();
		void CreateCommandPool();
		void CreateCommandBuffers();
		void CreateSyncObjects();
		void CleanUpSwapChain();

	private:
		VkInstance m_Instance;
		Ref<VulkanLogicalDevice> m_LogicalDevice;

		VkSurfaceKHR m_Surface;
		GLFWwindow* m_Window;
		bool m_VSync = false;

		uint32_t m_Width = 0, m_Height = 0;

		VkSwapchainKHR m_SwapChain = nullptr;
		uint32_t m_ImageCount = 0;
		std::vector<VkImage> m_SwapChainImages;
		VkFormat m_SwapChainImageFormat;

		VkRenderPass m_RenderPass;
		std::vector<VkFramebuffer> m_SwapChainFramebuffers;

		struct SwapChainBuffer
		{
			VkImage Image;
			VkImageView ImageView;
		};
		std::vector<SwapChainBuffer> m_SwapChainBuffers;

		VkCommandPool m_CommandPool = nullptr;
		std::vector<VkCommandBuffer> m_CommandBuffers;

		std::vector<VkSemaphore> m_WaitSemaphores;
		std::vector<VkSemaphore> m_SignalSemaphores;
		std::vector<VkFence> m_WaitInFlightFences;
		std::vector<VkFence> m_ImageInFlightFences;

		uint32_t m_CurrentBufferIndex = 0;
		uint32_t m_CurrentImageIndex = 0;
	};
}
