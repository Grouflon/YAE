-include make/header.make

BIN = $(BINDIR)/game.dll

-include make/game_common.make

LDFLAGS += -shared

LIBDIRS += \
  $(BINDIR)

LIBS += \
  yae 

DEFINES += \
  YAEGAME_EXPORT \
  MIRROR_API=__declspec(dllimport) \
  IMGUI_API=__declspec(dllimport) \
  _MT _DLL

-include make/footer.make