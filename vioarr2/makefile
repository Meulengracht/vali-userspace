# Makefile for building a generic userspace application
# Include all the definitions for os

INCLUDES =-I$(VALI_DDK_PATH)/include -I$(VALI_DDK_PATH)/include/wm -Iloader/include

APP_SOURCES_ASM := 
APP_SOURCES_C := loader/src/glad.c \
				 $(wildcard engine/backend/*.c) \
				 $(wildcard engine/*.c) \
		         $(wildcard *.c)
LFLAGS := ddk.lib libwm.lib libfreetype.lib gallium-osmesa.lib

ifeq ($(VALI_ARCH),i386)
	ASFLAGS = -f win32 -Xvc
else
	ASFLAGS = -f win64 -Xvc
endif

APP_DEFINES := -DGL_VIOARR_LOADER -DGL_GLEXT_PROTOTYPES -DNANOVG_GL3_IMPLEMENTATION -DFONS_USE_FREETYPE
ifdef VALI_HEADLESS
APP_SOURCES_C += engine/screen/vioarr_screen_headless.c
APP_DEFINES += -D__VIOARR_CONFIG_RENDERER_HEADLESS
else
APP_SOURCES_C += engine/screen/vioarr_screen_osmesa.c
APP_SOURCES_ASM += engine/screen/vioarr_screen_$(VALI_ARCH).s
APP_DEFINES += -D__VIOARR_CONFIG_RENDERER_OSMESA
endif

APP_CFLAGS = $(VALI_CFLAGS) $(CFLAGS) $(VALI_INCLUDES) $(INCLUDES) $(APP_DEFINES)
APP_CXXFLAGS = $(VALI_CXXFLAGS) $(CXXFLAGS) $(VALI_INCLUDES) $(INCLUDES) $(APP_DEFINES)
APP_LFLAGS = /entry:__CrtConsoleEntry $(VALI_LFLAGS) $(VALI_SDK_CXXLIBS) $(LFLAGS)

APP_OBJECTS = $(APP_SOURCES_CXX:.cpp=.o) $(APP_SOURCES_C:.c=.o) $(APP_SOURCES_ASM:.s=.o)

.PHONE: all
all: protocols/wm_core_protocol.h protocols/hid_events_protocol.h $(VALI_APPLICATION_PATH)/bin/vioarr.app

$(VALI_APPLICATION_PATH)/bin/vioarr.app: $(APP_OBJECTS)
	@printf "%b" "\033[0;36mCreating application " $@ "\033[m\n"
	@$(LD) $(APP_LFLAGS) $(APP_OBJECTS) /out:$@

protocols/wm_core_protocol.h: ../protocols/wm_protocol.xml
	@printf "%b" "\033[0;36mRegenerating protocol " $@ "\033[m\n"
	python $(VALI_DDK_PATH)/protocols/gracht_generator.py --protocol $< --lang-c --server --out ./protocols/

protocols/hid_events_protocol.h: $(VALI_DDK_PATH)/protocols/hid_protocol.xml
	@printf "%b" "\033[0;36mRegenerating protocol " $@ "\033[m\n"
	python $(VALI_DDK_PATH)/protocols/gracht_generator.py --protocol $< --lang-c --server --out ./protocols/

loader/src/glad.c:
	@printf "%b" "\033[0;36mRegenerating opengl loader files " $@ "\033[m\n"
	@pip install --upgrade git+https://github.com/meulengracht/glad.git#egg=glad
	@python -m glad --generator=c --profile=core --out-path=loader

%.o : %.cpp
	@printf "%b" "\033[0;32mCompiling C++ source object " $< "\033[m\n"
	@$(CC) -c $(APP_CXXFLAGS) -o $@ $<

%.o : %.c
	@printf "%b" "\033[0;32mCompiling C source object " $< "\033[m\n"
	@$(CC) -c $(APP_CFLAGS) -o $@ $<

%.o : %.s
	@printf "%b" "\033[0;32mAssembling source object " $< "\033[m\n"
	@$(AS) $(ASFLAGS) $< -o $@

.PHONY: clean
clean:
	@rm -rf loader/*
	@rm -f protocols/*
	@rm -f $(APP_OBJECTS)
