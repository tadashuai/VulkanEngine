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
		m_Window->SetEventCallback( [this]( Event& e ) { return OnEvent( e ); } );
		m_Window->SetResizable( specification.Resizable );
		m_Window->SetVSync( false );
	}

	Application::~Application()
	{
		m_Window->SetEventCallback( []( Event& e ) {} );
	}

	void Application::Close()
	{
		m_Running = false;
	}

	void Application::OnEvent( Event& event )
	{
		EventDispatcher dispatcher( event );
		dispatcher.Dispatch<WindowResizeEvent>( [this]( WindowResizeEvent& e ) { return OnWindowResize( e ); } );
		dispatcher.Dispatch<WindowCloseEvent>( [this]( WindowCloseEvent& e ) { return OnWindowClose( e ); } );
	}

	bool Application::OnWindowResize( WindowResizeEvent& e )
	{
		const uint32_t width = e.GetWidth(), height = e.GetHeight();
		if ( width == 0 || height == 0 )
		{
			m_Minimized = true;
			return false;
		}
		m_Minimized = false;

		m_Window->GetSwapChain().OnResize( width, height );

		return false;
	}

	bool Application::OnWindowClose( WindowCloseEvent& e )
	{
		Close();
		return true;
	}

	void Application::Run()
	{
		while ( m_Running )
		{
			m_Window->ProcessEvents();

			if ( !m_Minimized )
			{
				m_Window->GetSwapChain().DrawFrame();
			}
		}
	}

}
