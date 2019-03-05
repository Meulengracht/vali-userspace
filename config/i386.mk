# Makefile with rules and defines for the X86-32 platform
# 
build_target = target_i386
arch_flags = -m32 -Di386 -D__i386__ --target=i386-pc-win32-itanium-coff

# -Xclang -flto-visibility-public-std makes sure to generate cxx-abi stuff without __imp_ 
# -std=c11 enables c11 support for C compilation 0;35
# -gdwarf enables dwarf debugging generation, should be used ... -fexceptions -fcxx-exceptions
disable_warnings = -Wno-address-of-packed-member -Wno-self-assign -Wno-unused-function
shared_flags = -U_WIN32 -fms-extensions -Wall -nostdlib -nostdinc -O3 -DMOLLENOS -Xclang -flto-visibility-public-std
arch_lflags = /machine:X86
