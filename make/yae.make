-include make/common/header.make

# SOURCES
SRCS += src/yae/MirrorImplementation.cpp
SRCS += $(call rwildcard,src/yae,*.cpp)
SRCS := $(subst \,/,$(filter-out $(call rwildcard,src/yae/platforms,*.cpp), $(SRCS)))
SRCS += $(wildcard extern/imgui/*.cpp)
SRCS += extern/imgui/backends/imgui_impl_glfw.cpp
SRCS := $(subst \,/,$(filter-out $(call rwildcard,src/yae/rendering/renderers,*.cpp), $(SRCS)))
SRCS += $(wildcard extern/im3d/*.cpp)
SRCS += extern/imgui/backends/imgui_impl_opengl3.cpp

DEFINES += \
	_SILENCE_EXPERIMENTAL_FILESYSTEM_DEPRECATION_WARNING \
	_LIBCPP_NO_EXPERIMENTAL_DEPRECATION_WARNING_FILESYSTEM \

# Renderers setup
USE_OPENGL := 0
USE_VULKAN := 0

# PLATFORMS
ifeq ($(PLATFORM), Win64)
	TARGETEXT := .dll
	USE_OPENGL := 1

	SRCS += \
		$(call rwildcard,src/yae/platforms/windows,*.cpp) \

	LIBDIRS += \
		extern/GLFW/lib/$(PLATFORM) \
		extern/dbghelp/lib/x64/ \

	INCLUDEDIRS += \
		extern/dbghelp/inc/ \
		extern/gl3w/include \

	LIBS += \
		glfw3 \
		dbghelp \

	LDFLAGS += \
		-shared \

	DEFINES += \
		MIRROR_API=__declspec(dllexport) \
		IMGUI_API=__declspec(dllexport) \
		GL3W_API=__declspec(dllexport) \
		YAE_API=__declspec(dllexport) \
		_MT _DLL \

endif
ifeq ($(PLATFORM), Web)
	TARGETEXT := .wasm
	USE_OPENGL := 1

	SRCS += \
		$(call rwildcard,src/yae/platforms/web,*.cpp) \

	CPPFLAGS += \
		-fPIC \

	DEFINES += \

	LDFLAGS += \
		-s SIDE_MODULE=1 \

endif

# RENDERERS
ifeq ($(USE_VULKAN), 1)
	SRCS += $(call rwildcard,src/yae/rendering/renderers/vulkan,*.cpp)
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
		$(call rwildcard,src/yae/rendering/renderers/opengl,*.cpp) \

	ifeq ($(PLATFORM), Win64)
		SRCS += extern/gl3w/src/gl3w.c
		INCLUDEDIRS += extern/gl3w/include
	endif

	DEFINES += \
		YAE_IMPLEMENTS_RENDERER_OPENGL=1 \

endif

# Remove warnings on external code
%/imgui_widgets.o: CPPFLAGS += -Wno-unused-variable
%/windows_platform.o: CPPFLAGS += -Wno-extra-tokens -Wno-pragma-pack
%/imgui_impl_vulkan.o: CPPFLAGS += -Wno-unused-function
%/im3d.o: CPPFLAGS += -Wno-unused-variable -Wno-unused-function
%/Vma_impl.o: CPPFLAGS += -Wno-deprecated-copy
%/ResourceManager.o: CPPFLAGS += -Wno-missing-field-initializers

BIN = $(BINDIR)/yae$(TARGETEXT)

-include make/common/footer.make
