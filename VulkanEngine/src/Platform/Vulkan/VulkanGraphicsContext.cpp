#include "vepch.h"
#include "Platform/Vulkan/VulkanGraphicsContext.h"

#include "Platform/Vulkan/VulkanAllocator.h"
#include "Platform/Vulkan/VulkanSwapChain.h"

#include "Core/Application.h"

#include <GLFW/glfw3.h>

namespace VE
{
	const std::vector<const char*> validationLayers = { "VK_LAYER_KHRONOS_validation" };

	static VKAPI_ATTR VkBool32 VKAPI_CALL DebugCallback( VkDebugUtilsMessageSeverityFlagBitsEXT messageSeverity, VkDebugUtilsMessageTypeFlagsEXT messageType, const VkDebugUtilsMessengerCallbackDataEXT* pCallbackData, void* pUserData )
	{
		switch ( messageType )
		{
			case VK_DEBUG_UTILS_MESSAGE_SEVERITY_VERBOSE_BIT_EXT:
				VE_TRACE( pCallbackData->pMessage );
				break;
			case VK_DEBUG_UTILS_MESSAGE_SEVERITY_INFO_BIT_EXT:
				VE_INFO( pCallbackData->pMessage );
				break;
			case VK_DEBUG_UTILS_MESSAGE_SEVERITY_WARNING_BIT_EXT:
				VE_WARN( pCallbackData->pMessage );
				break;
			case VK_DEBUG_UTILS_MESSAGE_SEVERITY_ERROR_BIT_EXT:
				VE_ERROR( pCallbackData->pMessage );
				break;
			default:
				break;
		}
		//VE_TRACE( "Validation layer: {0}", pCallbackData->pMessage );

		return VK_FALSE;
	}

	static VKAPI_ATTR VkBool32 VKAPI_CALL VulkanDebugReportCallback( VkDebugReportFlagsEXT flags, VkDebugReportObjectTypeEXT objectType, uint64_t object, size_t location, int32_t messageCode, const char* pLayerPrefix, const char* pMessage, void* pUserData )
	{
		VE_WARN( "VulkanDebugCallback:\n  Object Type: {0}\n  Message: {1}", objectType, pMessage );

		if ( strstr( pMessage, "CoreValidation-DrawState-InvalidImageLayout" ) )
			VE_ASSERT( false );

		return VK_FALSE;
	}

	void VulkanGraphicsContext::Init()
	{
		CreateVulkanInstance();
		SetupDebugMessenger();

		DeviceSpecification deviceSpecification; // Settable
		m_PhysicalDevice = Ref<VulkanPhysicalDevice>::Create( deviceSpecification );

		// TODO: should be config driven
		VkPhysicalDeviceFeatures deviceFeatures{};
		deviceFeatures.samplerAnisotropy = VK_TRUE;
		deviceFeatures.wideLines = VK_TRUE;
		deviceFeatures.fillModeNonSolid = VK_TRUE;
		m_LogicalDevice = Ref<VulkanLogicalDevice>::Create( m_PhysicalDevice, deviceFeatures );

		VulkanAllocator::Init();

		VkPipelineCacheCreateInfo pipelineCacheCreateInfo{};
		pipelineCacheCreateInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_CACHE_CREATE_INFO;
		VK_CHECK_RESULT( vkCreatePipelineCache( m_LogicalDevice->GetVulkanLogicalDevice(), &pipelineCacheCreateInfo, ms_Allocator, &m_PipelineCache ) );

		// TODO: initialize allocation callback
	}

	void VulkanGraphicsContext::Shutdown()
	{
		vkDestroyPipelineCache( m_LogicalDevice->GetVulkanLogicalDevice(), m_PipelineCache, ms_Allocator );

		VulkanAllocator::Shutdown();

		m_LogicalDevice->Destroy();

		if ( s_EnableValidationLayers )
		{
			DestroyDebugUtilsMessengerEXT( ms_Instance, m_DebugMessenger, ms_Allocator );
			DestroyDebugReportCallbackEXT( ms_Instance, m_DebugReportCallback, ms_Allocator );
		}

		vkDestroyInstance( ms_Instance, ms_Allocator );
		ms_Instance = VK_NULL_HANDLE;
		VE_INFO( "Vulkan Instance destroyed." );
	}

	Ref<VulkanCommandBuffer>& VulkanGraphicsContext::GetCurrentCommandBuffer()
	{
		return GetCommandBuffer( Application::Get().GetWindow().GetSwapChain().GetCurrentImageIndex() );
	}

	void VulkanGraphicsContext::CreateVulkanInstance()
	{
		VE_ASSERT( !s_EnableValidationLayers || CheckValidationLayerSupport(), "Validation layers requested, but not available!" );

		VkApplicationInfo appInfo{};
		appInfo.sType = VK_STRUCTURE_TYPE_APPLICATION_INFO;
		appInfo.pApplicationName = "Vulkan Engine"; // TODO: change to application name
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

		if ( s_EnableValidationLayers )
		{
			createInfo.enabledLayerCount = static_cast< uint32_t >( validationLayers.size() );
			createInfo.ppEnabledLayerNames = validationLayers.data();

			VkDebugUtilsMessengerCreateInfoEXT debugCreateInfo{};
			PopulateDebugMessengerCreateInfo( debugCreateInfo );
			createInfo.pNext = ( VkDebugUtilsMessengerCreateInfoEXT* )&debugCreateInfo;
		}
		else
		{
			createInfo.enabledLayerCount = 0;
			createInfo.ppEnabledLayerNames = 0;

			createInfo.pNext = nullptr;
		}

		VK_CHECK_RESULT( vkCreateInstance( &createInfo, ms_Allocator, &ms_Instance ) );
		VE_INFO( "Vulkan Instance created." );
	}

	std::vector<const char*> VulkanGraphicsContext::GetRequiredExtensions()
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

	void VulkanGraphicsContext::PopulateDebugMessengerCreateInfo( VkDebugUtilsMessengerCreateInfoEXT& createInfo )
	{
		createInfo = {};
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

	VkResult CreateDebugReportCallbackEXT( VkInstance instance, const VkDebugReportCallbackCreateInfoEXT* pCreateInfo, const VkAllocationCallbacks* pAllocator, VkDebugReportCallbackEXT* pDebugReportCallback )
	{
		auto func = ( PFN_vkCreateDebugReportCallbackEXT )vkGetInstanceProcAddr( instance, "vkCreateDebugReportCallbackEXT" );
		VE_ASSERT( func != NULL );
		return func( instance, pCreateInfo, pAllocator, pDebugReportCallback );
	}

	void VulkanGraphicsContext::SetupDebugMessenger()
	{
		if ( !s_EnableValidationLayers )
			return;

		VkDebugUtilsMessengerCreateInfoEXT createInfo;
		PopulateDebugMessengerCreateInfo( createInfo );

		VK_CHECK_RESULT( CreateDebugUtilsMessengerEXT( ms_Instance, &createInfo, ms_Allocator, &m_DebugMessenger ) );

		VkDebugReportCallbackCreateInfoEXT reportCreateInfo{};
		reportCreateInfo.sType = VK_STRUCTURE_TYPE_DEBUG_REPORT_CALLBACK_CREATE_INFO_EXT;
		reportCreateInfo.flags = VK_DEBUG_REPORT_ERROR_BIT_EXT | VK_DEBUG_REPORT_WARNING_BIT_EXT | VK_DEBUG_REPORT_PERFORMANCE_WARNING_BIT_EXT;
		reportCreateInfo.pfnCallback = VulkanDebugReportCallback;
		reportCreateInfo.pUserData = NULL;
		VK_CHECK_RESULT( CreateDebugReportCallbackEXT( ms_Instance, &reportCreateInfo, ms_Allocator, &m_DebugReportCallback ) );
	}

	void VulkanGraphicsContext::DestroyDebugUtilsMessengerEXT( VkInstance instance, VkDebugUtilsMessengerEXT debugMessenger, const VkAllocationCallbacks* pAllocator )
	{
		auto func = ( PFN_vkDestroyDebugUtilsMessengerEXT )vkGetInstanceProcAddr( instance, "vkDestroyDebugUtilsMessengerEXT" );
		if ( func != nullptr )
		{
			func( instance, debugMessenger, pAllocator );
		}
	}

	void VulkanGraphicsContext::DestroyDebugReportCallbackEXT( VkInstance instance, VkDebugReportCallbackEXT reportCallback, const VkAllocationCallbacks* pAllocator )
	{
		auto func = ( PFN_vkDestroyDebugReportCallbackEXT )vkGetInstanceProcAddr( instance, "vkDestroyDebugReportCallbackEXT" );
		if ( func != nullptr )
		{
			func( instance, reportCallback, pAllocator );
		}
	}

	bool VulkanGraphicsContext::CheckValidationLayerSupport()
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