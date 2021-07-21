cmake_minimum_required (VERSION 3.14)
project (ValiUserspace)

include(ExternalProject)
include(ProcessorCount)

set (CMAKE_MODULE_PATH ${CMAKE_MODULE_PATH} "${CMAKE_SOURCE_DIR}/cmake/modules/")
set (CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/bin)
set (CMAKE_LIBRARY_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/bin)
set (CMAKE_ARCHIVE_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/lib)

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
    if (EXISTS ${CMAKE_BINARY_DIR}/llvm-build/bin/lli.app)
        message(STATUS "LLVM seems to be built already, assuming all is fine and skipping")
    else()
        set(ENV{LIBCXX_BOOTSTRAP} "true")
        execute_process(
            COMMAND ${CMAKE_COMMAND} ${CMAKE_SOURCE_DIR}/llvm -DLLVM_BOOTSTRAP_RUNTIME=ON -DLLVM_ENABLE_DUMP=ON -DLIBUNWIND_INSTALL_PREFIX=${CMAKE_INSTALL_PREFIX} -DLIBUNWIND_INCLUDE_DOCS=OFF -DLIBCXXABI_INSTALL_PREFIX=${CMAKE_INSTALL_PREFIX} -DLIBCXXABI_VALI_BOOTSTRAP=ON -DLIBCXXABI_HERMETIC_STATIC_LIBRARY=ON -DLIBCXXABI_USE_LLVM_UNWINDER=ON -DLIBCXXABI_HAS_EXTERNAL_THREAD_API=ON -DLIBCXXABI_ENABLE_NEW_DELETE_DEFINITIONS=OFF -DLIBCXX_INSTALL_PREFIX=${CMAKE_INSTALL_PREFIX} -DLIBCXX_VALI_BOOTSTRAP=ON -DLIBCXX_HERMETIC_STATIC_LIBRARY=ON -DLIBCXX_ENABLE_STATIC_ABI_LIBRARY=ON -DLIBCXX_HAS_EXTERNAL_THREAD_API=ON -DLIBCXX_ENABLE_EXPERIMENTAL_LIBRARY=OFF -DLIBCXX_INCLUDE_BENCHMARKS=OFF -DLIBCXX_INCLUDE_TESTS=OFF -DCMAKE_INSTALL_PREFIX=${CMAKE_INSTALL_PREFIX} -DCMAKE_BUILD_TYPE=Release -DCMAKE_TOOLCHAIN_FILE=cmake/platforms/Vali.cmake
            WORKING_DIRECTORY ${CMAKE_BINARY_DIR}/llvm-build
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
        BUILD_COMMAND ${MAKE_EXECUTABLE}
        INSTALL_COMMAND ${MAKE_EXECUTABLE} install
        BUILD_BYPRODUCTS ${CMAKE_INSTALL_PREFIX}/gallium-osmesa.dll
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
