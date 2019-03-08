# Makefile for building a generic userspace application
ifeq ($(VALI_ARCH),i386)
	ASFLAGS = -f win32 -Xvc
else
	ASFLAGS = -f win64 -Xvc
endif
INCLUDES = $(VALI_INCLUDES) -I$(VALI_DDK_PATH)/include -I. -Igraphics
SHARED_SOURCES = $(wildcard engine/elements/*.cpp) \
				 $(wildcard engine/dialogs/*.cpp) \
				 $(wildcard engine/*.cpp) \
				 $(wildcard scenes/*.cpp) \
				 $(wildcard utils/*.cpp) \
				 $(wildcard *.cpp)
SHARED_OBJECTS = $(SHARED_SOURCES:.cpp=.o)

NVG_SOURCES = engine/backend/nanovg.c
OSMESA_ASM = graphics/opengl/osmesa/display_osmesa_$(VALI_ARCH).s
OSMESA_OBJECTS = $(OSMESA_ASM:.s=.o) $(NVG_SOURCES:.c=.o)

SOFT_SOURCES = 
SOFT_ASM = graphics/displays/fb/display_framebuffer_$(VALI_ARCH).s
SOFT_OBJECTS = $(SOFT_SOURCES:.cpp=.o) $(SOFT_ASM:.s=.o)

CFLAGS = $(VALI_CFLAGS) -O3 -DGL_VIOARR_LOADER -DNANOVG_GL3_IMPLEMENTATION -DFONS_USE_FREETYPE $(INCLUDES)
CXXFLAGS = $(VALI_CXXFLAGS) -O3 -std=c++17 $(INCLUDES)
ifeq ($(MAKECMDGOALS),osmesa)
	OBJECTS = $(SHARED_OBJECTS) $(OSMESA_OBJECTS)
	CXXFLAGS += -D_VIOARR_OSMESA -D_VIOARR_OPENGL -DGL_GLEXT_PROTOTYPES -DGL_VIOARR_LOADER -DNANOVG_GL3_IMPLEMENTATION
	LFLAGS = $(VALI_LFLAGS) /entry:__CrtConsoleEntry $(VALI_SDK_CXXLIBS) gallium-osmesa.lib libfreetype.lib
else ifeq ($(MAKECMDGOALS),soft)
	OBJECTS = $(SHARED_OBJECTS) $(SOFT_OBJECTS)
	LFLAGS = $(VALI_LFLAGS) /entry:__CrtConsoleEntry $(VALI_SDK_CXXLIBS) libpng.lib
else
	OBJECTS = $(SHARED_OBJECTS) $(OSMESA_OBJECTS) $(SOFT_OBJECTS)
endif

.PHONY: all
all: $(VALI_APPLICATION_PATH)/bin/vioarr.app

.PHONY: osmesa
osmesa: all

$(VALI_APPLICATION_PATH)/bin/vioarr.app: $(OBJECTS)
	@printf "%b" "\033[0;36mCreating application " $@ "\033[m\n"
	@$(LD) $(LFLAGS) $(OBJECTS) /out:$@
	
%.o : %.cpp
	@printf "%b" "\033[0;32mCompiling C++ source object " $< "\033[m\n"
	@$(CC) -c $(CXXFLAGS) -o $@ $<

%.o : %.c
	@printf "%b" "\033[0;32mCompiling C source object " $< "\033[m\n"
	@$(CC) -c $(CFLAGS) -o $@ $<

%.o : %.s
	@printf "%b" "\033[0;32mAssembling source object " $< "\033[m\n"
	@$(AS) $(ASFLAGS) $< -o $@

.PHONY: clean
clean:
	@rm -f $(OBJECTS)