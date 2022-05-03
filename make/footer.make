OBJS += $(patsubst %,$(OBJDIR)/%.o,$(basename $(SRCS))) # object files, auto generated from source files
DEPS += $(patsubst %,$(DEPDIR)/%.d,$(basename $(SRCS))) # dependency files, auto generated from source files

# compilers (at least gcc and clang) don't create the subdirectories automatically
# create target directory if not exists
CREATE_TARGET_DIRECTORY = @if not exist $(subst /,\,$(@D)) ( mkdir $(subst /,\,$(@D)) )
# compile C source files
COMPILE.c = @$(CC) $(DEPFLAGS) $(CFLAGS) $(CPPFLAGS) $(patsubst %,-I%,$(INCLUDEDIRS)) $(patsubst %,-D%,$(DEFINES)) -c -o $@
# compile C++ source files
COMPILE.cc = @$(CXX) $(DEPFLAGS) $(CXXFLAGS) $(CPPFLAGS) $(patsubst %,-I%,$(INCLUDEDIRS)) $(patsubst %,-D%,$(DEFINES)) -c -o $@
# link object files to binary
LINK.o = @$(LD) $(LDFLAGS) $(LDLIBS) $(patsubst %,-L%,$(LIBDIRS)) $(patsubst %,-l%,$(LIBS)) -o $@
# precompile step
PRECOMPILE = $(CREATE_TARGET_DIRECTORY)
# postcompile step
POSTCOMPILE = @move /y $(subst /,\,$(DEPDIR)/$*.Td) $(subst /,\,$(DEPDIR)/$*.d) >nul 2>&1
# delete directory function
delete_directory = @(rd /s /q $(1) >nul 2>&1 & exit 0) && if exist $(1) (rd /s /q $(1) >nul 2>&1)  # windows will sometimes fail to remove the folder the first time so we are always doing it twice in a row


.PHONY: all clean

all: $(BIN)

clean:
	$(info Cleaning up $(BIN):$(TARGET) files...)
	$(call delete_directory,$(subst /,\,intermediate/build/$(TARGET)))
	$(call delete_directory,$(subst /,\,$(BINDIR)))


$(BIN): $(OBJS)
	$(info Linking $(BIN) ...)
	$(CREATE_TARGET_DIRECTORY)
	$(LINK.o) $^

$(OBJDIR)/%.o: %.c
$(OBJDIR)/%.o: %.c $(DEPDIR)/%.d
	$(PRECOMPILE)
	$(info Compiling $< ...)
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
	$(info Compiling $< ...)
	$(COMPILE.cc) $<
	$(POSTCOMPILE)

$(OBJDIR)/%.o: %.cxx
$(OBJDIR)/%.o: %.cxx $(DEPDIR)/%.d
	mkdir $(@D)
	$(PRECOMPILE)
	$(COMPILE.cc) $<
	$(POSTCOMPILE)

.PRECIOUS: $(DEPDIR)/%.d
$(DEPDIR)/%.d: ;
	$(CREATE_TARGET_DIRECTORY)

-include $(DEPS)