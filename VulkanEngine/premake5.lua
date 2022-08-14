project "VulkanEngine"
	kind "StaticLib"
	language "C++"
	cppdialect "C++17"
	staticruntime "off"

	targetdir ("%{wks.location}/bin/" .. outputdir .. "/%{prj.name}")
	objdir ("%{wks.location}/bin-int/" .. outputdir .. "/%{prj.name}")

	pchheader "vepch.h"
	pchsource "src/vepch.cpp"

	files
	{
		"src/**.h",
		"src/**.cpp",
		"vendor/stb_image/**.h",
		"vendor/stb_image/**.cpp",
		"vendor/GLM/glm/**.hpp",
		"vendor/GLM/glm/**.inl",
		
		"vendor/VulkanMemoryAllocator/**.h",
		"vendor/VulkanMemoryAllocator/**.cpp"
	}

	defines
	{
		"_CRT_SECURE_NO_WARNINGS",
		"GLFW_INCLUDE_VULKAN"
	}

	includedirs
	{
		"src",
		"%{IncludeDir.GLFW}",
		"%{IncludeDir.GLM}",
		"%{IncludeDir.spdlog}",
		"%{IncludeDir.stb_image}",
		"%{IncludeDir.VMA}",
		"%{IncludeDir.VulkanSDK}"
	}
	
	links
	{
		"opengl32.lib",
		
		"%{Library.Vulkan}",
		"%{Library.VulkanUtils}",
		"%{Library.GLFW3}"
	}

	filter "system:windows"
		systemversion "latest"

	filter "configurations:Debug"
		defines "VE_DEBUG"
		runtime "Debug"
		symbols "on"
		
		links
		{
			"%{Library.ShaderC_Debug}",
			"%{Library.SPIRV_Cross_Debug}",
			"%{Library.SPIRV_Cross_GLSL_Debug}",
			"%{Library.SPIRV_Tools_Debug}"
		}

	filter "configurations:Release"
		defines "VE_RELEASE"
		runtime "Release"
		optimize "on"
		
		links
		{
			"%{Library.ShaderC_Release}",
			"%{Library.SPIRV_Cross_Release}",
			"%{Library.SPIRV_Cross_GLSL_Release}"
		}