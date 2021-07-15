# Makefile for building userspace applications, the prerequisites for building
# are environmental variables CROSS, VALI_ARCH and VALI_SDK_PATH, that must be set to their
# appropriate values.

ifndef CROSS
$(error CROSS is not set)
endif
ifndef VALI_ARCH
$(error VALI_ARCH is not set)
endif

ifndef VALI_SDK_PATH
$(error VALI_SDK_PATH is not set)
endif

ifndef VALI_APPLICATION_PATH
export VALI_BUILD_ROOT = $(shell pwd)
export VALI_APPLICATION_PATH = $(VALI_BUILD_ROOT)/package_application_$(VALI_ARCH)
endif

ifndef VALI_VERSION
VALI_VERSION = local
endif

# Setup project tools
export CC := $(CROSS)/bin/clang
export CXX := $(CROSS)/bin/clang++
export LD := $(CROSS)/bin/lld-link
export LIB := $(CROSS)/bin/llvm-lib
export AS := nasm

export VALI_INCLUDES = -I${VALI_APPLICATION_PATH}/include/c++/v1 -I$(VALI_SDK_PATH)/include/clang-9.0.0 -I$(VALI_APPLICATION_PATH)/include -I$(VALI_SDK_PATH)/include 
export VALI_LIBRARIES = -LIBPATH:$(VALI_SDK_PATH)/lib -LIBPATH:$(VALI_APPLICATION_PATH)/lib
export VALI_SDK_CLIBS = static_libcrt.lib static_librt.lib c.lib m.lib
export VALI_SDK_CXXLIBS = $(VALI_SDK_CLIBS) static_c++.lib static_c++abi.lib unwind.lib

# Setup default build rules
include config/$(VALI_ARCH).mk

export VALI_LFLAGS = $(arch_lflags) /nodefaultlib /subsystem:native /lldmap $(VALI_LIBRARIES)
export VALI_CFLAGS = $(shared_flags) $(arch_flags)
export VALI_CXXFLAGS = $(shared_flags) -static $(arch_flags)

###################################
##### BUILD TARGETS           #####
###################################
.PHONY: init
init:
	git submodule update --init
	cd llvm/projects && git clone https://github.com/Meulengracht/libcxx.git
	cd llvm/projects && git clone https://github.com/Meulengracht/libcxxabi.git
	cd llvm/projects && git clone https://github.com/Meulengracht/libunwind.git

.PHONY: build
build: $(VALI_APPLICATION_PATH) build_zlib build_libpng build_lua build_libfreetype build_llvm build_apps build_wm
	
.PHONY: package
package: build
	@cd $(VALI_APPLICATION_PATH); zip -r vali-apps-$(VALI_VERSION)-$(VALI_ARCH).zip .
	@mv $(VALI_APPLICATION_PATH)/vali-apps-$(VALI_VERSION)-$(VALI_ARCH).zip .

$(VALI_APPLICATION_PATH):
	@mkdir -p $(VALI_APPLICATION_PATH)
	@mkdir -p $(VALI_APPLICATION_PATH)/bin
	@mkdir -p $(VALI_APPLICATION_PATH)/include
	@mkdir -p $(VALI_APPLICATION_PATH)/lib

.PHONY:
build_rt:
	@$(MAKE) -s -C libcxx -f makefile

.PHONY: build_apps
build_apps: build_asgaard build_macia build_wintest build_alumni build_doom

.PHONY: build_wm
build_wm: build_mesa build_glm build_vioarr

.PHONY: build_sdl_apps
build_sdl_apps: build_sdl build_sdlttf build_sdlimage build_sdlmixer build_sdlshooter build_lite

# Repositories
llvm/projects/libcxx:
	cd llvm/projects && git clone https://github.com/Meulengracht/libcxx.git

llvm/projects/libcxxabi:
	cd llvm/projects && git clone https://github.com/Meulengracht/libcxxabi.git

llvm/projects/libunwind:
	cd llvm/projects && git clone https://github.com/Meulengracht/libunwind.git

llvm-build: llvm/projects/libcxx llvm/projects/libcxxabi llvm/projects/libunwind
	mkdir -p llvm-build
	cd llvm-build && LIBCXX_BOOTSTRAP=true cmake -G "Unix Makefiles" \
		-DLLVM_BOOTSTRAP_RUNTIME=ON \
		-DLLVM_ENABLE_DUMP=ON \
		-DLIBUNWIND_INSTALL_PREFIX=$(VALI_APPLICATION_PATH)/ \
		-DLIBUNWIND_INCLUDE_DOCS=OFF \
		-DLIBCXXABI_INSTALL_PREFIX=$(VALI_APPLICATION_PATH)/ \
		-DLIBCXXABI_VALI_BOOTSTRAP=ON \
		-DLIBCXXABI_HERMETIC_STATIC_LIBRARY=ON \
		-DLIBCXXABI_USE_LLVM_UNWINDER=ON \
		-DLIBCXXABI_HAS_EXTERNAL_THREAD_API=ON \
		-DLIBCXXABI_ENABLE_NEW_DELETE_DEFINITIONS=OFF \
		-DLIBCXX_INSTALL_PREFIX=$(VALI_APPLICATION_PATH)/ \
		-DLIBCXX_VALI_BOOTSTRAP=ON \
		-DLIBCXX_HERMETIC_STATIC_LIBRARY=ON \
		-DLIBCXX_ENABLE_STATIC_ABI_LIBRARY=ON \
		-DLIBCXX_HAS_EXTERNAL_THREAD_API=ON \
		-DLIBCXX_ENABLE_EXPERIMENTAL_LIBRARY=OFF \
		-DLIBCXX_INCLUDE_BENCHMARKS=OFF \
		-DLIBCXX_INCLUDE_TESTS=OFF \
		-DLIBCXX_INCLUDE_BENCHMARKS=OFF \
		-DLIBCXX_ENABLE_EXPERIMENTAL_LIBRARY=OFF \
		-DCMAKE_INSTALL_PREFIX=$(VALI_APPLICATION_PATH) \
		-DCMAKE_BUILD_TYPE=Release \
		-DCMAKE_TOOLCHAIN_FILE=../llvm/cmake/platforms/Vali.cmake \
		../llvm

.PHONY: build_llvm
build_llvm: llvm-build
	$(eval CPU_COUNT = $(shell nproc))
	cd llvm-build && make -j$(CPU_COUNT) && make install
	@-mv llvm-build/bin/*.lib $(VALI_APPLICATION_PATH)/lib/
	@-mv $(VALI_APPLICATION_PATH)/lib/*.dll $(VALI_APPLICATION_PATH)/bin/

.PHONY: build_mesa
build_mesa:
	#$(eval CPU_COUNT = $(shell nproc))
	cd mesa && make && make install

.PHONY: clean_mesa
clean_mesa:
	cd mesa && make clean

.PHONY: clean_asgaard
clean_asgaard:
	@rm -rf asgaard-build

.PHONY: clean_heimdall
clean_heimdall:
	@rm -rf heimdall-build

.PHONY: clean_doom
clean_doom:
	@rm -rf doom-build

.PHONY: clean_lua
clean_lua:
	@rm -rf lua-build

.PHONY: clean_sdl
clean_sdl:
	@rm -rf sdl-build

.PHONY: clean_sdlimage
clean_sdlimage:
	@rm -rf sdlimage-build

.PHONY: clean_sdlttf
clean_sdlttf:
	@rm -rf sdlttf-build

.PHONY: clean_sdlmixer
clean_sdlmixer:
	@rm -rf sdlmixer-build

.PHONY: clean_sdlshooter
clean_sdlshooter:
	@rm -rf sdlshooter-build

.PHONY: clean_lite
clean_lite:
	@rm -rf lite-build

.PHONY: clean_libzip
clean_libzip:
	@rm -rf libzip-build

.PHONY: clean_alumni
clean_alumni:
	cd alumni && make clean

.PHONY: build_glm
build_glm:
	@printf "%b" "\033[1;35mInstalling GLM\033[m\n"
	@cp -r glm/glm/. $(VALI_APPLICATION_PATH)/include/glm
	@rm $(VALI_APPLICATION_PATH)/include/glm/CMakeLists.txt

.PHONY: build_vioarr
build_vioarr:
	@printf "%b" "\033[1;35mChecking if vioarr needs to be built\033[m\n"
	@$(MAKE) -s -C vioarr -f makefile

.PHONY: clean_vioarr
clean_vioarr:
	@printf "%b" "\033[1;35mChecking if vioarr needs to be built\033[m\n"
	@$(MAKE) -s -C vioarr -f makefile clean

.PHONY: clean_apps
clean_apps:
	@$(MAKE) -s -C zlib -f makefile clean
	@$(MAKE) -s -C libpng -f makefile clean
	@$(MAKE) -s -C freetype -f makefile clean
	@$(MAKE) -s -C macia -f makefile clean
	@$(MAKE) -s -C alumni -f makefile clean
	@$(MAKE) -s -C wintest -f makefile clean
	@rm -rf asgaard-build
	@rm -rf heimdall-build

.PHONY: clean
clean:
	@$(MAKE) -s -C zlib -f makefile clean
	@$(MAKE) -s -C libpng -f makefile clean
	@$(MAKE) -s -C freetype -f makefile clean
	@$(MAKE) -s -C macia -f makefile clean
	@$(MAKE) -s -C alumni -f makefile clean
	@$(MAKE) -s -C mesa -f makefile clean
	@$(MAKE) -s -C vioarr -f makefile clean
	@$(MAKE) -s -C wintest -f makefile clean
	@rm -rf asgaard-build
	@rm -rf heimdall-build
	@rm -rf llvm-build
	@rm -rf doom-build
	@rm -rf lua-build
	@rm -rf sdl-build
	@rm -rf sdlimage-build
	@rm -rf sdlttf-build
	@rm -rf sdlmixer-build
	@rm -rf sdlshooter-build
	@rm -rf lite-build
	@rm -rf libzip-build
	@rm -rf $(VALI_APPLICATION_PATH)
