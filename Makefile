# made based on this makefile: https://gist.github.com/maxtruxa/4b3929e118914ccef057f8a05c614b0f
# install make on windows : https://stackoverflow.com/a/57042516

# ======================
# PARAMETERS

# Platform : (Win64|Web)
PLATFORM := Win64
# Configuration : (Debug|Release)
CONFIG := Release

$(info Building for $(PLATFORM):$(CONFIG))

# ======================
# FUNCTIONS
# found here: https://stackoverflow.com/questions/2483182/recursive-wildcards-in-gnu-make
rwildcard=$(foreach d,$(wildcard $(1:=/*)),$(call rwildcard,$d,$2) $(filter $(subst *,%,$2),$d))
# ======================

$(shell set VSLANG=1033)

# intermediate directory for generated object files
OBJDIR := intermediate/build/obj
# intermediate directory for generated dependency files
DEPDIR := intermediate/build/dep

# output binary
BIN := bin/yae.dll

# source files
SRCS := \
    src/test.cpp

#SRC := $(wildcard src/yae/*/*.cpp)
YAE_SRCS := $(call rwildcard,src/yae,*.cpp)
YAE_SRCS := $(subst \,/,$(filter-out $(call rwildcard,src/yae/platforms,*.cpp), $(YAE_SRCS)))
YAE_SRCS += 
YAE_SRCS += $(wildcard extern/imgui/*.cpp)
YAE_SRCS += extern/imgui/backends/imgui_impl_glfw.cpp
YAE_SRCS += $(wildcard extern/im3d/*.cpp)
YAE_SRCS += $(wildcard extern/mirror/*.cpp)
YAE_SRCS += extern/mirror/Tools/BinarySerializer.cpp

YAE_SRCS += $(call rwildcard,src/yae/platforms/windows,*.cpp)

YAE_INCLUDEDIRS := \
	src/ \
	extern/glm/ \
	extern/GLFW/include/ \
	extern/imgui/ \
	extern/im3d/ \
	extern/glm/ \
	extern/VulkanMemoryAllocator/ \
	extern/ \
	$(VK_SDK_PATH)/include \
	extern/dbghelp/inc/ 

YAE_LIBDIRS := \
	extern/GLFW/lib/$(PLATFORM) \
	$(VK_SDK_PATH)/Lib/ \
	extern/dbghelp/lib/x64/ 

YAE_LIBS := \
	glfw3 \
	vulkan-1 \
	shaderc_combined \
	dbghelp \
	msvcrt ucrt msvcprt vcruntime user32 kernel32 gdi32 imm32 shell32


#$(info $(YAE_SRCS))

# object files, auto generated from source files
OBJS := $(patsubst %,$(OBJDIR)/%.o,$(basename $(YAE_SRCS)))
OBJ_DIRS := $(subst /,\,$(dir $(OBJS)))
# dependency files, auto generated from source files
DEPS := $(patsubst %,$(DEPDIR)/%.d,$(basename $(YAE_SRCS)))
DEP_DIRS := $(subst /,\,$(dir $(DEPS)))

# compilers (at least gcc and clang) don't create the subdirectories automatically
$(shell mkdir $(OBJ_DIRS) >nul 2>&1)
$(shell mkdir $(DEP_DIRS) >nul 2>&1)

# C compiler
CC := clang
# C++ compiler
CXX := clang++
# linker
LD := clang++

# C flags
CFLAGS := -std=c14
# C++ flags
CXXFLAGS := -std=c++14
# C/C++ flags
CPPFLAGS := -g -Wall -Wextra -Wno-unused-parameter -Wno-gnu-anonymous-struct -Wno-nullability-completeness -Wno-nullability-extension
# linker flags
LDFLAGS := -shared -Xlinker /NODEFAULTLIB
# flags required for dependency generation; passed to compilers
DEPFLAGS = -MT $@ -MD -MP -MF $(DEPDIR)/$*.Td

DEFINES := \
	_CRT_SECURE_NO_WARNINGS \
	GLFW_INCLUDE_VULKAN \
	GLM_FORCE_RADIANS \
	GLM_FORCE_DEPTH_ZERO_TO_ONE \
	\
	YAELIB_EXPORT \
	MIRROR_EXPORT \
	IMGUI_API=__declspec(dllexport) \
	_SILENCE_EXPERIMENTAL_FILESYSTEM_DEPRECATION_WARNING \
	IM3D_CONFIG=\<yae/yae_im3d_config.h\> \
	_MT _DLL


INCLUDEFLAGS = $(patsubst %,-I%,$(YAE_INCLUDEDIRS))
DEFINEFLAGS = $(patsubst %,-D%,$(DEFINES))
LIBDIRSFLAGS = $(patsubst %,-L%,$(YAE_LIBDIRS))
LIBSFLAGS = $(patsubst %,-l%,$(YAE_LIBS))

# compile C source files
COMPILE.c = @$(CC) $(DEPFLAGS) $(CFLAGS) $(CPPFLAGS) $(INCLUDEFLAGS) $(DEFINEFLAGS) -c -o $@
# compile C++ source files
COMPILE.cc = @$(CXX) $(DEPFLAGS) $(CXXFLAGS) $(CPPFLAGS) $(INCLUDEFLAGS) $(DEFINEFLAGS) -c -o $@
# link object files to binary
LINK.o = $(LD) $(LDFLAGS) $(LDLIBS) $(LIBDIRSFLAGS) $(LIBSFLAGS) -o $@
# precompile step
PRECOMPILE =
# postcompile step
POSTCOMPILE = @move /y $(subst /,\,$(DEPDIR)/$*.Td) $(subst /,\,$(DEPDIR)/$*.d) >nul 2>&1

all: $(BIN)

.PHONY: clean
clean:
	del /s /q $(subst /,\,$(OBJDIR)) $(subst /,\,$(DEPDIR))

$(BIN): $(OBJS)
	$(info Linking $(BIN) ...)
	$(LINK.o) $^

$(OBJDIR)/%.o: %.c
$(OBJDIR)/%.o: %.c $(DEPDIR)/%.d
	$(PRECOMPILE)
	$(COMPILE.c) $<
	$(POSTCOMPILE)

$(OBJDIR)/%.o: %.cpp
$(OBJDIR)/%.o: %.cpp $(DEPDIR)/%.d
	$(PRECOMPILE)
	$(info Compiling $< ...)
	$(COMPILE.cc) $<
	$(POSTCOMPILE)

$(OBJDIR)/%.o: %.cc
$(OBJDIR)/%.o: %.cc $(DEPDIR)/%.d
	$(PRECOMPILE)
	$(COMPILE.cc) $<
	$(POSTCOMPILE)

$(OBJDIR)/%.o: %.cxx
$(OBJDIR)/%.o: %.cxx $(DEPDIR)/%.d
	$(PRECOMPILE)
	$(COMPILE.cc) $<
	$(POSTCOMPILE)

.PRECIOUS: $(DEPDIR)/%.d
$(DEPDIR)/%.d: ;

-include $(DEPS)