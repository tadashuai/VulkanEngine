#pragma once

#include "Core/Window.h"

#include "Events/ApplicationEvent.h"

#include <vulkan/vulkan.h>

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

	class Application
	{
	public:
		Application( const ApplicationSpecification& specification );
		virtual ~Application();

		void Run();
		void Close();

		virtual void OnEvent( Event& event );

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
		bool OnWindowResize( WindowResizeEvent& e );
		bool OnWindowClose( WindowCloseEvent& e );

	private:
		Scope<Window> m_Window;
		bool m_Running = true;
		bool m_Minimized = false;

		ApplicationSpecification m_Specification;

		static Application* s_Instance;
	};

	Application* CreateApplication( int argc, char** argv );
}
