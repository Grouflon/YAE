# ======================
# FUNCTIONS
# found here: https://stackoverflow.com/questions/2483182/recursive-wildcards-in-gnu-make
rwildcard=$(foreach d,$(wildcard $(1:=/*)),$(call rwildcard,$d,$2) $(filter $(subst *,%,$2),$d))
# ======================

TARGET_FOLDER := $(PLATFORM)_$(CONFIG)
BINDIR := bin/$(TARGET_FOLDER)
#$(shell set VSLANG=1033)

# intermediate directory for generated object files
OBJDIR := intermediate/build/$(TARGET_FOLDER)/obj
# intermediate directory for generated dependency files
DEPDIR := intermediate/build/$(TARGET_FOLDER)/dep

# ======================
# GLOBAL
# ======================

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

INCLUDEDIRS := \
	src/ \
	extern/ \
	extern/mirror/ \
	extern/glm/ \
	extern/GLFW/include/ \
	extern/imgui/ \
	extern/im3d/ \
	extern/glm/ \
	extern/VulkanMemoryAllocator/ \
	$(VK_SDK_PATH)/include

DEFINES := \
	_CRT_SECURE_NO_WARNINGS \
	GLM_FORCE_RADIANS \
	GLM_FORCE_DEPTH_ZERO_TO_ONE \
	IM3D_CONFIG=\<yae/yae_im3d_config.h\>

LIBS :=
ifeq ($(PLATFORM), Win64)
LIBS += msvcrt ucrt msvcprt vcruntime user32 kernel32 gdi32 imm32 shell32
endif

OBJS :=
DEPS :=
