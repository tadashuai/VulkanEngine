#include "VulkanEngine.h"
#include "Core/EntryPoint.h"

namespace VE
{
	class TestbedApp : public Application
	{
	public:
		TestbedApp( const ApplicationSpecification& specification )
			: Application( specification )
		{
		}

		virtual ~TestbedApp()
		{
		}
	};

	Application* CreateApplication( int argc, char** argv )
	{
		ApplicationSpecification specification;
		specification.Name = "Vulkan Engine Testbed";
		specification.WindowWidth = 1600;
		specification.WindowHeight = 900;

		return new TestbedApp( specification);
	}
}