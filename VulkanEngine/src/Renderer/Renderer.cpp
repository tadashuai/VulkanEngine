#include "vepch.h"
#include "Renderer/Renderer.h"

#include "Platform/Vulkan/VulkanBackend.h"

#include <glm/glm.hpp>

namespace VE
{
	void Renderer::Init()
	{
		VulkanBackend::Init();

		Renderer::GetConfig().MaxFramesInFlight = glm::min<uint32_t>( Renderer::GetConfig().MaxFramesInFlight, VulkanBackend::GetVulkanSwapChain()->GetImageCount() );
	}

	void Renderer::Shutdown()
	{
		VulkanBackend::Shutdown();
	}

	bool Renderer::DrawFrame()
	{
		if ( VulkanBackend::GetVulkanSwapChain()->BeginFrame() )
		{
			if ( !VulkanBackend::GetVulkanSwapChain()->EndFrame() )
			{
				VE_ERROR( "VulkanSwapChain::EndFrame() failed. Application shutting down..." );
				return false;
			}
		}
		return true;
	}

	void Renderer::Resize( uint32_t width, uint32_t height )
	{
		VulkanBackend::GetVulkanSwapChain()->Recreate( width, height );
	}

	RendererConfig& Renderer::GetConfig()
	{
		static RendererConfig config;
		return config;
	}
}