# Copyright (c) 2012 The Native Client Authors. All rights reserved.
# Use of this source code is governed by a BSD-style license that can be
# found in the LICENSE file.

#
# GNU Make based build file.  For details on GNU Make see:
#   http://www.gnu.org/software/make/manual/make.html
#

VALID_TOOLCHAINS:=glibc

NACL_SDK_ROOT:=$(NACL_SDK_ROOT)
include $(NACL_SDK_ROOT)/tools/common.mk

TARGET=test

test_SOURCES= \
	geturl_handler.cc \
	jpeg_mem_src.cc \
	test.cc \

# Libraries and dependencies
# Dependencies need to be rebuild everytime, while libraries are not
DEPS=
LIBS=$(DEPS) ppapi_cpp ppapi pthread jpeg naclmounts opencv_objdetect opencv_calib3d opencv_features2d opencv_imgproc opencv_core opencv_contrib opencv_flann opencv_highgui z

$(foreach dep,$(DEPS),$(eval $(call DEPEND_RULE,$(dep))))
$(foreach src,$(test_SOURCES),$(eval $(call COMPILE_RULE,$(src),)))

ifeq ($(CONFIG),Release)
$(eval $(call LINK_RULE,test_unstripped,$(test_SOURCES),$(LIBS),$(DEPS)))
$(eval $(call STRIP_RULE,test,test_unstripped))
else
$(eval $(call LINK_RULE,test,$(test_SOURCES),$(LIBS),$(DEPS)))
endif

$(eval $(call NMF_RULE,$(TARGET),))
