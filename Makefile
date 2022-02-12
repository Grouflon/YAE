# made based on this makefile: https://gist.github.com/maxtruxa/4b3929e118914ccef057f8a05c614b0f
# install make on windows : https://stackoverflow.com/a/57042516

# ======================
# PARAMETERS

# Platform : (Win64|Web)
PLATFORM := Win64
# Configuration : (Debug|DebugOptimized|Release)
CONFIG := Release
# ======================

PROJECTS := yae game application

.PHONY: all clean help $(PROJECTS) 

all: $(PROJECTS)

yae:
	@echo ==== Building yae ($(PLATFORM):$(CONFIG)) ====
	@${MAKE} --no-print-directory -C . -f yae.make PLATFORM=$(PLATFORM) CONFIG=$(CONFIG)

game: yae
	@echo ==== Building game ($(PLATFORM):$(CONFIG)) ====
	@${MAKE} --no-print-directory -C . -f game.make PLATFORM=$(PLATFORM) CONFIG=$(CONFIG)

application: yae game
	@echo ==== Building application ($(PLATFORM):$(CONFIG)) ====
	@${MAKE} --no-print-directory -C . -f application.make PLATFORM=$(PLATFORM) CONFIG=$(CONFIG)

clean:
	@${MAKE} --no-print-directory -C . -f yae.make clean PLATFORM=$(PLATFORM) CONFIG=$(CONFIG)
	@${MAKE} --no-print-directory -C . -f game.make clean PLATFORM=$(PLATFORM) CONFIG=$(CONFIG)
	@${MAKE} --no-print-directory -C . -f application.make clean PLATFORM=$(PLATFORM) CONFIG=$(CONFIG)

help:
	@echo "Usage: make [PLATFORM=name] [CONFIG=name] [target]"
	@echo ""
	@echo "CONFIG:"
	@echo "  Debug"
	@echo "  DebugOptimized"
	@echo "  Release"
	@echo ""
	@echo "PLATFORM:"
	@echo "  Win64"
	@echo "  Web"
	@echo ""
	@echo "TARGETS:"
	@echo "   all (default)"
	@echo "   clean"
	@echo "   yae"
	@echo "   game"
	@echo "   application"
	@echo ""
	@echo "For more information, see https://github.com/premake/premake-core/wiki"