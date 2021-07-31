# Cross toolchain configuration for using clang-cl on non-Windows hosts to
# target MSVC.
#
# Usage:
# cmake -G Ninja
#    -DCMAKE_TOOLCHAIN_FILE=/path/to/this/file
#    -DHOST_ARCH=[aarch64|arm64|armv7|arm|i686|x86|x86_64|x64]
#    -DMSVC_BASE=/path/to/MSVC/system/libraries/and/includes
#    -DWINSDK_BASE=/path/to/windows-sdk
#    -DWINSDK_VER=windows sdk version folder name
#
# VALI_ARCH:
#    The architecture to build for.
#

# Sanitize expected environmental variables
if(NOT DEFINED ENV{CROSS})
    message(FATAL_ERROR "Please set the CROSS environmental variable to the path of the Vali Crosscompiler.")
endif()

if(NOT DEFINED ENV{VALI_ARCH})
    message(WARNING "VALI_ARCH environmental variable was not set, defauling to amd64.")
    set(ENV{VALI_ARCH} amd64)
endif()

if(NOT DEFINED ENV{VALI_SDK_PATH})
    message(FATAL_ERROR "Please set the VALI_SDK_PATH environmental variable to the path of the Vali SDK.")
endif()

# Setup expected environment variables
set(ENV{VALI_INCLUDES}  "-I$ENV{VALI_SDK_PATH}/include/clang-9.0.0 -I$ENV{VALI_SDK_PATH}/include")
set(ENV{VALI_LIBRARIES} "-LIBPATH:$ENV{VALI_SDK_PATH}/lib")
set(ENV{VALI_SDK_CLIBS}   "static_libcrt.lib static_librt.lib c.lib m.lib")
set(ENV{VALI_SDK_CXXLIBS} "$ENV{VALI_SDK_CLIBS} static_c++.lib static_c++abi.lib unwind.lib")

if("$ENV{VALI_ARCH}" STREQUAL "i386")
    set(ENV{VALI_LFLAGS} "/machine:X86 /nodefaultlib /subsystem:native /lldmap $ENV{VALI_LIBRARIES}")
else()
    set(ENV{VALI_LFLAGS} "/machine:X64 /nodefaultlib /subsystem:native /lldmap $ENV{VALI_LIBRARIES}")
endif()

# Setup environment stuff for cmake configuration
set(CMAKE_SYSTEM_NAME vali-cross)
set(CMAKE_CROSSCOMPILING OFF CACHE BOOL "")
set(CMAKE_C_COMPILER "$ENV{CROSS}/bin/clang" CACHE FILEPATH "")
set(CMAKE_CXX_COMPILER "$ENV{CROSS}/bin/clang++" CACHE FILEPATH "")
set(CMAKE_LINKER "$ENV{CROSS}/bin/lld-link" CACHE FILEPATH "")
set(CMAKE_AR "$ENV{CROSS}/bin/llvm-ar" CACHE FILEPATH "")
set(CMAKE_RANLIB "$ENV{CROSS}/bin/llvm-ranlib" CACHE FILEPATH "")
set(VERBOSE 1)

set(CMAKE_C_COMPILER_WORKS 1)
set(CMAKE_CXX_COMPILER_WORKS 1)

# Setup shared compile flags to make compilation succeed
set(VALI_COMPILE_FLAGS -U_WIN32 -fms-extensions -nostdlib -nostdinc -static -DMOLLENOS)
if("$ENV{VALI_ARCH}" STREQUAL "i386")
    set(VALI_COMPILE_FLAGS ${VALI_COMPILE_FLAGS} -Di386 -D__i386__ -m32 --target=i386-pc-win32-itanium-coff)
else()
    set(VALI_COMPILE_FLAGS ${VALI_COMPILE_FLAGS} -Damd64 -D__amd64__ -D__x86_64__ -m64 -fdwarf-exceptions --target=amd64-pc-win32-itanium-coff)
endif()
string(REPLACE ";" " " VALI_COMPILE_FLAGS "${VALI_COMPILE_FLAGS}")

# We need to preserve any flags that were passed in by the user. However, we
# can't append to CMAKE_C_FLAGS and friends directly, because toolchain files
# will be re-invoked on each reconfigure and therefore need to be idempotent.
# The assignments to the _INITIAL cache variables don't use FORCE, so they'll
# only be populated on the initial configure, and their values won't change
# afterward.
set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} ${VALI_COMPILE_FLAGS}" CACHE STRING "" FORCE)
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} ${VALI_COMPILE_FLAGS}" CACHE STRING "" FORCE)
