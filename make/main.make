-include make/common/header.make

SRCS += src/main.cpp

ifeq ($(PLATFORM), Win64)
TARGETEXT := .exe

LIBS += \
	yae \

DEFINES += \

endif

ifeq ($(PLATFORM), Web)
TARGETEXT := .html
DEFINES += \

CPPFLAGS += \
	-fPIC \

LDFLAGS += \
	-s MAIN_MODULE=1 \
	-s LLD_REPORT_UNDEFINED \
	-s INITIAL_MEMORY=134217728 \
	-s ALLOW_MEMORY_GROWTH=1 \
	-s ASSERTIONS=1 \
	--preload-file ./data \
	--preload-file $(BINDIR)/game.wasm@/ \
	--use-preload-plugins \
	$(BINDIR)/yae.wasm \
	--emrun \

endif

BIN = $(BINDIR)/main$(TARGETEXT)

-include make/common/footer.make