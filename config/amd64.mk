# Makefile with rules and defines for the X86-64 platform
# 
arch_flags = -m64 -Damd64 -D__amd64__ -D__x86_64__ -D__STDC_FORMAT_MACROS_64 --target=amd64-pc-win32-itanium-coff -fdwarf-exceptions
shared_flags = -U_WIN32 -fms-extensions -nostdlib -nostdinc -DMOLLENOS -Xclang -flto-visibility-public-std
arch_lflags = /machine:X64
