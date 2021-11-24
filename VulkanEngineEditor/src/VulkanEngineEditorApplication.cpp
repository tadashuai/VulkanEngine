#include "VulkanEngine.h"
#include "Core/EntryPoint.h"

class VulkanEngineEditorApplication : public VE::Application
{
public:
	VulkanEngineEditorApplication( const VE::ApplicationSpecification& specification )
		: Application( specification )
	{
	}

	~VulkanEngineEditorApplication()
	{
	}
};

VE::Application* VE::CreateApplication( int argc, char** argv )
{
	VE::ApplicationSpecification specification;
	specification.Name = "Vulkan Engine Editor";
	specification.WindowWidth = 1600;
	specification.WindowHeight = 900;
	specification.VSync = true;
	specification.Resizable = true;

	return new VulkanEngineEditorApplication( specification );
}
