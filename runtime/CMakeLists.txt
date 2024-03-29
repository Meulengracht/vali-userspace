cmake_minimum_required(VERSION 3.16)
project(ValiRuntime)
include(ExternalProject)

set(PROJECTS_TO_BUILD "libcxx|libcxxabi|libunwind")
set(ARCHS_TO_BUILD "X86")

Externalproject_Add(
    LLVMRuntime

    SOURCE_DIR ${RUNTIME_SOURCE_DIR}/llvm-project/llvm
    CMAKE_ARGS 
        -DLLVM_INCLUDE_TESTS=OFF
        -DLLVM_INCLUDE_EXAMPLES=OFF
        -DLLVM_INCLUDE_BENCHMARKS=OFF
        -DLLVM_BOOTSTRAP_RUNTIME=ON
        -DLLVM_ENABLE_DUMP=ON
        -DLLVM_ENABLE_PROJECTS:STRING=${PROJECTS_TO_BUILD}
        -DLLVM_TARGETS_TO_BUILD:STRING=${ARCHS_TO_BUILD}
        -DLLVM_ENABLE_LIBCXX=ON
        -DLIBCXXABI_USE_LLVM_UNWINDER=ON
        -DLIBCXX_CXX_ABI:STRING=libcxxabi
        -DCMAKE_INSTALL_PREFIX=${CMAKE_INSTALL_PREFIX}/
        -DCMAKE_BUILD_TYPE=Release
        -DCMAKE_TOOLCHAIN_FILE:STRING=cmake/platforms/Vali.cmake
    LIST_SEPARATOR |
)
