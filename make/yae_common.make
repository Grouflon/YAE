SRCS += $(call rwildcard,src/yae,*.cpp)
SRCS := $(subst \,/,$(filter-out $(call rwildcard,src/yae/platforms,*.cpp), $(SRCS)))
SRCS := $(subst \,/,$(filter-out $(call rwildcard,src/yae/renderers,*.cpp), $(SRCS)))
SRCS += $(wildcard extern/imgui/*.cpp)
SRCS += extern/imgui/backends/imgui_impl_glfw.cpp
SRCS += $(wildcard extern/im3d/*.cpp)
SRCS += $(wildcard extern/mirror/*.cpp)
SRCS += extern/mirror/Tools/BinarySerializer.cpp

INCLUDEDIRS +=

DEFINES += \
	_SILENCE_EXPERIMENTAL_FILESYSTEM_DEPRECATION_WARNING \
	_LIBCPP_NO_EXPERIMENTAL_DEPRECATION_WARNING_FILESYSTEM

ifeq ($(PLATFORM), Win64)
SRCS += $(call rwildcard,src/yae/platforms/windows,*.cpp)
INCLUDEDIRS += extern/dbghelp/inc/
LIBDIRS += extern/dbghelp/lib/x64/
LIBS += dbghelp
endif
ifeq ($(PLATFORM), Web)
SRCS += $(call rwildcard,src/yae/platforms/web,*.cpp)
endif

# Renderers setup
USE_OPENGL := 0
USE_VULKAN := 0
USE_SHADERCOMPILER := 0

ifeq ($(PLATFORM), Win64)
	USE_OPENGL := 1
	USE_SHADERCOMPILER := 1
endif
ifeq ($(PLATFORM), Web)
	USE_OPENGL := 1
endif

ifeq ($(USE_SHADERCOMPILER), 1)
	DEFINES += YAE_USE_SHADERCOMPILER=1
	LIBDIRS += extern/shaderc/lib/x64/

	ifeq ($(CONFIG), Debug)
		LIBS += shaderc_combinedd
	else
		LIBS += shaderc_combined
	endif
endif

ifeq ($(USE_VULKAN), 1)
	SRCS += $(call rwildcard,src/yae/renderers/vulkan,*.cpp)
	DEFINES += \
		YAE_IMPLEMENTS_RENDERER_VULKAN=1 \
		GLFW_INCLUDE_VULKAN \

	LIBDIRS += $(VK_SDK_PATH)/Lib/
	LIBS += vulkan-1

	ifeq ($(CONFIG), Debug)
		LIBDIRS += $(VK_SDK_PATH)/DebugLibs/Lib/
	endif
endif

ifeq ($(USE_OPENGL), 1)
	SRCS += \
		$(call rwildcard,src/yae/renderers/opengl,*.cpp) \
		extern/gl3w/src/gl3w.c \
		extern/imgui/backends/imgui_impl_opengl3.cpp \

	INCLUDEDIRS += \
		extern/gl3w/include \

	DEFINES += \
		YAE_IMPLEMENTS_RENDERER_OPENGL=1 \

endif

# Remove warnings on external code
%/imgui_impl_vulkan.o: CPPFLAGS += -Wno-unused-function
%/im3d.o: CPPFLAGS += -Wno-unused-variable -Wno-unused-function
%/windows_platform.o: CPPFLAGS += -Wno-extra-tokens -Wno-pragma-pack
%/imgui_widgets.o: CPPFLAGS += -Wno-unused-variable
%/Vma_impl.o: CPPFLAGS += -Wno-deprecated-copy