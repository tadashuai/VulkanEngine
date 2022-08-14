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

	RendererConfig& Renderer::GetConfig()
	{
		static RendererConfig config;
		return config;
	}
}