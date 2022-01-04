OBJS += $(patsubst %,$(OBJDIR)/%.o,$(basename $(SRCS))) # object files, auto generated from source files
DEPS += $(patsubst %,$(DEPDIR)/%.d,$(basename $(SRCS))) # dependency files, auto generated from source files

# compilers (at least gcc and clang) don't create the subdirectories automatically
OBJ_DIRS := $(subst /,\,$(dir $(OBJS)))
DEP_DIRS := $(subst /,\,$(dir $(DEPS)))
$(shell mkdir $(OBJ_DIRS) >nul 2>&1)
$(shell mkdir $(DEP_DIRS) >nul 2>&1)
$(info $(BINDIR))
$(shell mkdir $(subst /,\,$(BINDIR)) >nul 2>&1)

.PHONY: all clean

all: $(BIN)

clean:
	@del /s /q $(subst /,\,intermediate/build/$(TARGET_FOLDER))
	@del /s /q $(subst /,\,$(BINDIR))

$(BIN): $(OBJS)
	$(info Linking $(BIN) ...)
	$(LINK.o) $^

# compile C source files
COMPILE.c = @$(CC) $(DEPFLAGS) $(CFLAGS) $(CPPFLAGS) $(patsubst %,-I%,$(INCLUDEDIRS)) $(patsubst %,-D%,$(DEFINES)) -c -o $@
# compile C++ source files
COMPILE.cc = @$(CXX) $(DEPFLAGS) $(CXXFLAGS) $(CPPFLAGS) $(patsubst %,-I%,$(INCLUDEDIRS)) $(patsubst %,-D%,$(DEFINES)) -c -o $@
# link object files to binary
LINK.o = $(LD) $(LDFLAGS) $(LDLIBS) $(patsubst %,-L%,$(LIBDIRS)) $(patsubst %,-l%,$(LIBS)) -o $@
# precompile step
PRECOMPILE =
# postcompile step
POSTCOMPILE = @move /y $(subst /,\,$(DEPDIR)/$*.Td) $(subst /,\,$(DEPDIR)/$*.d) >nul 2>&1

$(OBJDIR)/%.o: %.c
$(OBJDIR)/%.o: %.c $(DEPDIR)/%.d
	$(PRECOMPILE)
	$(COMPILE.c) $<
	$(POSTCOMPILE)

$(OBJDIR)/%.o: %.cpp
$(OBJDIR)/%.o: %.cpp $(DEPDIR)/%.d
	$(PRECOMPILE)
	$(info Compiling $< ...)
	$(COMPILE.cc) $<
	$(POSTCOMPILE)

$(OBJDIR)/%.o: %.cc
$(OBJDIR)/%.o: %.cc $(DEPDIR)/%.d
	$(PRECOMPILE)
	$(COMPILE.cc) $<
	$(POSTCOMPILE)

$(OBJDIR)/%.o: %.cxx
$(OBJDIR)/%.o: %.cxx $(DEPDIR)/%.d
	$(PRECOMPILE)
	$(COMPILE.cc) $<
	$(POSTCOMPILE)

.PRECIOUS: $(DEPDIR)/%.d
$(DEPDIR)/%.d: ;
