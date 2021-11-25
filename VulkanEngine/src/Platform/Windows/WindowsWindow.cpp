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


	Window* Window::Create( const WindowSpecification& specification )
	{
		return new WindowsWindow( specification );
	}

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

		VE_INFO( "Creating window {0} ({1}, {2})", m_Specification.Title, m_Specification.Width, m_Specification.Height );

		if ( !s_GLFWInitialized )
		{
			int success = glfwInit();
			VE_ASSERT( success, "Could not initialize GLFW!" );
			glfwSetErrorCallback( GLFWErrorCallback );

			s_GLFWInitialized = true;
		}

		glfwWindowHint( GLFW_CLIENT_API, GLFW_NO_API );

		m_Window = glfwCreateWindow( ( int )m_Specification.Width, ( int )m_Specification.Height, m_Data.Title.c_str(), nullptr, nullptr );

		m_VulkanInstance = CreateRef<VulkanInstance>();
		m_VulkanInstance->Init();

		glfwSetWindowUserPointer( m_Window, &m_Data );

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
