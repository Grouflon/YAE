# ======================
# FUNCTIONS
# found here: https://stackoverflow.com/questions/2483182/recursive-wildcards-in-gnu-make
rwildcard=$(foreach d,$(wildcard $(1:=/*)),$(call rwildcard,$d,$2) $(filter $(subst *,%,$2),$d))
# ======================

TARGET := $(PLATFORM)_$(CONFIG)
BINDIR := bin/$(TARGET)
#$(shell set VSLANG=1033)

# intermediate directory for generated object files
OBJDIR := intermediate/build/$(TARGET)/obj
# intermediate directory for generated dependency files
DEPDIR := intermediate/build/$(TARGET)/dep

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
CPPFLAGS := -Wall -Werror -Wextra -Wno-unused-parameter -Wno-gnu-anonymous-struct -Wno-nullability-completeness -Wno-nullability-extension
# linker flags
LDFLAGS := -Xlinker /NODEFAULTLIB
LDFLAGS += -Xlinker /ignore:4099 # Warning about missing pdbs for external libs, we don't care
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

ifeq ($(CONFIG), Debug)
CPPFLAGS += -g -O0
LDFLAGS += -g -O0
DEFINES += \
	DEBUG \
	_DEBUG \
	YAE_DEBUG
endif
ifeq ($(CONFIG), Release)
CPPFLAGS += -g -O3
CPPFLAGS += -Wno-unused-comparison -Wno-unused-variable # happens a lot with deactivated asserts
LDFLAGS += -g -O3
DEFINES += \
	NDEBUG \
	YAE_RELEASE
endif

ifeq ($(PLATFORM), Win64)
LIBS += user32 kernel32 gdi32 imm32 shell32
endif

ifeq ($(TARGET), Win64_Debug)
LIBS += msvcrtd ucrtd vcruntimed msvcprtd
else ifeq ($(TARGET), Win64_Release)
LIBS += msvcrt ucrt vcruntime msvcprt
endif


OBJS :=
DEPS :=
