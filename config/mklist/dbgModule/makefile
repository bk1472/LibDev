#############################################################################
#
#    LGE. ROBOT RESEARCH LABORATORY
#    COPYRIGHT(c) LGE CO.,LTD. 2017. SEOUL, KOREA.
#    All rights are reserved.
#    No part of this work covered by the copyright hereon may be
#    reproduced, stored in a retrieval system, in any form or
#    by any means, electronic, mechanical, photocopying, recording
#    or otherwise, without the prior permission of LG Electronics.
#
#    File Name  : makefile
#    Version    :
#    Author     : kyokushin (baekwon.choi@lge.com)
#    Date       : 2017/03/05
#    Descrition : modules library makefile
#
#############################################################################
CURR_DIR		 = $(shell pwd)
ROOT_DIR		 = $(CURR_DIR)/..
MAK_DIR			 = $(ROOT_DIR)/config/mak
PBUILT_DIR		 = $(CURR_DIR)/prebuilt_lib/$(ARCH)

include		$(MAK_DIR)/incs.mk

#==============================================================================
#	Declare output binary and libraries location for intermediate files
#==============================================================================
TGT_NAME		 = dbgModule_$(ARCH)

DBG_TGT			 = $(LIB_DIR)/lib$(TGT_NAME).a


LOC_DIRS		 =
LOC_DIRS		+= arch
LOC_DIRS		+= arch/$(ARCH)
LOC_DIRS		+= xlibc
LOC_DIRS		+= debug
LOC_DIRS		+= thread
LOC_DIRS		+= symbol
LOC_DIRS		+= comm

#==============================================================================
#	Pre-built libraries for link
#==============================================================================
pbuilt_libs		 =
pbuilt_libs		+= capstone
PBUILT_LIB		 = $(addsuffix .a, $(addprefix lib, $(pbuilt_libs)))

#==============================================================================
#	pre-built object lists
#==============================================================================
PBUILT_OBJS	 =
PBUILT_OBJS	+= $(shell ls $(PBUILT_DIR)/pbobj/capstone/*.po)

#==============================================================================
#	Project source List
#==============================================================================
-include $(foreach dirs, $(LOC_DIRS), $(ROOT_DIR)/config/mklist/$(notdir $(CURR_DIR))/$(dirs)/$(notdir $(dirs)).mak)

#==============================================================================
#	Root Source Code
#==============================================================================
SRCS			 =
SRCS			+= root.c

#==============================================================================
#	Extract list of object files from list of source files
#==============================================================================
mak_srcs		 = $(foreach dirs, $(LOC_DIRS), $($(notdir $(dirs))_srcs))
mak_srcs_cc		 = $(foreach dirs, $(LOC_DIRS), $($(notdir $(dirs))_srcs_cc))
mak_srcs_cpp	 = $(foreach dirs, $(LOC_DIRS), $($(notdir $(dirs))_srcs_cpp))

OBJS			 =
OBJS			+= $(foreach src, $(SRCS),         $(OBJ_DIR)/$(src:.c=.o))
OBJS			+= $(foreach src, $(mak_srcs),     $(OBJ_DIR)/$(src:.c=.o))
OBJS			+= $(foreach src, $(mak_srcs_cc),  $(OBJ_DIR)/$(src:.cc=.o))
OBJS			+= $(foreach src, $(mak_srcs_cpp), $(OBJ_DIR)/$(src:.cpp=.o))

VPATH			 = $(LOC_DIRS)

#==============================================================================
#	Srcs Include path List
#==============================================================================
INC_PERMOD_PATH	 =
INC_PERMOD_PATH	+= ./inc
INC_PERMOD_PATH	+= ./inc/xlibc
INC_PERMOD_PATH	+= ./inc/comm
INC_PERMOD_PATH	+= ./inc/mm
INC_PERMOD_PATH	+= ./inc/debug
INC_PERMOD_PATH	+= ./inc/thread
INC_PERMOD_PATH	+= ./inc/symbol
INC_PERMOD_PATH	+= ./inc/capstone
INC_PERMOD_PATH	+= ./inc/arch
INC_PERMOD_PATH	+= ./inc/arch/$(ARCH)

#==============================================================================
#	target rules
#==============================================================================
all: $(DBG_TGT)

$(DBG_TGT): $(OBJS) $(PBUILT_OBJS)
	@$(TST) -d $(LIB_DIR) || mkdir -p $(LIB_DIR)
	@$(RM) $(DBG_TGT)
	@$(CCDV) $(CCDV_OPTS) $(AR) crs $(DBG_TGT) $(OBJS) $(PBUILT_OBJS) || exit 1

#==============================================================================
#	Read global make rules from top most directory
#==============================================================================
include		$(MAK_DIR)/rules.mk
