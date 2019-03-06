# Makefile with rules and defines for the X86-32 platform
# 
build_target = target_i386
arch_flags = -m32 -Di386 -D__i386__ --target=i386-pc-win32-itanium-coff
shared_flags = -U_WIN32 -fms-extensions -nostdlib -nostdinc -DMOLLENOS -Xclang -flto-visibility-public-std
arch_lflags = /machine:X86
