local VK_SDK_PATH = os.getenv("VK_SDK_PATH")
local TARGET_FOLDER_NAME = "%{cfg.system}_%{cfg.architecture}_%{cfg.buildcfg}"

workspace "yae"
	configurations { "Debug", "Release" }
	platforms { "Win64" }

project "yae"
	kind "ConsoleApp"
	language "C++"
	cppdialect "C++14"
	targetdir("bin/"..TARGET_FOLDER_NAME.."/")
	objdir("obj/"..TARGET_FOLDER_NAME.."/")

	filter { "platforms:Win32" }
		system "Windows"
    	architecture "x86"

	filter { "platforms:Win64" }
		system "Windows"
    	architecture "x86_64"

	filter "configurations:Debug"
      defines { "DEBUG", "YAE_DEBUG" }
      symbols "On"

   	filter "configurations:Release"
      defines { "NDEBUG" }
      optimize "On"

    filter {}

	files { 
		"./src/**.h",
		"./src/**.cpp",

		"./extern/imgui/*.h",
		"./extern/imgui/*.cpp",
		"./extern/imgui/backends/imgui_impl_glfw.cpp",
		"./extern/imgui/backends/imgui_impl_glfw.h",

		"./extern/mirror/*.h",
		"./extern/mirror/*.cpp",
		"./extern/mirror/Tools/BinarySerializer.h",
		"./extern/mirror/Tools/BinarySerializer.cpp",

		"./extern/json/json.h",
	}
	removefiles {
		"./src/00-Platform/**.cpp"
	}
	filter { "system:Windows"}
    	files { "./src/00-Platform/Windows_Platform.cpp" }

	defines {
		"_CRT_SECURE_NO_WARNINGS",
		"GLFW_INCLUDE_VULKAN",
		"GLM_FORCE_RADIANS",
		"GLM_FORCE_DEPTH_ZERO_TO_ONE",
	}

	includedirs {
		"./src/",
		"./extern/GLFW/include/",
		"./extern/glm/",
		"./extern/stb/",
		"./extern/tinyobjloader/",
		"./extern/imgui/",
		"./extern/VulkanMemoryAllocator/",
		"./extern/mirror/",
		"./extern/json/",
		VK_SDK_PATH.."/include/",
	}

	libdirs {
		"./extern/GLFW/lib-vc2019/",
		VK_SDK_PATH.."/Lib/",
	}

	links {
		"glfw3",
		"vulkan-1",
	}
