-include header.make

BIN = $(BINDIR)/yae.dll

SRCS += $(call rwildcard,src/yae,*.cpp)
SRCS := $(subst \,/,$(filter-out $(call rwildcard,src/yae/platforms,*.cpp), $(SRCS)))
SRCS += $(wildcard extern/imgui/*.cpp)
SRCS += extern/imgui/backends/imgui_impl_glfw.cpp
SRCS += $(wildcard extern/im3d/*.cpp)
SRCS += $(wildcard extern/mirror/*.cpp)
SRCS += extern/mirror/Tools/BinarySerializer.cpp

INCLUDEDIRS +=

LIBDIRS += \
	extern/GLFW/lib/$(PLATFORM) \
	$(VK_SDK_PATH)/Lib/

LIBS += \
	glfw3 \
	vulkan-1 \
	shaderc_combined

ifeq ($(PLATFORM), Win64)
SRCS += $(call rwildcard,src/yae/platforms/windows,*.cpp)
INCLUDEDIRS += extern/dbghelp/inc/
LIBDIRS += extern/dbghelp/lib/x64/
LIBS += dbghelp
LIBS += msvcrt ucrt msvcprt vcruntime user32 kernel32 gdi32 imm32 shell32
endif

DEFINES += \
	GLFW_INCLUDE_VULKAN \
	YAELIB_EXPORT \
	MIRROR_EXPORT \
	IMGUI_API=__declspec(dllexport) \
	_SILENCE_EXPERIMENTAL_FILESYSTEM_DEPRECATION_WARNING \
	_MT _DLL

-include footer.make