#include "vepch.h"
#include "Renderer/Renderer.h"

#include "Core/Application.h"

#include "Platform/Vulkan/VulkanSwapChain.h"

#include <glm/glm.hpp>

namespace VE
{
	void Renderer::Init()
	{
		Renderer::GetConfig().MaxFramesInFlight = glm::min<uint32_t>( Renderer::GetConfig().MaxFramesInFlight, Application::Get().GetWindow().GetSwapChain().GetImageCount() );
	}

	void Renderer::Shutdown()
	{
	}

	bool Renderer::DrawFrame()
	{
		// TODO: create a render class like backend to let it hold the reference of swapchain and graphics context.
		if ( Application::Get().GetWindow().GetSwapChain().BeginFrame() )
		{
			if ( !Application::Get().GetWindow().GetSwapChain().EndFrame() )
			{
				VE_ERROR( "VulkanSwapChain::EndFrame() failed. Application shutting down..." );
				return false;
			}
		}
		return true;
	}

	RendererConfig& Renderer::GetConfig()
	{
		static RendererConfig config;
		return config;
	}
}