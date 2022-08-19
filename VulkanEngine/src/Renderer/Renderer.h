#pragma once

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
		static void Resize( uint32_t width, uint32_t height );

		static RendererConfig& GetConfig();
	};
}
