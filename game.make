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

ifeq ($(PLATFORM), Win64)
SRCS += $(call rwildcard,src/yae/platforms/windows,*.cpp)
INCLUDEDIRS += extern/dbghelp/inc/
LIBDIRS += extern/dbghelp/lib/x64/
LIBS += dbghelp
endif

DEFINES += \
  YAEGAME_EXPORT \
  MIRROR_IMPORT \
  IMGUI_API=__declspec(dllimport) \
  _MT _DLL

-include footer.make