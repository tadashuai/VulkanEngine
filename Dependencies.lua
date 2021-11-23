
-- Vulkan Engine Dependencies

VULKAN_SDK = os.getenv("VULKAN_SDK")

-- Include directories relative to root folder (solution directory)
IncludeDir = {}
IncludeDir["GLFW"]			= "%{wks.location}/VulkanEngine/vendor/GLFW/include"
IncludeDir["GLM"]			= "%{wks.location}/VulkanEngine/vendor/GLM"
IncludeDir["VulkanSDK"]		= "%{VULKAN_SDK}/Include"

LibraryDir = {}

LibraryDir["VulkanSDK"]				= "%{VULKAN_SDK}/Lib"
LibraryDir["VulkanSDK_Debug"]		= "%{wks.location}/VulkanEngine/vendor/VulkanSDK/Lib"
LibraryDir["VulkanSDK_DebugDLL"]	= "%{wks.location}/VulkanEngine/vendor/VulkanSDK/Bin"

LibraryDir["GLFW3Lib"]				= "F:/Documents/Visual Studio/Libraries/glfw-3.3.5.bin.WIN64/lib-vc2019"

Library = {}
Library["Vulkan"]					= "%{LibraryDir.VulkanSDK}/vulkan-1.lib"
Library["VulkanUtils"]				= "%{LibraryDir.VulkanSDK}/VkLayer_utils.lib"

Library["ShaderC_Debug"]			= "%{LibraryDir.VulkanSDK_Debug}/shaderc_sharedd.lib"
Library["SPIRV_Cross_Debug"]		= "%{LibraryDir.VulkanSDK_Debug}/spirv-cross-cored.lib"
Library["SPIRV_Cross_GLSL_Debug"]	= "%{LibraryDir.VulkanSDK_Debug}/spirv-cross-glsld.lib"
Library["SPIRV_Tools_Debug"]		= "%{LibraryDir.VulkanSDK_Debug}/SPIRV-Toolsd.lib"

Library["ShaderC_Release"]			= "%{LibraryDir.VulkanSDK}/shaderc_shared.lib"
Library["SPIRV_Cross_Release"]		= "%{LibraryDir.VulkanSDK}/spirv-cross-core.lib"
Library["SPIRV_Cross_GLSL_Release"]	= "%{LibraryDir.VulkanSDK}/spirv-cross-glsl.lib"

Library["GLFW3"]					= "%{LibraryDir.GLFW3Lib}/glfw3.lib"