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
CFLAGS := -std=c11
# C++ flags
CXXFLAGS := -std=c++14
# C/C++ flags
CPPFLAGS := -Wall -Werror -Wextra -Wno-unused-parameter -Wno-gnu-anonymous-struct -Wno-nullability-completeness -Wno-nullability-extension -Wno-void-pointer-to-int-cast -Wno-int-to-void-pointer-cast -Wno-switch
CPPFLAGS += -g #generate symbols
# linker flags
LDFLAGS := 
LDFLAGS += -g #generate symbols
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
	extern/stb/ \
	$(VK_SDK_PATH)/include

DEFINES := \
	_CRT_SECURE_NO_WARNINGS \
	GLM_FORCE_RADIANS \
	GLM_FORCE_DEPTH_ZERO_TO_ONE \
	IM3D_CONFIG=\"yae/yae_im3d_config.h\"\
	IMGUI_USER_CONFIG=\"yae/yae_imconfig.h\"\
	YAE_CONFIG=\"$(CONFIG)\"

LIBS :=

# ======================
# CUSTOM RULES
# ======================
# CONFIG
# =============
ifeq ($(CONFIG), Debug)
CPPFLAGS += -O0
LDFLAGS += -O0
DEFINES += \
	DEBUG \
	_DEBUG \
	YAE_DEBUG
endif
ifeq ($(CONFIG), DebugOptimized)
CPPFLAGS += -O3
LDFLAGS += -O3
DEFINES += \
	NDEBUG \
	YAE_DEBUG
endif
ifeq ($(CONFIG), Release)
CPPFLAGS += -O3
CPPFLAGS += -Wno-unused-comparison -Wno-unused-variable -Wno-unused-but-set-variable # happens a lot with deactivated asserts
LDFLAGS += -O3
DEFINES += \
	NDEBUG \
	YAE_RELEASE
endif

# =============
# WINDOWS
# =============
ifeq ($(PLATFORM), Win64)
LIBS += user32 kernel32 gdi32 imm32 shell32
DEFINES += \
	YAE_PLATFORM_WINDOWS

LDFLAGS += -Xlinker /NODEFAULTLIB
LDFLAGS += -Xlinker /ignore:4099 # Warning about missing pdbs for external libs, we don't care

ifeq ($(CONFIG), Debug)
LIBS += msvcrtd ucrtd vcruntimed msvcprtd
else
LIBS += msvcrt ucrt vcruntime msvcprt
endif

endif

# =============
# WEB
# =============
ifeq ($(PLATFORM), Web)
CC = emcc
CXX = emcc
LD = emcc
CPPFLAGS += \
	-Wno-unknown-pragmas \

DEFINES += \
	YAE_PLATFORM_WEB

LDFLAGS += \
	-s USE_GLFW=3 \
	-gsource-map \
	--source-map-base=http://localhost:6931/bin/Web_Debug/
	# allow the web browser debugger to have c++ functions information 

endif


OBJS :=
DEPS :=
