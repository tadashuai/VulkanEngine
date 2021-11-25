#include "vepch.h"
#include "Platform/Vulkan/VulkanInstance.h"

#include <GLFW/glfw3.h>

namespace VE
{

	const std::vector<const char*> validationLayers = { "VK_LAYER_KHRONOS_validation" };

	static VKAPI_ATTR VkBool32 VKAPI_CALL DebugCallback( VkDebugUtilsMessageSeverityFlagBitsEXT messageSeverity, VkDebugUtilsMessageTypeFlagsEXT messageType, const VkDebugUtilsMessengerCallbackDataEXT* pCallbackData, void* pUserData )
	{
		VE_TRACE( "Validation layer: {0}", pCallbackData->pMessage );

		return VK_FALSE;
	}

	VulkanInstance::VulkanInstance()
	{
	}

	VulkanInstance::~VulkanInstance()
	{
		m_LogicalDevice->Destroy();

		if ( s_EnableValidationLayers )
		{
			DestroyDebugUtilsMessengerEXT( s_Instance, m_DebugMessenger, nullptr );
		}

		vkDestroyInstance( s_Instance, nullptr );
		s_Instance = nullptr;
	}

	void VulkanInstance::Init()
	{
		CreateInstance();
		SetupDebugMessenger();

		m_PhysicalDevice = VulkanPhysicalDevice::Pick();

		VkPhysicalDeviceFeatures deviceFeatures{};
		deviceFeatures.samplerAnisotropy = VK_TRUE;
		deviceFeatures.wideLines = VK_TRUE;
		deviceFeatures.fillModeNonSolid = VK_TRUE;
		m_LogicalDevice = CreateRef<VulkanLogicalDevice>( m_PhysicalDevice, deviceFeatures );
	}

	void VulkanInstance::CreateInstance()
	{
		VE_ASSERT( !s_EnableValidationLayers || CheckValidationLayerSupport(), "Validation layers requested, but not available!" );

		VkApplicationInfo appInfo{};
		appInfo.sType = VK_STRUCTURE_TYPE_APPLICATION_INFO;
		appInfo.pApplicationName = "Vulkan Engine";
		appInfo.applicationVersion = VK_MAKE_VERSION( 1, 0, 0 );
		appInfo.pEngineName = "Vulkan Engine";
		appInfo.engineVersion = VK_MAKE_VERSION( 1, 0, 0 );
		appInfo.apiVersion = VK_API_VERSION_1_2;

		VkInstanceCreateInfo createInfo{};
		createInfo.sType = VK_STRUCTURE_TYPE_INSTANCE_CREATE_INFO;
		createInfo.pApplicationInfo = &appInfo;

		auto extension = GetRequiredExtensions();
		createInfo.enabledExtensionCount = static_cast< uint32_t >( extension.size() );
		createInfo.ppEnabledExtensionNames = extension.data();

		VkDebugUtilsMessengerCreateInfoEXT debugCreateInfo{};
		if ( s_EnableValidationLayers )
		{
			createInfo.enabledLayerCount = static_cast< uint32_t >( validationLayers.size() );
			createInfo.ppEnabledLayerNames = validationLayers.data();

			PopulateDebugMessengerCreateInfo( debugCreateInfo );
			createInfo.pNext = ( VkDebugUtilsMessengerCreateInfoEXT* )&debugCreateInfo;
		}
		else
		{
			createInfo.enabledLayerCount = 0;

			createInfo.pNext = nullptr;
		}

		VK_CHECK_RESULT( vkCreateInstance( &createInfo, nullptr, &s_Instance ) );
	}

	std::vector<const char*> VulkanInstance::GetRequiredExtensions()
	{
		uint32_t glfwExtensionCount = 0;
		const char** glfwExtensions;
		glfwExtensions = glfwGetRequiredInstanceExtensions( &glfwExtensionCount );

		std::vector<const char*> extensions( glfwExtensions, glfwExtensions + glfwExtensionCount );

		if ( s_EnableValidationLayers )
		{
			extensions.push_back( VK_EXT_DEBUG_UTILS_EXTENSION_NAME );
			extensions.push_back( VK_EXT_DEBUG_REPORT_EXTENSION_NAME );
		}

		return extensions;
	}

	void VulkanInstance::PopulateDebugMessengerCreateInfo( VkDebugUtilsMessengerCreateInfoEXT& createInfo )
	{
		createInfo{};
		createInfo.sType = VK_STRUCTURE_TYPE_DEBUG_UTILS_MESSENGER_CREATE_INFO_EXT;
		createInfo.messageSeverity = VK_DEBUG_UTILS_MESSAGE_SEVERITY_VERBOSE_BIT_EXT | VK_DEBUG_UTILS_MESSAGE_SEVERITY_WARNING_BIT_EXT | VK_DEBUG_UTILS_MESSAGE_SEVERITY_ERROR_BIT_EXT;
		createInfo.messageType = VK_DEBUG_UTILS_MESSAGE_TYPE_GENERAL_BIT_EXT | VK_DEBUG_UTILS_MESSAGE_TYPE_VALIDATION_BIT_EXT | VK_DEBUG_UTILS_MESSAGE_TYPE_PERFORMANCE_BIT_EXT;
		createInfo.pfnUserCallback = DebugCallback;
	}

	VkResult CreateDebugUtilsMessengerEXT( VkInstance instance, const VkDebugUtilsMessengerCreateInfoEXT* pCreateInfo, const VkAllocationCallbacks* pAllocator, VkDebugUtilsMessengerEXT* pDebugMessenger )
	{
		auto func = ( PFN_vkCreateDebugUtilsMessengerEXT )vkGetInstanceProcAddr( instance, "vkCreateDebugUtilsMessengerEXT" );
		if ( func != nullptr )
		{
			return func( instance, pCreateInfo, pAllocator, pDebugMessenger );
		}
		else
		{
			return VK_ERROR_EXTENSION_NOT_PRESENT;
		}
	}

	void VulkanInstance::SetupDebugMessenger()
	{
		if ( !s_EnableValidationLayers )
			return;

		VkDebugUtilsMessengerCreateInfoEXT createInfo;
		PopulateDebugMessengerCreateInfo( createInfo );

		VK_CHECK_RESULT( CreateDebugUtilsMessengerEXT( s_Instance, &createInfo, nullptr, &m_DebugMessenger ) );
	}

	void VulkanInstance::DestroyDebugUtilsMessengerEXT( VkInstance instance, VkDebugUtilsMessengerEXT debugMessenger, const VkAllocationCallbacks* pAllocator )
	{
		auto func = ( PFN_vkDestroyDebugUtilsMessengerEXT )vkGetInstanceProcAddr( instance, "vkDestroyDebugUtilsMessengerEXT" );
		if ( func != nullptr )
		{
			func( instance, debugMessenger, pAllocator );
		}
	}

	bool VulkanInstance::CheckValidationLayerSupport()
	{
		uint32_t layerCount;
		vkEnumerateInstanceLayerProperties( &layerCount, nullptr );

		std::vector<VkLayerProperties> availableLayers( layerCount );
		vkEnumerateInstanceLayerProperties( &layerCount, availableLayers.data() );

		for ( const char* layerName : validationLayers )
		{
			bool layerFound = false;

			for ( const auto& layerProperties : availableLayers )
			{
				if ( strcmp( layerName, layerProperties.layerName ) == 0 )
				{
					layerFound = true;
					break;
				}
			}

			if ( !layerFound )
			{
				return false;
			}
		}

		return true;
	}

}
