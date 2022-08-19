#include "vepch.h"
#include "Platform/Windows/WindowsWindow.h"

#include "Events/ApplicationEvent.h"
#include "Events/KeyEvent.h"
#include "Events/MouseEvent.h"

namespace VE
{

	static void GLFWErrorCallback( int error, const char* description )
	{
		VE_ERROR( "GLFW Error ({0}): {1}", error, description );
	}

	static bool s_GLFWInitialized = false;

	WindowsWindow::WindowsWindow( const WindowSpecification& specification )
		: m_Specification( specification )
	{
	}

	WindowsWindow::~WindowsWindow()
	{
		Shutdown();
	}

	void WindowsWindow::Init()
	{
		m_Data.Title = m_Specification.Title;
		m_Data.Width = m_Specification.Width;
		m_Data.Height = m_Specification.Height;
		m_Data.VSync = m_Specification.VSync;

		VE_INFO( "Creating window {0} ({1}, {2})", m_Data.Title, m_Data.Width, m_Data.Height );

		if ( !s_GLFWInitialized )
		{
			int success = glfwInit();
			VE_ASSERT( success, "Could not initialize GLFW!" );
			glfwSetErrorCallback( GLFWErrorCallback );

			s_GLFWInitialized = true;
		}

		glfwWindowHint( GLFW_CLIENT_API, GLFW_NO_API );

		m_Window = glfwCreateWindow( ( int )m_Data.Width, ( int )m_Data.Height, m_Data.Title.c_str(), nullptr, nullptr );

		glfwSetWindowUserPointer( m_Window, &m_Data );

		glfwSetWindowSizeCallback( m_Window, []( GLFWwindow* window, int width, int height )
			{
				auto& data = *( ( WindowData* )glfwGetWindowUserPointer( window ) );

				WindowResizeEvent event( ( uint32_t )width, ( uint32_t )height );
				data.EventCallback( event );
				data.Width = width;
				data.Height = height;
			} );

		glfwSetWindowCloseCallback( m_Window, []( GLFWwindow* window )
			{
				auto& data = *( ( WindowData* )glfwGetWindowUserPointer( window ) );

				WindowCloseEvent event;
				data.EventCallback( event );
			} );

		glfwSetKeyCallback( m_Window, []( GLFWwindow* window, int key, int scancode, int action, int mods )
			{
				auto& data = *( ( WindowData* )glfwGetWindowUserPointer( window ) );

				switch ( action )
				{
					case GLFW_PRESS:
						{
							KeyPressedEvent event( ( KeyCode )key );
							data.EventCallback( event );
							break;
						}
					case GLFW_RELEASE:
						{
							KeyReleasedEvent event( ( KeyCode )key );
							data.EventCallback( event );
							break;
						}
					case GLFW_REPEAT:
						{
							KeyPressedEvent event( ( KeyCode )key, true );
							data.EventCallback( event );
							break;
						}
				}
			} );

		glfwSetCharCallback( m_Window, []( GLFWwindow* window, uint32_t codepoint )
			{
				auto& data = *( ( WindowData* )glfwGetWindowUserPointer( window ) );

				KeyTypedEvent event( ( KeyCode )codepoint );
				data.EventCallback( event );
			} );

		glfwSetMouseButtonCallback( m_Window, []( GLFWwindow* window, int button, int action, int mods )
			{
				auto& data = *( ( WindowData* )glfwGetWindowUserPointer( window ) );

				switch ( action )
				{
					case GLFW_PRESS:
						{
							MouseButtonPressedEvent event( button );
							data.EventCallback( event );
							break;
						}
					case GLFW_RELEASE:
						{
							MouseButtonReleasedEvent event( button );
							data.EventCallback( event );
							break;
						}
				}
			} );

		glfwSetScrollCallback( m_Window, []( GLFWwindow* window, double xOffset, double yOffset )
			{
				auto& data = *( ( WindowData* )glfwGetWindowUserPointer( window ) );

				MouseScrolledEvent event( ( float )xOffset, ( float )yOffset );
				data.EventCallback( event );
			} );

		glfwSetCursorPosCallback( m_Window, []( GLFWwindow* window, double x, double y )
			{
				auto& data = *( ( WindowData* )glfwGetWindowUserPointer( window ) );
				MouseMovedEvent event( ( float )x, ( float )y );
				data.EventCallback( event );
			} );

		{
			int width, height;
			glfwGetWindowSize( m_Window, &width, &height );
			m_Data.Width = width;
			m_Data.Height = height;
		}
	}

	void WindowsWindow::ProcessEvents()
	{
		glfwPollEvents();
	}

	std::pair<float, float> WindowsWindow::GetWindowPos() const
	{
		int x, y;
		glfwGetWindowPos( m_Window, &x, &y );
		return { ( float )x, ( float )y };
	}

	void WindowsWindow::SetResizable( bool resizable ) const
	{
		glfwSetWindowAttrib( m_Window, GLFW_RESIZABLE, resizable ? GLFW_TRUE : GLFW_FALSE );
	}

	void WindowsWindow::SetVSync( bool enabled )
	{
		m_Data.VSync = enabled;
	}

	bool WindowsWindow::IsVSync() const
	{
		return m_Data.VSync;
	}

	void WindowsWindow::Shutdown()
	{
		glfwTerminate();
		s_GLFWInitialized = false;
	}

}
