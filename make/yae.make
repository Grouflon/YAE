-include make/header.make

BIN = $(BINDIR)/yae.dll

-include make/yae_common.make

LIBDIRS += \
	extern/GLFW/lib/$(PLATFORM)

LIBS += \
	glfw3

LDFLAGS += -shared
DEFINES += \
	YAELIB_EXPORT \
	MIRROR_API=__declspec(dllexport) \
	IMGUI_API=__declspec(dllexport) \
	_MT _DLL

-include make/footer.make