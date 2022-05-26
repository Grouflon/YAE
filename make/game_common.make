SRCS += $(call rwildcard,src/game,*.cpp)
INCLUDEDIRS +=

CPPFLAGS += -Wno-unused-variable -Wno-unused-function
ifeq ($(PLATFORM), Web)
LDFLAGS += --preload-file ./data
endif