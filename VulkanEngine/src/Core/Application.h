#pragma once

#include "Core/Window.h"

#include <optional>

#include <vulkan/vulkan.h>

#define VK_USE_PLATFORM_WIN32_KHR
#include <GLFW/glfw3.h>
#define GLFW_EXPOSE_NATIVE_WIN32
#include <GLFW/glfw3native.h>

int main( int args, char** argv );

namespace VE
{
	struct ApplicationSpecification
	{
		std::string Name = "Vulkan Engine";
		uint32_t WindowWidth = 1600;
		uint32_t WindowHeight = 900;
		bool VSync = true;
		bool Resizable = true;
	};

	struct QueueFamilyIndices
	{
		std::optional<uint32_t> graphicsFamily;
		std::optional<uint32_t> presentFamily;

		bool isComplete()
		{
			return graphicsFamily.has_value() && presentFamily.has_value();
		}
	};

	struct SwapChainSupportDetails
	{
		VkSurfaceCapabilitiesKHR capabilities;
		std::vector<VkSurfaceFormatKHR> formats;
		std::vector<VkPresentModeKHR> presentModes;
	};

	class Application
	{
	public:
		Application( const ApplicationSpecification& specification );
		virtual ~Application();

		void Close();

		Window& GetWindow()
		{
			return *m_Window;
		}

		static Application& Get()
		{
			return *s_Instance;
		}

		const ApplicationSpecification& GetSpecification() const
		{
			return m_Specification;
		}

	private:
		void Run();

		void createInstance();
		void setupDebugMessenger();
		VkResult CreateDebugUtilsMessengerEXT( VkInstance instance, const VkDebugUtilsMessengerCreateInfoEXT* pCreateInfo, const VkAllocationCallbacks* pAllocator, VkDebugUtilsMessengerEXT* pDebugMessenger );
		void DestroyDebugUtilsMessengerEXT( VkInstance instance, VkDebugUtilsMessengerEXT debugMessenger, const VkAllocationCallbacks* pAllocator );
		void pickPhysicalDevice();
		void createLogicalDevice();
		void createSurface();
		QueueFamilyIndices findQueueFamilies( VkPhysicalDevice device );
		bool isDeviceSuitable( VkPhysicalDevice device );
		SwapChainSupportDetails querySwapChainSupport( VkPhysicalDevice device );
		VkExtent2D chooseSwapExtent( const VkSurfaceCapabilitiesKHR& capabilities );
		void createSwapChain();
		void createImageViews();
		VkShaderModule createShaderModule( const std::vector<char>& code );
		void createRenderPass();
		void createGraphicsPipeline();
		void createFramebuffers();
		void createCommandPool();
		void createCommandBuffers();
		void drawFrame();
		void createSyncObjects();
		void cleanupSwapChain();
		void recreateSwapChain();

		Scope<Window> m_Window;
		bool m_Running = true;

	public:
		bool framebufferResized = false;

	private:
		ApplicationSpecification m_Specification;
		VkInstance instance;
		VkDebugUtilsMessengerEXT debugMessenger;
		VkPhysicalDevice physicalDevice = VK_NULL_HANDLE;
		VkDevice device;
		VkSurfaceKHR surface;
		VkSwapchainKHR swapChain;
		std::vector<VkImage> swapChainImages;
		VkFormat swapChainImageFormat;
		VkExtent2D swapChainExtent;
		std::vector<VkImageView> swapChainImageViews;
		VkShaderModule vertShaderModule;
		VkShaderModule fragShaderModule;
		VkRenderPass renderPass;
		VkPipelineLayout pipelineLayout;
		VkPipeline graphicsPipeline;
		std::vector<VkFramebuffer> swapChainFramebuffers;
		VkCommandPool commandPool;
		std::vector<VkCommandBuffer> commandBuffers;
		VkSemaphore imageAvailableSemaphore;
		VkSemaphore renderFinishedSemaphore;
		std::vector<VkSemaphore> imageAvailableSemaphores;
		std::vector<VkSemaphore> renderFinishedSemaphores;
		std::vector<VkFence> inFlightFences;
		std::vector<VkFence> imagesInFlight;
		size_t currentFrame = 0;
		VkQueue graphicsQueue;
		VkQueue presentQueue;

		static Application* s_Instance;
		friend int ::main( int args, char** argv );
	};

	Application* CreateApplication( int argc, char** argv );
}
