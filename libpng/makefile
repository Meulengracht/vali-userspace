# Makefile for building a generic userspace application or library
SOURCES = $(wildcard ./*.c)
OBJECTS = $(SOURCES:.c=.o)

CONFIG_FLAGS = 
CFLAGS = $(VALI_CFLAGS) $(CONFIG_FLAGS) $(VALI_INCLUDES)
LFLAGS = /version:1.6 /def:libpng.def /entry:__CrtLibraryEntry /dll $(VALI_LFLAGS) $(VALI_SDK_CLIBS) zlib.lib

.PHONY: all
all: $(VALI_APPLICATION_PATH)/bin/libpng.dll

$(VALI_APPLICATION_PATH)/bin/libpng.dll: $(OBJECTS)
	@printf "%b" "\033[0;36mCreating shared library " $@ "\033[m\n"
	@$(LD) $(LFLAGS) $(OBJECTS) /out:$@
	@mv -f $(VALI_APPLICATION_PATH)/bin/libpng.lib $(VALI_APPLICATION_PATH)/lib/libpng.lib
	@cp pnglibconf.h $(VALI_APPLICATION_PATH)/include/pnglibconf.h
	@cp pngconf.h $(VALI_APPLICATION_PATH)/include/pngconf.h
	@cp png.h $(VALI_APPLICATION_PATH)/include/png.h
	
%.o : %.c
	@printf "%b" "\033[0;32mCompiling C source object " $< "\033[m\n"
	@$(CC) -c $(CFLAGS) -o $@ $<

.PHONY: clean
clean:
	@rm -f $(OBJECTS)