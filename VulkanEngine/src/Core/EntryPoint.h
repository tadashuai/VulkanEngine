#pragma once

#ifdef VE_PLATFORM_WINDOWS

extern VE::Application* VE::CreateApplication( int argc, char** argv );

int main( int argc, char** argv )
{
	VE::Log::Init();
	auto app = VE::CreateApplication(argc, argv);
	app->Run();
	delete app;
	VE::Log::Shutdown();
}

#endif
