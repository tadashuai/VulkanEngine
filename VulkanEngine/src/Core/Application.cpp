#include "vepch.h"
#include "Core/Application.h"

#include <GLFW/glfw3.h>

namespace VE
{

	Application* Application::s_Instance;

	Application::Application( const ApplicationSpecification& specification )
		: m_Specification( specification )
	{
		VE_ASSERT( !s_Instance, "Application already exists!" );
		s_Instance = this;

		WindowSpecification windowSepcification;
		windowSepcification.Title = specification.Name;
		windowSepcification.Width = specification.WindowWidth;
		windowSepcification.Height = specification.WindowHeight;
		windowSepcification.VSync = specification.VSync;
		m_Window = std::unique_ptr<Window>( Window::Create( windowSepcification ) );
		m_Window->Init();
		m_Window->SetResizable( specification.Resizable );
		m_Window->SetVSync( false );
	}

	Application::~Application()
	{
	}

	void Application::Close()
	{
		m_Running = false;
	}

	void Application::Run()
	{
		while ( m_Running )
		{
			m_Window->ProcessEvents();
		}
	}

}
