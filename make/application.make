-include make/header.make

BIN = $(BINDIR)/main.exe

-include make/application_common.make

LIBDIRS += \
	$(BINDIR)

LIBS += \
	yae

DEFINES += \
	MIRROR_IMPORT \
	IMGUI_API=__declspec(dllimport)

-include make/footer.make