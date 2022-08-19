#pragma once

#include "Core/Application.h"

#include "Renderer/GraphicsContext.h"

namespace VE
{
	struct RendererConfig
	{
		uint32_t MaxFramesInFlight = 3;
	};

	class Renderer
	{
	public:
		static void Init();
		static void Shutdown();

		static bool DrawFrame();

		static Ref<GraphicsContext> GetContext()
		{
			return Application::Get().GetWindow().GetGraphicsContext();
		}

		static RendererConfig& GetConfig();
	};
}
