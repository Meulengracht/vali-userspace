# Makefile for building a generic userspace application
# Include all the definitions for os
APP_NAME := wintest
APP_SOURCES_CXX := main.cpp

INCLUDES := -I$(VALI_DDK_PATH)/include
LFLAGS := -LIBPATH:$(VALI_DDK_PATH)/lib asgaard.lib static_libgracht.lib static_libddk.lib libfreetype.lib

include ../application.mk