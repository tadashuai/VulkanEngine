#pragma once

#include "Core/Window.h"

#include <vulkan/vulkan.h>

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

		Scope<Window> m_Window;
		bool m_Running = true;

	private:
		ApplicationSpecification m_Specification;
		VkInstance instance;

		static Application* s_Instance;
		friend int ::main( int args, char** argv );
	};

	Application* CreateApplication( int argc, char** argv );
}
