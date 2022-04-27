-include header.make

LINKTYPE := shared
ifeq ($(PLATFORM), Web)
LINKTYPE := static
endif

ifeq ($(LINKTYPE), shared)
BIN = $(BINDIR)/yae.dll
else ifeq ($(LINKTYPE), static)
BIN = $(BINDIR)/yae.lib
endif


SRCS += $(call rwildcard,src/yae,*.cpp)
SRCS := $(subst \,/,$(filter-out $(call rwildcard,src/yae/platforms,*.cpp), $(SRCS)))
SRCS += $(wildcard extern/imgui/*.cpp)
SRCS += extern/imgui/backends/imgui_impl_glfw.cpp
SRCS += $(wildcard extern/im3d/*.cpp)
SRCS += $(wildcard extern/mirror/*.cpp)
SRCS += extern/mirror/Tools/BinarySerializer.cpp

INCLUDEDIRS +=

LIBDIRS += \
	extern/GLFW/lib/$(PLATFORM)

LIBS += \
	glfw3

ifeq ($(PLATFORM), Win64)
SRCS += $(call rwildcard,src/yae/platforms/windows,*.cpp)
INCLUDEDIRS += extern/dbghelp/inc/
LIBDIRS += extern/dbghelp/lib/x64/
LIBS += dbghelp
endif

DEFINES += \
	_SILENCE_EXPERIMENTAL_FILESYSTEM_DEPRECATION_WARNING \
	_LIBCPP_NO_EXPERIMENTAL_DEPRECATION_WARNING_FILESYSTEM

ifeq ($(LINKTYPE), shared)
LDFLAGS += -shared
DEFINES += \
	YAELIB_EXPORT \
	MIRROR_EXPORT \
	IMGUI_API=__declspec(dllexport) \
	_MT _DLL

else ifeq ($(LINKTYPE), static)
LDFLAGS += -static
DEFINES += \
	YAELIB_API= \
	MIRROR_API=
endif

# Renderers setup
USE_OPENGL := 0
USE_VULKAN := 0

ifeq ($(PLATFORM), Win64)
	USE_OPENGL := 1
	USE_VULKAN := 1
endif
ifeq ($(PLATFORM), Web)
	USE_OPENGL := 1
	USE_VULKAN := 0
endif

ifeq ($(USE_VULKAN), 1)
	DEFINES += GLFW_INCLUDE_VULKAN
	LIBDIRS += $(VK_SDK_PATH)/Lib/
	LIBS += vulkan-1

	ifeq ($(CONFIG), Debug)
		LIBDIRS += $(VK_SDK_PATH)/DebugLibs/Lib/
		LIBS += shaderc_combinedd
	else
		LIBS += shaderc_combined
	endif
endif

# Remove warnings on external code
%/imgui_impl_vulkan.o: CPPFLAGS += -Wno-unused-function
%/im3d.o: CPPFLAGS += -Wno-unused-variable -Wno-unused-function
%/windows_platform.o: CPPFLAGS += -Wno-extra-tokens -Wno-pragma-pack
%/imgui_widgets.o: CPPFLAGS += -Wno-unused-variable
%/Vma_impl.o: CPPFLAGS += -Wno-deprecated-copy


-include footer.make