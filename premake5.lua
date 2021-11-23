include "Dependencies.lua"

workspace "VulkanEngine"
	architecture "x86_64"

	startproject "VulkanEngine"

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