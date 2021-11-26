#pragma once

#include "Core/Window.h"

#include <GLFW/glfw3.h>

#include "Platform/Vulkan/VulkanInstance.h"
#include "Platform/Vulkan/VulkanSwapChain.h"

namespace VE
{
	class WindowsWindow : public Window
	{
	public:
		WindowsWindow( const WindowSpecification& specification );
		virtual ~WindowsWindow();

		virtual void Init() override;
		virtual void ProcessEvents() override;

		inline uint32_t GetWidth() const override
		{
			return m_Data.Width;
		}
		inline uint32_t GetHeight() const override
		{
			return m_Data.Height;
		}

		virtual std::pair<uint32_t, uint32_t> GetSize() const override
		{
			return { m_Data.Width, m_Data.Height };
		}
		virtual std::pair<float, float> GetWindowPos() const override;

		virtual void SetEventCallback( const EventCallbackFn& callback ) override
		{
			m_Data.EventCallback = callback;
		}
		virtual void SetVSync( bool enabled ) override;
		virtual bool IsVSync() const override;
		virtual void SetResizable( bool resizable ) const override;

		virtual Ref<VulkanInstance> GetVulkanInstance() override
		{
			return m_VulkanInstance;
		}
		virtual VulkanSwapChain& GetSwapChain() override
		{
			return m_SwapChain;
		}

		inline void* GetNativeWindow() const override
		{
			return m_Window;
		};

	private:
		virtual void Shutdown();

	public:
		GLFWwindow* m_Window;
		WindowSpecification m_Specification;

		struct WindowData
		{
			std::string Title;
			unsigned int Width, Height;
			bool VSync;

			EventCallbackFn EventCallback;
		};

		WindowData m_Data;

		Ref<VulkanInstance> m_VulkanInstance;
		VulkanSwapChain m_SwapChain;
	};
}
