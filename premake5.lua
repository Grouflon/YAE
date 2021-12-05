local VK_SDK_PATH = os.getenv("VK_SDK_PATH")
local TARGET_FOLDER_NAME = "%{cfg.platform}_%{cfg.buildcfg}"

workspace "yaeApplication"
	configurations { "Debug", "Release" }
	platforms { "Win32", "Win64" }

	language "C++"
	cppdialect "C++14"
	symbols "On"
	targetdir("./bin/"..TARGET_FOLDER_NAME.."/")
	objdir("./obj/"..TARGET_FOLDER_NAME.."/")

	includedirs {
		"./src/",
		"./extern/GLFW/include/",
		"./extern/imgui/",
		"./extern/im3d/",
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
		"IM3D_CONFIG=<yae/yae_im3d_config.h>",
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
		defines { "NDEBUG", "YAE_RELEASE" }
		optimize "On"

	filter "system:Windows"
		buildoptions("/w44251") -- disable class '...' needs to have dll-interface to be used by clients of class '...'

	filter {}

project "yae"
	kind "SharedLib"
	targetdir("bin/"..TARGET_FOLDER_NAME.."/")

	files { 
		"./src/yae/**.h",
		"./src/yae/**.cpp",

		"./extern/imgui/*.h",
		"./extern/imgui/*.cpp",
		"./extern/imgui/backends/imgui_impl_glfw.cpp",
		"./extern/imgui/backends/imgui_impl_glfw.h",

		"./extern/im3d/*.h",
		"./extern/im3d/*.cpp",

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
    	includedirs {
			"./extern/dbghelp/inc/"
		}
    	libdirs {
			"./extern/dbghelp/lib/%{cfg.architecture}/",
		}
		links {
			"dbghelp"
		}

    	--[[links {
    		"vcruntime",
    		"msvcrt",
    		"ucrt",
    		"Gdi32",
    		"Shell32",
    	}]]--
    filter {}

	defines {
		"YAELIB_EXPORT",
		"MIRROR_EXPORT",
		"IMGUI_API=__declspec(dllexport)",
		"_SILENCE_EXPERIMENTAL_FILESYSTEM_DEPRECATION_WARNING", -- Allows use of experimental/filesystem on windows
	}

	libdirs {
		"./extern/GLFW/lib/%{cfg.platform}/",
	}

	filter {"configurations:Debug" }
		ignoredefaultlibraries { "MSVCRT" } -- we need to build glfw debug libraries to get rid of that
		links { "shaderc_combinedd" }
		linkoptions { "/ignore:4099" } -- we are missing the pdbs of shaderc_combined debug libs, it generates warnings but we dont care

	filter {"configurations:Release" }
		links { "shaderc_combined" }

	filter { "platforms:Win32", "configurations:Debug" }
		libdirs {
			VK_SDK_PATH.."/DebugLibs/Lib32/" --This is a custom path, you need to get the custom libs separately on vulkan's website
		}

	filter { "platforms:Win64", "configurations:Debug" }
		libdirs {
			VK_SDK_PATH.."/DebugLibs/Lib/" --This is a custom path, you need to get the custom libs separately on vulkan's website
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

	--postbuildcommands {
	--	"echo F|xcopy \"./lib/"..TARGET_FOLDER_NAME.."/yae.dll\" \"./bin/"..TARGET_FOLDER_NAME.."/yae.dll\" /f /e /d /y "
	--}

project "game"
	kind "SharedLib"
	targetdir("bin/"..TARGET_FOLDER_NAME.."/")
	--targetdir("data/code/")
	dependson { "yae" }

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

	--postbuildcommands {
	--	"echo F|xcopy \"./lib/"..TARGET_FOLDER_NAME.."/game.dll\" \"./bin/"..TARGET_FOLDER_NAME.."/game.dll\" /f /e /d /y "
	--}

project "application"
	kind "ConsoleApp"
	dependson { "yae", "game" }
	targetname "main"

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
