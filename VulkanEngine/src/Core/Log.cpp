#include "vepch.h"
#include "Core/Log.h"

#include <spdlog/sinks/stdout_color_sinks.h>
#include <spdlog/sinks/basic_file_sink.h>

namespace VE
{
	std::shared_ptr<spdlog::logger> Log::s_Logger;

	void Log::Init()
	{
		std::vector<spdlog::sink_ptr> VESink =
		{
			std::make_shared<spdlog::sinks::stdout_color_sink_mt>(),
			std::make_shared<spdlog::sinks::basic_file_sink_mt>( "VulkanEngine.log", true )
		};

		VESink[ 0 ]->set_pattern( "%^[%T] %n: %v%$" );
		VESink[ 1 ]->set_pattern( "[%T] [%l] %n: %v" );

		s_Logger = std::make_shared<spdlog::logger>( "VulkanEngine", VESink.begin(), VESink.end() );
		s_Logger->set_level( spdlog::level::trace );
	}

	void Log::Shutdown()
	{
		s_Logger.reset();
		spdlog::drop_all();
	}
}
