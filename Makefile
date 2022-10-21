# made based on this makefile: https://gist.github.com/maxtruxa/4b3929e118914ccef057f8a05c614b0f
# install make on windows : https://stackoverflow.com/a/57042516

# ======================
# PARAMETERS

# Platform : (Win64|Web)
PLATFORM := Win64
# Configuration : (Debug|DebugOptimized|Release)
CONFIG := Release
# ======================

PROJECTS := yae test editor game main

.PHONY: all clean help $(PROJECTS) 

all: $(PROJECTS)

#application.make.uptodate: make/application.make
	#@echo ==== $< ====
	#@copy /b $< +,, 
	#${MAKE} --no-print-directory -C . -f $< clean PLATFORM=$(PLATFORM) CONFIG=$(CONFIG)

yae:
	@echo ==== Building yae ($(PLATFORM):$(CONFIG)) ====
	@${MAKE} --no-print-directory -C . -f make/yae.make PLATFORM=$(PLATFORM) CONFIG=$(CONFIG)

test: yae
	@echo ==== Building test ($(PLATFORM):$(CONFIG)) ====
	@${MAKE} --no-print-directory -C . -f make/test.make PLATFORM=$(PLATFORM) CONFIG=$(CONFIG)

editor: yae
	@echo ==== Building editor ($(PLATFORM):$(CONFIG)) ====
	@${MAKE} --no-print-directory -C . -f make/editor.make PLATFORM=$(PLATFORM) CONFIG=$(CONFIG)

game: yae
	@echo ==== Building game ($(PLATFORM):$(CONFIG)) ====
	@${MAKE} --no-print-directory -C . -f make/game.make PLATFORM=$(PLATFORM) CONFIG=$(CONFIG)

main: yae test editor game
	@echo ==== Building main ($(PLATFORM):$(CONFIG)) ====
	@${MAKE} --no-print-directory -C . -f make/main.make PLATFORM=$(PLATFORM) CONFIG=$(CONFIG)

clean:
	@${MAKE} --no-print-directory -C . -f make/yae.make clean PLATFORM=$(PLATFORM) CONFIG=$(CONFIG)
	@${MAKE} --no-print-directory -C . -f make/test.make clean PLATFORM=$(PLATFORM) CONFIG=$(CONFIG)
	@${MAKE} --no-print-directory -C . -f make/editor.make clean PLATFORM=$(PLATFORM) CONFIG=$(CONFIG)
	@${MAKE} --no-print-directory -C . -f make/game.make clean PLATFORM=$(PLATFORM) CONFIG=$(CONFIG)
	@${MAKE} --no-print-directory -C . -f make/main.make clean PLATFORM=$(PLATFORM) CONFIG=$(CONFIG)

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
	@echo "   test"
	@echo "   editor"
	@echo "   game"
	@echo "   main"
	@echo ""
