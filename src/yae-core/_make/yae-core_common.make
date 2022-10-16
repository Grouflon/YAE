SRCS += $(call rwildcard,src/yae-core,*.cpp)
SRCS := $(subst \,/,$(filter-out $(call rwildcard,src/yae-core/platforms,*.cpp), $(SRCS)))
SRCS += $(wildcard extern/imgui/*.cpp)
SRCS += extern/imgui/backends/imgui_impl_glfw.cpp

ifeq ($(PLATFORM), Win64)
	SRCS += \
		$(call rwildcard,src/yae-core/platforms/windows,*.cpp) \
		extern/gl3w/src/gl3w.c \
		extern/imgui/backends/imgui_impl_opengl3.cpp \

	INCLUDEDIRS += \
		extern/dbghelp/inc/ \
		extern/gl3w/include \

	INCLUDEDIRS += \
		extern/gl3w/include \

	LIBDIRS += \
		extern/dbghelp/lib/x64/ \
		
	LIBS += \
		dbghelp \

endif
ifeq ($(PLATFORM), Web)
	SRCS += $(call rwildcard,src/yae-core/platforms/web,*.cpp)
endif

INCLUDEDIRS +=

DEFINES += \
	_SILENCE_EXPERIMENTAL_FILESYSTEM_DEPRECATION_WARNING \
	_LIBCPP_NO_EXPERIMENTAL_DEPRECATION_WARNING_FILESYSTEM

# Remove warnings on external code
%/imgui_widgets.o: CPPFLAGS += -Wno-unused-variable
%/windows_platform.o: CPPFLAGS += -Wno-extra-tokens -Wno-pragma-pack
