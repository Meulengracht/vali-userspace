# Makefile with rules and defines for the X86-64 platform
# 
build_target = target_amd64
arch_flags = -m64 -Damd64 -D__x86_64__ -D__STDC_FORMAT_MACROS_64 --target=amd64-pc-win32-itanium-coff -fdwarf-exceptions

# -Xclang -flto-visibility-public-std makes sure to generate cxx-abi stuff without __imp_ 
# -std=c11 enables c11 support for C compilation 0;35
# -gdwarf enables dwarf debugging generation, should be used ... -fexceptions -fcxx-exceptions
disable_warnings = -Wno-address-of-packed-member -Wno-self-assign -Wno-unused-function
shared_flags = -U_WIN32 -fms-extensions -Wall -nostdlib -nostdinc -O3 -DMOLLENOS -Xclang -flto-visibility-public-std
arch_lflags = /machine:X64
