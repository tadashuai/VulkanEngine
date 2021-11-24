include "Dependencies.lua"

workspace "VulkanEngine"
	architecture "x86_64"

	startproject "VulkanEngineEditor"

	configurations
	{
		"Debug",
		"Release",
		"Dist"
	}

	flags
	{
		"MultiProcessorCompile"
	}

outputdir = "%{cfg.buildcfg}-%{cfg.system}-%{cfg.architecture}"

group "Dependencies"
	include "VulkanEngine/vendor/GLFW"
group ""

include "VulkanEngine"
include "VulkanEngineEditor"