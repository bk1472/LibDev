############################################################################
#
#    LGE. ROBOT RESEARCH LABORATORY
#    COPYRIGHT(c) LGE CO.,LTD. 2017. SEOUL, KOREA.
#    All rights are reserved.
#    No part of this work covered by the copyright hereon may be
#    reproduced, stored in a retrieval system, in any form or
#    by any means, electronic, mechanical, photocopying, recording
#    or otherwise, without the prior permission of LG Electronics.
#
#    File Name  : incs.mk
#    Version    :
#    Author     : kyokushin (baekwon.choi@lge.com)
#    Date       : 2018/03/05
#    Descrition : Define Path / Variables of makefile
#
#############################################################################
ifndef PROC_MON_BUILD_MAK
$(error "Can not use incs.mk file, for build project")
endif

.PHONY : dumpsym ccdv tags

ifeq ($(ARCH),)
-include $(ROOT_DIR)/config/_config.mk
endif

ifeq ($(ARCH),)
$(error "ARCH is not defined!")
endif
#============================================================================
#    Output Directory path definitions
#============================================================================
BUILD_DIR		 = $(ROOT_DIR)/build/$(ARCH)
OBJ_DIR			 = $(BUILD_DIR)/objs/$(MOD_NAME)
BIN_DIR			 = $(BUILD_DIR)/bin
LIB_DIR			 = $(BUILD_DIR)/lib

#============================================================================
#    Module name for output
#============================================================================
MOD_NAME		 = $(notdir $(shell pwd))

#============================================================================
#    Include extra make scripts
#============================================================================
ENDIAN_TYPE_MK	 = $(BUILD_DIR)/endian.mk
-include	$(ENDIAN_TYPE_MK)

#============================================================================
#    Include conditional make scripts
#============================================================================
include		$(ROOT_DIR)/config/$(ARCH)/config.mk
include		$(MAK_DIR)/tools.mk

#============================================================================
#    Utilities Directory path definitions
#============================================================================
UTILS_DIR		 = $(ROOT_DIR)/utils
CCDV_DIR		 = $(UTILS_DIR)/ccdv.src
DUMPSYM_DIR		 = $(UTILS_DIR)/dumpsym.src

#============================================================================
#    pre-build conditions before building main projects
#============================================================================
all: $(CCDV) $(DUMPSYM)

tags:
	@$(ECHO) -n "Make tags..."
	@$(Tafs)
	@$(ECHO) "done"
