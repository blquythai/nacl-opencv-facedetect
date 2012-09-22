# Copyright (c) 2012 The Native Client Authors. All rights reserved.
# Use of this source code is governed by a BSD-style license that can be
# found in the LICENSE file.

#
# GNU Make based build file.  For details on GNU Make see:
#   http://www.gnu.org/software/make/manual/make.html
#


#
# Get pepper directory for toolchain and includes.
#
# If NACL_SDK_ROOT is not set, then assume it can be found a two directories up,
# from the default example directory location.
#
THIS_MAKEFILE:=$(abspath $(lastword $(MAKEFILE_LIST)))
NACL_SDK_ROOT?=$(abspath $(dir $(THIS_MAKEFILE))../..)


#
# Project Build flags
#
# Turns on warnings (-Wxxx), builds with zero optimization (-O0) and adds debug
# information (-g) for correctness and ease of debugging.
WARNINGS:=-Wno-long-long -Wall
CXXFLAGS:=-pthread -O0 -g $(WARNINGS)
LDFLAGS:=-lppapi_cpp -lppapi -ljpeg -lfilesys -lopencv_objdetect -lopencv_calib3d -lopencv_features2d -lopencv_imgproc -lopencv_core -lopencv_flann -lopencv_highgui -lz
#order of putting library while linking
#'jpeg','filesys','opencv_objdetect','opencv_calib3d', 'opencv_features2d','opencv_imgproc', 'opencv_core', 'opencv_flann', 'opencv_highgui', 'pthread','png','z',

#
# Compute path to compiler
#
OSNAME:=$(shell python $(NACL_SDK_ROOT)/tools/getos.py)
TC_PATH:=$(abspath $(NACL_SDK_ROOT)/toolchain/$(OSNAME)_x86_glibc)


# Alias for C++ compiler
CXX:=$(TC_PATH)/bin/i686-nacl-g++


#
# Disable DOS PATH warning when using Cygwin based tools Windows
#
CYGWIN ?= nodosfilewarning
export CYGWIN


# Default target is everything
all : test_x86_32.nexe test_x86_64.nexe test.nmf

# Define 32 bit compile rule for C++ sources
test_32.o geturl_handler_32.o jpeg_mem_src_32.o: %_32.o : %.cc
	$(CXX) -o $@ -c $< -m32 -O0 -g $(CXXFLAGS)

# Define 64 bit compile rule for C++ sources
test_64.o geturl_handler_64.o jpeg_mem_src_64.o: %_64.o : %.cc
	$(CXX) -o $@ -c $< -m64 -O0 -g $(CXXFLAGS)
 
# Define link rule for 32 bit (-m32) nexe
test_x86_32.nexe : test_32.o geturl_handler_32.o jpeg_mem_src_32.o
	$(CXX) -o $@ $^ -m32 -O0 -g $(LDFLAGS)

# Define link rule for 64 bit (-m64) nexe
test_x86_64.nexe : test_64.o geturl_handler_64.o jpeg_mem_src_64.o
	$(CXX) -o $@ $^ -m64 -O0 -g $(LDFLAGS)

#
# NMF Manifiest generation
#
# Use the python script create_nmf to scan the binaries for dependencies using
# objdump.  Pass in the (-L) paths to the default library toolchains so that we
# can find those libraries and have it automatically copy the files (-s) to
# the target directory for us.
NMF:=python $(NACL_SDK_ROOT)/tools/create_nmf.py
NMF_ARGS:=-D $(TC_PATH)/x86_64-nacl/bin/objdump
NMF_PATHS:=-L $(TC_PATH)/x86_64-nacl/lib32 -L $(TC_PATH)/x86_64-nacl/lib

test.nmf : test_x86_64.nexe test_x86_32.nexe
	echo $(NMF) $(NMF_ARGS) -s . -o $@ $(NMF_PATHS) $^
	$(NMF) $(NMF_ARGS) -s . -o $@ $(NMF_PATHS) $^

# Define a phony rule so it always runs, to build nexe and start up server.
.PHONY: RUN 
RUN: all
	python ../httpd.py
