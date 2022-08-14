#pragma once

#include <spdlog/spdlog.h>

namespace VE
{
	class Log
	{
	public:
		static void Init();
		static void Shutdown();

		inline static std::shared_ptr<spdlog::logger>& GetLogger()
		{
			return s_Logger;
		}

	private:
		static std::shared_ptr<spdlog::logger> s_Logger;
	};
}

#define VE_TRACE(...)     ::VE::Log::GetLogger()->trace(__VA_ARGS__)
#define VE_INFO(...)      ::VE::Log::GetLogger()->info(__VA_ARGS__)
#define VE_WARN(...)      ::VE::Log::GetLogger()->warn(__VA_ARGS__)
#define VE_ERROR(...)     ::VE::Log::GetLogger()->error(__VA_ARGS__)
#define VE_CRITICAL(...)  ::VE::Log::GetLogger()->critical(__VA_ARGS__)
