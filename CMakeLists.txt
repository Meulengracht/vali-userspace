cmake_minimum_required (VERSION 3.14)
project (ValiUserspace)

include(ExternalProject)
include(ProcessorCount)

set (CMAKE_MODULE_PATH ${CMAKE_MODULE_PATH} "${CMAKE_SOURCE_DIR}/cmake/modules/")
set (CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/bin)
set (CMAKE_LIBRARY_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/bin)
set (CMAKE_ARCHIVE_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/lib)

# initialize the default install path to be the build directory unless
# VALI_APPLICATION_PATH is set.
if (CMAKE_INSTALL_PREFIX_INITIALIZED_TO_DEFAULT)
    if (NOT DEFINED ENV{VALI_APPLICATION_PATH})
        file(MAKE_DIRECTORY ${CMAKE_BINARY_DIR}/install)
        set(CMAKE_INSTALL_PREFIX "${CMAKE_BINARY_DIR}/install" CACHE PATH "Update the default install path" FORCE)
    else()
        set(CMAKE_INSTALL_PREFIX "$ENV{VALI_APPLICATION_PATH}" CACHE PATH "Update the default install path" FORCE)
    endif()
endif(CMAKE_INSTALL_PREFIX_INITIALIZED_TO_DEFAULT)
message (STATUS "Install directory: ${CMAKE_INSTALL_PREFIX}")

# retrieve the processor count to allow for quicker sub builds
ProcessorCount(NUM_CORES)

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
    file(MAKE_DIRECTORY ${CMAKE_BINARY_DIR}/llvm-build)
    if (EXISTS ${CMAKE_BINARY_DIR}/llvm-build/bin/lli.run)
        message(STATUS "LLVM seems to be built already, assuming all is fine and skipping")
    else()
        set(ENV{LIBCXX_BOOTSTRAP} "true")
        execute_process(
            COMMAND "${CMAKE_COMMAND} -B ${CMAKE_BINARY_DIR}/llvm-build -DLLVM_ENABLE_PROJECTS=\"libcxx;libcxxabi;libunwind\" -DLLVM_INCLUDE_TESTS=Off -DLLVM_INCLUDE_EXAMPLES=Off -DLLVM_BOOTSTRAP_RUNTIME=ON -DLLVM_ENABLE_DUMP=ON -DCMAKE_INSTALL_PREFIX=${CMAKE_INSTALL_PREFIX}/ -DLIBCXXABI_VALI_BOOTSTRAP=ON -DLIBCXXABI_HERMETIC_STATIC_LIBRARY=ON -DLIBCXXABI_USE_LLVM_UNWINDER=ON -DLIBCXXABI_HAS_EXTERNAL_THREAD_API=ON -DLIBCXXABI_ENABLE_NEW_DELETE_DEFINITIONS=OFF -DLIBCXX_VALI_BOOTSTRAP=ON -DLIBCXX_HERMETIC_STATIC_LIBRARY=ON -DLIBCXX_ENABLE_STATIC_ABI_LIBRARY=ON -DLIBCXX_HAS_EXTERNAL_THREAD_API=ON -DLIBCXX_ENABLE_EXPERIMENTAL_LIBRARY=OFF -DCMAKE_BUILD_TYPE=Release -DCMAKE_TOOLCHAIN_FILE=cmake/platforms/Vali.cmake ${CMAKE_SOURCE_DIR}/llvm-project/llvm"
            WORKING_DIRECTORY ${CMAKE_SOURCE_DIR}/llvm-project/llvm
        )
        execute_process(
            COMMAND ${CMAKE_COMMAND} --build ${CMAKE_BINARY_DIR}/llvm-build -j${NUM_CORES}
        )
        execute_process(
            COMMAND ${CMAKE_COMMAND} --build ${CMAKE_BINARY_DIR}/llvm-build --target install --config Release
        )
        set(ENV{LIBCXX_BOOTSTRAP})
    endif()

    # include c++ and glm headers for rest of the projects
    include_directories(${CMAKE_INSTALL_PREFIX}/include/c++/v1)
    include_directories(${CMAKE_SOURCE_DIR}/mesa/include)
    include_directories(${CMAKE_SOURCE_DIR}/glm/glm)

    # mark GLM for install
    install(DIRECTORY ${CMAKE_SOURCE_DIR}/glm/glm 
        DESTINATION ${CMAKE_INSTALL_PREFIX}/include/glm
        PATTERN "*.txt" EXCLUDE
    )

########################################
# Build OSMesa for OpenGL support to Vioarr
    ExternalProject_Add(Mesa3D
        SOURCE_DIR ${CMAKE_SOURCE_DIR}/mesa
        CONFIGURE_COMMAND ""
        BUILD_COMMAND ${CMAKE_COMMAND} -E env VALI_ARCH=$ENV{VALI_ARCH} ${MAKE_EXECUTABLE}
        BUILD_IN_SOURCE ON
        INSTALL_COMMAND ${CMAKE_COMMAND} -E env VALI_ARCH=$ENV{VALI_ARCH} ${MAKE_EXECUTABLE} install
        BUILD_BYPRODUCTS ${CMAKE_INSTALL_PREFIX}/bin/gallium-osmesa.dll ${CMAKE_INSTALL_PREFIX}/lib/gallium-osmesa.lib
    )

    # add virtual target(s) to allow proper linking against the built library
    add_library(opengl-osmesa SHARED IMPORTED)
    set_property(TARGET opengl-osmesa PROPERTY
                IMPORTED_LOCATION "${CMAKE_INSTALL_PREFIX}/bin/gallium-osmesa.dll")
    set_property(TARGET opengl-osmesa PROPERTY
                IMPORTED_IMPLIB "${CMAKE_INSTALL_PREFIX}/lib/gallium-osmesa.lib")

########################################
# Include link directories for the above libraries
    link_directories(${CMAKE_INSTALL_PREFIX}/lib)
endif ()

########################################
# Add shared projects for all platforms
add_subdirectory (lua)
#add_subdirectory (macia)

########################################
# Build the vioarr project and it's dependencies last
add_subdirectory (vioarr)

# add include paths from vioarr
include_directories(vioarr/libs/common/include)
include_directories(vioarr/libs/asgaard/include)

# build apps that use asgaard
add_subdirectory (doom)

set(SDL_SHARED_ENABLED_BY_DEFAULT ON CACHE BOOL "Enable SDL shared library build")
set(SDL_AUDIO ON CACHE BOOL "Enable SDL audio subsystem")
set(SDL_ENABLE_VIORR ON CACHE BOOL "Enable Vioarr video subsystem")
add_subdirectory (SDL)

# add SDL include path
include_directories(SDL/include)

set(PNG_BUILD_ZLIB ON CACHE BOOL "Enable zlib build for SDL_image")
add_subdirectory (SDL_image)
add_subdirectory (SDL_mixer)
add_subdirectory (SDL_ttf)

# todo probably add sdl paths

add_subdirectory (sdlshooter)
add_subdirectory (lite)
