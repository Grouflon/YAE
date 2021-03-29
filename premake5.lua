local VK_SDK_PATH = os.getenv("VK_SDK_PATH")
local TARGET_FOLDER_NAME = "%{cfg.platform}_%{cfg.buildcfg}"

workspace "yae"
	configurations { "Debug", "Release" }
	platforms { "Win32", "Win64" }

	language "C++"
	cppdialect "C++14"
	targetdir("./bin/"..TARGET_FOLDER_NAME.."/")
	objdir("./obj/"..TARGET_FOLDER_NAME.."/")

	includedirs {
		"./src/Engine/",
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
		"./lib/"..TARGET_FOLDER_NAME.."/",
	}

	defines {
		"_CRT_SECURE_NO_WARNINGS",
		"GLFW_INCLUDE_VULKAN",
		"GLM_FORCE_RADIANS",
		"GLM_FORCE_DEPTH_ZERO_TO_ONE",
	}

	filter { "platforms:Win32" }
		system "Windows"
    	architecture "x86"

	filter { "platforms:Win64" }
		system "Windows"
    	architecture "x86_64"

	filter "configurations:Debug"
		defines { "DEBUG", "YAE_DEBUG" }
		optimize "Debug"
		linkoptions { "/DEBUG:FULL" }

	filter "configurations:Release"
		defines { "NDEBUG" }
		optimize "On"

	filter "system:Windows"
		buildoptions("/w44251") -- disable class '...' needs to have dll-interface to be used by clients of class '...'

	filter {}

project "yaeLib"
	kind "SharedLib"
	targetdir("lib/"..TARGET_FOLDER_NAME.."/")
	ignoredefaultlibraries { "MSVCRT" }

	optimize "Debug"
	symbols "On"

	files { 
		"./src/Engine/**.h",
		"./src/Engine/**.cpp",

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
		"./src/Engine/00-Platform/**.cpp"
	}
	filter { "system:Windows"}
    	files {
    		"./src/Engine/00-Platform/Windows_Platform.cpp"
    	}
    	--[[links {
    		"vcruntime",
    		"msvcrt",
    		"ucrt",
    		"Gdi32",
    		"Shell32",
    	}]]--

	defines {
		"YAELIB_EXPORT",
		"MIRROR_EXPORT"
	}

	libdirs {
		"./extern/GLFW/lib/%{cfg.platform}/",
	}
	filter { "platforms:Win32" }
		libdirs {
			VK_SDK_PATH.."/Lib32/"
		}

	filter { "platforms:Win64" }
		libdirs {
			VK_SDK_PATH.."/Lib/"
		}

	filter {}

	links {
		"glfw3",
		"vulkan-1",
	}

project "yaeGame"
	kind "SharedLib"
	targetdir("lib/"..TARGET_FOLDER_NAME.."/")
	--targetdir("data/code/")
	dependson { "yaeLib" }
	optimize "Debug"
	symbols "On"

	files { 
		"./src/Game/**.h",
		"./src/Game/**.cpp",
	}

	includedirs {
		"./src/Game/",
	}

	defines {
		"YAEGAME_EXPORT"
	}

	links {
		"yaeLib",
	}

project "yae"
	kind "ConsoleApp"
	dependson { "yaeLib", "yaeGame" }
	optimize "Debug"
	symbols "On"

	files { 
		"./src/main.cpp",
	}

	links {
		"yaeLib",
	}

	postbuildcommands {
		"{COPY} ./lib/"..TARGET_FOLDER_NAME.."/yaeLib.dll ./bin/"..TARGET_FOLDER_NAME.."/",
		"{COPY} ./lib/"..TARGET_FOLDER_NAME.."/yaeGame.dll ./data/code/",
	}
