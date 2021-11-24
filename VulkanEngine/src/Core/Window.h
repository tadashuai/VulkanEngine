#pragma once

#include "Events/Event.h"

namespace VE
{
	struct WindowSpecification
	{
		std::string Title = "Vulkan Engine";
		uint32_t Width = 1600;
		uint32_t Height = 900;
		bool VSync = true;
	};

	class Window
	{
	public:
		using EventCallbackFn = std::function<void( Event& )>;

		virtual ~Window() = default;

		virtual void Init() = 0;
		virtual void ProcessEvents() = 0;

		virtual uint32_t GetWidth() const = 0;
		virtual uint32_t GetHeight() const = 0;
		virtual std::pair<uint32_t, uint32_t> GetSize() const = 0;
		virtual std::pair<float, float> GetWindowPos() const = 0;

		virtual void SetEventCallback( const EventCallbackFn& callback ) = 0;
		virtual void SetVSync( bool enabled ) = 0;
		virtual bool IsVSync()const = 0;
		virtual void SetResizable( bool resizable ) const = 0;

		virtual void* GetNativeWindow() const = 0;

		static Window* Create( const WindowSpecification& specification = WindowSpecification() );
	};
}
