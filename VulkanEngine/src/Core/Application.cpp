#include "vepch.h"
#include "Core/Application.h"

#include "Renderer/Renderer.h"

#include "Platform/Vulkan/VulkanSwapChain.h"

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
		m_Window = Window::Create( windowSepcification );
		m_Window->Init();
		m_Window->SetEventCallback( [this]( Event& e ) { return OnEvent( e ); } );
		m_Window->SetResizable( specification.Resizable );
		m_Window->SetVSync( false );

		Renderer::Init();
		//Renderer::WaitCommandQueueExecute();
	}

	Application::~Application()
	{
		m_Window->SetEventCallback( []( Event& e ) {} );

		Renderer::Shutdown();
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

		if ( width == m_Window->GetWidth() && height == m_Window->GetHeight() )
			return false;

		if ( width == 0 || height == 0 )
		{
			VE_INFO( "Window minimized, suspending application." );
			m_Minimized = true;
			return false;
		}

		if ( m_Minimized )
		{
			VE_INFO( "Window restored, resuming application." );
			m_Minimized = false;
		}

		Renderer::Resize( width, height );

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
				if ( !Renderer::DrawFrame() )
				{
					Close();
				}
			}
		}
	}

}
