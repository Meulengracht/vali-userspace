# Makefile for building a generic userspace application or library
SOURCES = $(wildcard ./*.c)
OBJECTS = $(SOURCES:.c=.o)

CONFIG_FLAGS = -DZLIB_DLL
CFLAGS = $(VALI_CFLAGS) -O3 $(CONFIG_FLAGS) $(VALI_INCLUDES)
LFLAGS = /entry:__CrtLibraryEntry /dll $(VALI_LFLAGS) $(VALI_SDK_CLIBS)

.PHONY: all
all: $(VALI_APPLICATION_PATH)/bin/z.dll

$(VALI_APPLICATION_PATH)/bin/z.dll: $(OBJECTS)
	@printf "%b" "\033[0;36mCreating shared library " $@ "\033[m\n"
	@$(LD) $(LFLAGS) $(OBJECTS) /out:$@
	@mv -f $(VALI_APPLICATION_PATH)/bin/z.lib $(VALI_APPLICATION_PATH)/lib/z.lib
	@cp zconf.h $(VALI_APPLICATION_PATH)/include/zconf.h
	@cp zlib.h $(VALI_APPLICATION_PATH)/include/zlib.h
	
%.o : %.c
	@printf "%b" "\033[0;32mCompiling C source object " $< "\033[m\n"
	@$(CC) -c $(CFLAGS) -o $@ $<

.PHONY: clean
clean:
	@rm -f $(OBJECTS)