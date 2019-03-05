# Makefile for building userspace applications, the prerequisites for building
# are environmental variables CROSS, VALI_ARCH and VALI_SDK_PATH, that must be set to their
# appropriate values.

ifndef VALI_ARCH
$(error VALI_ARCH is not set)
endif
ifndef VALI_SDK_PATH
$(error VALI_SDK_PATH is not set)
endif
ifndef CROSS
$(error CROSS is not set)
endif

# Setup project tools
export CC := $(CROSS)/bin/clang
export CXX := $(CROSS)/bin/clang++
export LD := $(CROSS)/bin/lld-link
export LIB := $(CROSS)/bin/llvm-lib
export AS := nasm

export VALI_BUILD_ROOT = $(shell pwd)
export VALI_APPLICATION_PATH = $(VALI_BUILD_ROOT)/package_application
export VALI_INCLUDES = -I$(VALI_SDK_PATH)/include/cxx -I$(VALI_SDK_PATH)/include -I$(VALI_APPLICATION_PATH)/include
export VALI_LIBRARIES = -LIBPATH:$(VALI_SDK_PATH)/lib -LIBPATH:$(VALI_APPLICATION_PATH)/lib
export VALI_SDK_CLIBS = libcrt.lib libclang.lib libc.lib libunwind.lib libm.lib
export VALI_SDK_CXXLIBS = libcxx.lib libclang.lib libc.lib libunwind.lib libm.lib

# Setup default build rules
include config/$(VALI_ARCH).mk

export VALI_LFLAGS = $(arch_lflags) /nodefaultlib /subsystem:native /lldmap $(VALI_LIBRARIES)
export VALI_CFLAGS = $(shared_flags) $(arch_flags) $(disable_warnings)
export VALI_CXXFLAGS = -std=c++17 $(shared_flags) $(arch_flags) $(disable_warnings)

###################################
##### BUILD TARGETS           #####
###################################
#   build_cpptest build_wintest   build_mesa build_glm build_vioarr_osmesa

.PHONY: build
build: package_application build_zlib build_libpng build_libfreetype build_macia build_alumni build_llvm
	
.PHONY:
package: build
	@cd $(VALI_APPLICATION_PATH); zip -r vali-apps-$(VALI_ARCH).zip .
	@mv $(VALI_APPLICATION_PATH)/vali-apps-$(VALI_ARCH).zip .

package_application:
	@mkdir -p $@
	@mkdir -p $@/bin
	@mkdir -p $@/include
	@mkdir -p $@/lib

.PHONY: build_zlib
build_zlib:
	@printf "%b" "\033[1;35mChecking if zlib needs to be built\033[m\n"
	@$(MAKE) -s -C zlib -f makefile

.PHONY: build_libpng
build_libpng:
	@printf "%b" "\033[1;35mChecking if libpng needs to be built\033[m\n"
	@$(MAKE) -s -C libpng -f makefile

.PHONY: build_libfreetype
build_libfreetype:
	@printf "%b" "\033[1;35mChecking if freetype needs to be built\033[m\n"
	@$(MAKE) -s -C freetype -f makefile

.PHONY: build_cpptest
build_cpptest:
	@printf "%b" "\033[1;35mChecking if cpptest needs to be built\033[m\n"
	@$(MAKE) -s -C cpptest -f makefile

.PHONY: build_wintest
build_wintest:
	@printf "%b" "\033[1;35mChecking if wintest needs to be built\033[m\n"
	@$(MAKE) -s -C wintest -f makefile

.PHONY: build_alumni
build_alumni:
	@printf "%b" "\033[1;35mChecking if alumni needs to be built\033[m\n"
	@$(MAKE) -s -C alumni -f makefile

.PHONY: build_macia
build_macia:
	@printf "%b" "\033[1;35mChecking if macia needs to be built\033[m\n"
	@$(MAKE) -s -C macia -f makefile

.PHONY: build_vioarr_soft
build_vioarr_soft:
	@printf "%b" "\033[1;35mChecking if vioarr needs to be built\033[m\n"
	@$(MAKE) -s -C vioarr -f makefile

llvm-build:
	mkdir -p llvm-build
	cd llvm-build && cmake -G "Unix Makefiles" -DCMAKE_INSTALL_PREFIX=$(VALI_APPLICATION_PATH) -DCMAKE_BUILD_TYPE=Release -DCMAKE_TOOLCHAIN_FILE=../llvm/cmake/platforms/Vali.cmake ../llvm

.PHONY: build_llvm
build_llvm: llvm-build
	$(eval CPU_COUNT = $(shell nproc))
	cd llvm-build && make -j$(CPU_COUNT) && make install
	@cp llvm-build/bin/*.lib $(VALI_APPLICATION_PATH)/lib/
	@mv $(VALI_APPLICATION_PATH)/lib/*.dll $(VALI_APPLICATION_PATH)/bin/

.PHONY: build_mesa
build_mesa:
	cd mesa && make && make install

.PHONY: build_glm
build_glm:
	@printf "%b" "\033[1;35mInstalling GLM\033[m\n"
	@cp -r glm/glm/. $(VALI_APPLICATION_PATH)/include/glm
	@rm $(VALI_APPLICATION_PATH)/include/glm/CMakeLists.txt

.PHONY: build_vioarr_osmesa
build_vioarr_osmesa:
	@printf "%b" "\033[1;35mChecking if vioarr needs to be built\033[m\n"
	@$(MAKE) -s -C vioarr -f makefile osmesa

.PHONY: clean
clean:
	@$(MAKE) -s -C zlib -f makefile clean
	@$(MAKE) -s -C libpng -f makefile clean
	@$(MAKE) -s -C freetype -f makefile clean
	@$(MAKE) -s -C macia -f makefile clean
	@$(MAKE) -s -C alumni -f makefile clean
	#@$(MAKE) -s -C mesa -f makefile clean
	#@$(MAKE) -s -C vioarr -f makefile clean
	#@$(MAKE) -s -C cpptest -f makefile clean
	#@$(MAKE) -s -C wintest -f makefile clean
	@rm -rf package_application
	@rm -rf llvm-build