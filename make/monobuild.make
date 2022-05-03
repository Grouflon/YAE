# This file is only used and implemented for Web right now

-include make/header.make

BIN = $(BINDIR)/yae.html

DEFINES += \
	YAELIB_API= \
	MIRROR_API= \
	YAEGAME_API= \
	STATIC_GAME_API=1 \

LDFLAGS += \
	-s LLD_REPORT_UNDEFINED \
	-s INITIAL_MEMORY=134217728 \
	-s ALLOW_MEMORY_GROWTH=1 \
	-s ASSERTIONS=2\
	--emrun \

-include make/yae_common.make
-include make/game_common.make
-include make/application_common.make

-include make/footer.make
