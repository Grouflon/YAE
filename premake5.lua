local VK_SDK_PATH = os.getenv("VK_SDK_PATH")
local TARGET_FOLDER_NAME = "%{cfg.platform}_%{cfg.buildcfg}"

workspace "yaeApplication"
	configurations { "Debug", "Release" }
	platforms { "Win32", "Win64" }

	language "C++"
	cppdialect "C++14"
	targetdir("./bin/"..TARGET_FOLDER_NAME.."/")
	objdir("./obj/"..TARGET_FOLDER_NAME.."/")

	includedirs {
		"./src/",
		"./extern/GLFW/include/",
		"./extern/imgui/",
		"./extern/glm/",
		"./extern/VulkanMemoryAllocator/",
		"./extern/",
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
		defines { "DEBUG", "_DEBUG", "YAE_DEBUG" }
		optimize "Debug"
		linkoptions { "/DEBUG:FULL" }

	filter "configurations:Release"
		defines { "NDEBUG" }
		optimize "On"

	filter "system:Windows"
		buildoptions("/w44251") -- disable class '...' needs to have dll-interface to be used by clients of class '...'

	filter {}

project "yae"
	kind "SharedLib"
	targetdir("lib/"..TARGET_FOLDER_NAME.."/")
	ignoredefaultlibraries { "MSVCRT" }

	optimize "Debug"
	symbols "On"

	files { 
		"./src/yae/**.h",
		"./src/yae/**.cpp",

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
		"./src/Engine/platforms/**",
	}

	filter { "system:Windows"}
    	files {
    		"./src/Engine/platforms/windows/**.cpp"
    	}
    	--[[links {
    		"vcruntime",
    		"msvcrt",
    		"ucrt",
    		"Gdi32",
    		"Shell32",
    	}]]--

    filter {"system:Windows", "configurations:Debug" }
		includedirs {
			"./extern/dbghelp/inc/"
		}
		libdirs {
			"./extern/dbghelp/lib/%{cfg.architecture}/"
		}
		links {
			"dbghelp"
		}

	defines {
		"YAELIB_EXPORT",
		"MIRROR_EXPORT",
		"IMGUI_API=__declspec(dllexport)",
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

	postbuildcommands {
		"xcopy \"./lib/"..TARGET_FOLDER_NAME.."/yae.dll\" \"./bin/"..TARGET_FOLDER_NAME.."/yae.dll\" /f /e /d /y "
	}

project "game"
	kind "SharedLib"
	targetdir("lib/"..TARGET_FOLDER_NAME.."/")
	--targetdir("data/code/")
	dependson { "yae" }
	optimize "Debug"
	symbols "On"

	files { 
		"./src/game/**.h",
		"./src/game/**.cpp",
	}

	includedirs {
		"./src/game/",
	}

	defines {
		"YAEGAME_EXPORT",
		"MIRROR_IMPORT",
		"IMGUI_API=__declspec(dllimport)",
	}

	links {
		"yae",
	}

project "application"
	kind "ConsoleApp"
	dependson { "yae", "game" }
	optimize "Debug"
	symbols "On"
	targetname "yae"

	files { 
		"./src/main.cpp",
	}

	defines {
		"MIRROR_IMPORT",
		"IMGUI_API=__declspec(dllimport)",
	}

	links {
		"yae",
	}
