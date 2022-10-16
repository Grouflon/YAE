-include make/header.make


-include src/yae-core/_make/yae-core_common.make

ifeq ($(PLATFORM), Win64)
TARGETEXT := .dll
LIBDIRS += \
	extern/GLFW/lib/$(PLATFORM) \

LIBS += \
	glfw3 \

LDFLAGS += \
	-shared \

DEFINES += \
	YAE_API=__declspec(dllexport) \
	MIRROR_API=__declspec(dllexport) \
	GL3W_API=__declspec(dllexport) \
	IMGUI_API=__declspec(dllexport) \
	_MT _DLL \
	
endif

ifeq ($(PLATFORM), Web)
TARGETEXT := .wasm
CPPFLAGS += \
	-fPIC \

LDFLAGS += \
	-s SIDE_MODULE \

endif

BIN = $(BINDIR)/yae-core$(TARGETEXT)

-include make/footer.make