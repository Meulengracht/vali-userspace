# Makefile for building a generic userspace application
# Include all the definitions for os
APP_NAME := alumni
APP_SOURCES_CXX := targets/alumni.cpp \
				   targets/alumni_vali.cpp \
				   surfaces/surface_vali.cpp \
			       $(wildcard *.cpp)

CXXFLAGS := -O3 -std=c++17
LFLAGS := $(VALI_APPLICATION_PATH)/lib/libfreetype.lib

include ../application.mk