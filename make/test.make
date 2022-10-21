-include make/common/header.make

SRCS += $(call rwildcard,src/test,*.cpp)
INCLUDEDIRS +=

ifeq ($(PLATFORM), Win64)
TARGETEXT := .dll
LDFLAGS += \
  -shared \

LIBS += \
  yae \

DEFINES += \
  MIRROR_API=__declspec(dllimport) \
  IMGUI_API=__declspec(dllimport) \
  YAE_API=__declspec(dllimport) \
  TEST_API=__declspec(dllexport) \
  _MT _DLL \

endif

ifeq ($(PLATFORM), Web)
TARGETEXT := .wasm
CPPFLAGS += \
  -fPIC \

LDFLAGS += \
  -s SIDE_MODULE=1 \

endif

BIN = $(BINDIR)/test$(TARGETEXT)

-include make/common/footer.make