# Applications and libraries for Vali/MollenOS
This repository can be downloaded as a basis for application development in Vali. To start using this repository, a few things are mandatory.

## Pre-requisites
You must have built a working cross compiler toolchain, specifically the toolchain located [here](https://github.com/Meulengracht/vali-toolchain). Also required is the SDK kit that can be built from the primary repository [here](https://github.com/Meulengracht/MollenOS).

You must then setup the environment variable VALI_SDK to point to location of the unpacked SDK kit. Then it will be possible to build the applications located in this repository, and they can then be transferred to the operating system repository and can be included in the built kernel image. They will then be available on load.

## Programs/libraries that are in pipeline

| Library       | Version   | Description             |
| ------------- | ---------:|:-----------------------:|
| clang/lld     | 9.0.0-dev | C/C++ Compiler and linker   |

## Status of programs and libraries

| Library       | Version   | Description             |
| ------------- | ---------:|:-----------------------:|
| openlibm      | <unk>     | open mathematical c-library   |
| compiler-rt   | <unk>     | llvm/clang compiler runtime support  |
| libunwind     | <unk>     | unwind support for the c++ itanium abi  |
| libcxxabi     | <unk>     | c++ itanium abi compiler support implementation  |
| libcxx        | <unk>     | c++ standard library support   |
| llvm          | 9.0.0-dev | The LLVM compiler back-end   |
| mesa3d        | 18.3-dev  | Open source 3d opengl framework   |
| zlib          | 1.2.11    | Compression library   |
| libpng        | <unk>     | PNG image library   |
| freetype2     | <unk>     | Library to handle and render fonts   |
