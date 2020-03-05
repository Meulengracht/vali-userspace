# Makefile for building a generic userspace application
# Include all the definitions for os

INCLUDES =-I$(VALI_DDK_PATH)/include -I$(VALI_DDK_PATH)/include/wm -Iloader/include
LFLAGS := ddk.lib libgracht.lib

APP_SOURCES_C :=
APP_SOURCES_CXX := object_manager.cpp screen.cpp window_base.cpp application.cpp surface.cpp window_memory.cpp window_buffer.cpp
		         
APP_PROTOCOLS_C := protocols/wm_core_protocol_client.c \
				   protocols/wm_screen_protocol_client.c \
				   protocols/wm_memory_protocol_client.c \
				   protocols/wm_memory_pool_protocol_client.c \
				   protocols/wm_buffer_protocol_client.c \
				   protocols/wm_surface_protocol_client.c \
				   protocols/hid_events_protocol_client.c

APP_DEFINES := -DGL_VIOARR_LOADER -DGL_GLEXT_PROTOTYPES -DNANOVG_GL3_IMPLEMENTATION -DFONS_USE_FREETYPE

APP_CFLAGS = $(VALI_CFLAGS) $(CFLAGS) $(VALI_INCLUDES) $(INCLUDES) $(APP_DEFINES)
APP_CXXFLAGS = $(VALI_CXXFLAGS) $(CXXFLAGS) $(VALI_INCLUDES) $(INCLUDES) $(APP_DEFINES)
APP_LFLAGS = /lib

APP_OBJECTS = $(APP_PROTOCOLS_C:.c=.o) $(APP_SOURCES_CXX:.cpp=.o) $(APP_SOURCES_C:.c=.o)

.PHONY: all
all: protocols $(VALI_APPLICATION_PATH)/lib/asgaard.lib install

.PHONY: install
install:
	@mkdir -p $(VALI_APPLICATION_PATH)/include/asgaard
	@cp -r include/* $(VALI_APPLICATION_PATH)/include/asgaard/

$(VALI_APPLICATION_PATH)/lib/asgaard.lib: $(APP_OBJECTS)
	@printf "%b" "\033[0;36mCreating application " $@ "\033[m\n"
	@$(LD) $(APP_LFLAGS) $(APP_OBJECTS) /out:$@

protocols:
	@mkdir -p $@

protocols/wm_core_protocol_client.c: ../protocols/wm_protocol.xml
	@printf "%b" "\033[0;36mRegenerating protocol " $@ "\033[m\n"
	python $(VALI_DDK_PATH)/protocols/gracht_generator.py --protocol $< --lang-c --client --out ./protocols/

protocols/wm_screen_protocol_client.c: ../protocols/wm_protocol.xml
	@printf "%b" "\033[0;36mRegenerating protocol " $@ "\033[m\n"
	python $(VALI_DDK_PATH)/protocols/gracht_generator.py --protocol $< --lang-c --client --out ./protocols/

protocols/wm_memory_protocol_client.c: ../protocols/wm_protocol.xml
	@printf "%b" "\033[0;36mRegenerating protocol " $@ "\033[m\n"
	python $(VALI_DDK_PATH)/protocols/gracht_generator.py --protocol $< --lang-c --client --out ./protocols/

protocols/wm_memory_pool_protocol_client.c: ../protocols/wm_protocol.xml
	@printf "%b" "\033[0;36mRegenerating protocol " $@ "\033[m\n"
	python $(VALI_DDK_PATH)/protocols/gracht_generator.py --protocol $< --lang-c --client --out ./protocols/

protocols/wm_buffer_protocol_client.c: ../protocols/wm_protocol.xml
	@printf "%b" "\033[0;36mRegenerating protocol " $@ "\033[m\n"
	python $(VALI_DDK_PATH)/protocols/gracht_generator.py --protocol $< --lang-c --client --out ./protocols/

protocols/wm_surface_protocol_client.c: ../protocols/wm_protocol.xml
	@printf "%b" "\033[0;36mRegenerating protocol " $@ "\033[m\n"
	python $(VALI_DDK_PATH)/protocols/gracht_generator.py --protocol $< --lang-c --client --out ./protocols/

protocols/hid_events_protocol_client.c: $(VALI_DDK_PATH)/protocols/hid_protocol.xml
	@printf "%b" "\033[0;36mRegenerating protocol " $@ "\033[m\n"
	python $(VALI_DDK_PATH)/protocols/gracht_generator.py --protocol $< --lang-c --client --out ./protocols/

%.o : %.cpp
	@printf "%b" "\033[0;32mCompiling C++ source object " $< "\033[m\n"
	@$(CC) -c $(APP_CXXFLAGS) -o $@ $<

%.o : %.c
	@printf "%b" "\033[0;32mCompiling C source object " $< "\033[m\n"
	@$(CC) -c $(APP_CFLAGS) -o $@ $<

.PHONY: clean
clean:
	@rm -f protocols/*
	@rm -f $(APP_OBJECTS)
