cmake_minimum_required (VERSION 3.14)
project (ValiUserspace)

include(ExternalProject)

set (CMAKE_MODULE_PATH ${CMAKE_MODULE_PATH} "${CMAKE_SOURCE_DIR}/cmake/modules/")
set (CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/bin)
set (CMAKE_LIBRARY_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/bin)
set (CMAKE_ARCHIVE_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/lib)

if (WIN32)
    # todo
elseif (UNIX AND NOT APPLE)
    # todo
elseif (APPLE)
    # todo 
elseif (MOLLENOS)
    find_program(MAKE_EXECUTABLE 
        NAMES gmake mingw32-make make 
        NAMES_PER_DIR 
        DOC "GNU Make"
    )
########################################
# Build platform runtime for MollenOS
# Must be invoked with -DCMAKE_TOOLCHAIN_FILE=../config/Vali.cmake
    execute_process(
        COMMAND ${CMAKE_COMMAND} ${CMAKE_SOURCE_DIR}/llvm -DLLVM_BOOTSTRAP_RUNTIME=ON -DLLVM_ENABLE_DUMP=ON -DLIBUNWIND_INSTALL_PREFIX=${CMAKE_INSTALL_PREFIX} -DLIBUNWIND_INCLUDE_DOCS=OFF -DLIBCXXABI_INSTALL_PREFIX=${CMAKE_INSTALL_PREFIX} -DLIBCXXABI_VALI_BOOTSTRAP=ON -DLIBCXXABI_HERMETIC_STATIC_LIBRARY=ON -DLIBCXXABI_USE_LLVM_UNWINDER=ON -DLIBCXXABI_HAS_EXTERNAL_THREAD_API=ON -DLIBCXXABI_ENABLE_NEW_DELETE_DEFINITIONS=OFF -DLIBCXX_INSTALL_PREFIX=${CMAKE_INSTALL_PREFIX} -DLIBCXX_VALI_BOOTSTRAP=ON -DLIBCXX_HERMETIC_STATIC_LIBRARY=ON -DLIBCXX_ENABLE_STATIC_ABI_LIBRARY=ON -DLIBCXX_HAS_EXTERNAL_THREAD_API=ON -DLIBCXX_ENABLE_EXPERIMENTAL_LIBRARY=OFF -DLIBCXX_INCLUDE_BENCHMARKS=OFF -DLIBCXX_INCLUDE_TESTS=OFF -DCMAKE_INSTALL_PREFIX=${CMAKE_INSTALL_PREFIX} -DCMAKE_BUILD_TYPE=Release -DCMAKE_TOOLCHAIN_FILE=cmake/platforms/Vali.cmake
        WORKING_DIRECTORY ${CMAKE_BINARY_DIR}/llvm
    )
    execute_process(
        COMMAND ${CMAKE_COMMAND} --build ${CMAKE_BINARY_DIR}/llvm
    )
    #set(LLVM_BOOTSTRAP_RUNTIME ON CACHE BOOL "Set bootstrap runtime")
    #set(LLVM_ENABLE_DUMP ON CACHE BOOL "Enable dump functionality")
    #set(LIBUNWIND_INSTALL_PREFIX ${CMAKE_INSTALL_PREFIX})
    #set(LIBUNWIND_INCLUDE_DOCS OFF CACHE BOOL "Disable doc build")
    #set(LIBCXXABI_INSTALL_PREFIX ${CMAKE_INSTALL_PREFIX})
    #set(LIBCXXABI_VALI_BOOTSTRAP ON CACHE BOOL "Enable bootstrap build for vali")
    #set(LIBCXXABI_HERMETIC_STATIC_LIBRARY ON CACHE BOOL "Build libcxxabi as static library")
    #set(LIBCXXABI_USE_LLVM_UNWINDER ON CACHE BOOL "Use the LLVM unwinder")
    #set(LIBCXXABI_HAS_EXTERNAL_THREAD_API ON CACHE BOOL "External threading is enabled")
    #set(LIBCXXABI_ENABLE_NEW_DELETE_DEFINITIONS OFF CACHE BOOL "New/delete is already provided")
    #set(LIBCXX_INSTALL_PREFIX ${CMAKE_INSTALL_PREFIX})
    #set(LIBCXX_VALI_BOOTSTRAP ON CACHE BOOL "Enable bootstrap build for vali")
    #set(LIBCXX_HERMETIC_STATIC_LIBRARY ON CACHE BOOL "Build libcxx as static library")
    #set(LIBCXX_ENABLE_STATIC_ABI_LIBRARY ON CACHE BOOL "Enable the static abi library")
    #set(LIBCXX_HAS_EXTERNAL_THREAD_API ON CACHE BOOL "We provide our own threading")
    #set(LIBCXX_ENABLE_EXPERIMENTAL_LIBRARY OFF CACHE BOOL "Do not enable experimental namespace")
    #set(LIBCXX_INCLUDE_BENCHMARKS OFF CACHE BOOL "Do not build benchmarks")
    #set(LIBCXX_INCLUDE_TESTS OFF CACHE BOOL "Do not build tests")
    #add_subdirectory (llvm)

########################################
# Build OSMesa for OpenGL support to Vioarr
    ExternalProject_Add(Mesa3D
        SOURCE_DIR mesa
        CONFIGURE_COMMAND ""
        BUILD_COMMAND ${MAKE_EXECUTABLE}
        INSTALL_COMMAND ${MAKE_EXECUTABLE} install
        BUILD_BYPRODUCTS ${Scotch_LIBRARIES}  # Ninja needs this
    )
endif ()

########################################
# Add shared projects for all platforms
add_subdirectory (lua)
#add_subdirectory (macia)

########################################
# Build the vioarr project and it's dependencies last
add_subdirectory (vioarr)
add_subdirectory (doom)

set(SDL_SHARED_ENABLED_BY_DEFAULT ON CACHE BOOL "Enable SDL shared library build")
set(SDL_AUDIO ON CACHE BOOL "Enable SDL audio subsystem")
add_subdirectory (SDL)

set(PNG_BUILD_ZLIB ON CACHE BOOL "Enable zlib build for SDL_image")
add_subdirectory (SDL_image)
add_subdirectory (SDL_mixer)
add_subdirectory (SDL_ttf)
add_subdirectory (sdlshooter)
add_subdirectory (lite)
