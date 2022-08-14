#include "vepch.h"
#include "Core/Window.h"

#ifdef VE_PLATFORM_WINDOWS
	#include "Platform/Windows/WindowsWindow.h"
#endif

namespace VE
{

	Scope<Window> Window::Create( const WindowSpecification& specification )
	{
		#ifdef VE_PLATFORM_WINDOWS
			return CreateScope<WindowsWindow>( specification );
		#else
			VE_ASSERT( false, "Unknown platform!" );
			return nullptr;
		#endif
	}

}
