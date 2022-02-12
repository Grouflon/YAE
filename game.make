-include header.make

BIN = $(BINDIR)/game.dll

CPPFLAGS += -Wno-unused-variable -Wno-unused-function
LDFLAGS += -shared

SRCS += $(call rwildcard,src/game,*.cpp)

INCLUDEDIRS +=

LIBDIRS += \
  $(BINDIR)

LIBS += \
  yae 

DEFINES += \
  YAEGAME_EXPORT \
  MIRROR_IMPORT \
  IMGUI_API=__declspec(dllimport) \
  _MT _DLL

-include footer.make