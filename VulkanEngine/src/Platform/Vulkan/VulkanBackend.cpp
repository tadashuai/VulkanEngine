#include "vepch.h"
#include "Platform/Vulkan/VulkanBackend.h"

#include "Core/Application.h"

namespace VE
{
	static Ref<VulkanGraphicsContext> m_GraphicsContext;
	static Ref<VulkanSwapChain> m_SwapChain;

	void VulkanBackend::Init()
	{
		m_GraphicsContext = Ref<VulkanGraphicsContext>::Create();
		m_GraphicsContext->Init();

		m_SwapChain = Ref<VulkanSwapChain>::Create();
		m_SwapChain->CreateSurface();

		auto [width, height] = Application::Get().GetWindow().GetSize();
		bool isVSync = Application::Get().GetWindow().IsVSync();
		m_SwapChain->Create( width, height, isVSync );
	}

	void VulkanBackend::Shutdown()
	{
		m_SwapChain->CleanUp();
		m_SwapChain = nullptr;

		m_GraphicsContext->Shutdown();
		m_GraphicsContext = nullptr;
	}

	Ref<VulkanGraphicsContext> VulkanBackend::GetVulkanGraphicsContext()
	{
		return m_GraphicsContext;
	}

	Ref<VulkanSwapChain> VulkanBackend::GetVulkanSwapChain()
	{
		return m_SwapChain;
	}
}