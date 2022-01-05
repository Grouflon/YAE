-include header.make

BIN = $(BINDIR)/main.exe

SRCS += src/main.cpp

INCLUDEDIRS +=

LIBDIRS += \
	$(BINDIR)

LIBS += \
	yae

DEFINES += \
	MIRROR_IMPORT \
	IMGUI_API=__declspec(dllimport)

-include footer.make