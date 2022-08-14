#include "vepch.h"
#include "Renderer/GraphicsContext.h"

#include "Platform/Vulkan/VulkanGraphicsContext.h"

namespace VE
{
	Ref<GraphicsContext> GraphicsContext::Create()
	{
		return Ref<VulkanGraphicsContext>::Create();
	}
}
