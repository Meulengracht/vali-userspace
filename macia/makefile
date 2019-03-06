# Makefile for building a generic userspace application
SOURCES = $(wildcard generator/*.cpp) \
		  $(wildcard interpreter/*.cpp) \
		  $(wildcard lexer/*.cpp) \
		  $(wildcard parser/*.cpp) \
		  $(wildcard shared/*.cpp) \
		  macia.cpp
OBJECTS = $(SOURCES:.cpp=.o)

CXXFLAGS = $(VALI_CXXFLAGS) -O3 $(VALI_INCLUDES)
LFLAGS = /entry:__CrtConsoleEntry $(VALI_LFLAGS) $(VALI_SDK_CXXLIBS)

.PHONY: all
all: $(VALI_APPLICATION_PATH)/bin/macia.app

$(VALI_APPLICATION_PATH)/bin/macia.app: $(OBJECTS)
	@printf "%b" "\033[0;36mCreating application " $@ "\033[m\n"
	@$(LD) $(LFLAGS) $(OBJECTS) /out:$@
	
%.o : %.cpp
	@printf "%b" "\033[0;32mCompiling C++ source object " $< "\033[m\n"
	@$(CXX) -c $(CXXFLAGS) -o $@ $<

.PHONY: clean
clean:
	@rm -f $(OBJECTS)