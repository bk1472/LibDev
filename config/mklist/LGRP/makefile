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

include		$(MAK_DIR)/incs.mk

#==============================================================================
#	Declare output binary and libraries location for intermediate files
#==============================================================================
TGT_NAME		 = LGRP_$(ARCH)

#SHTGT_LIB		 = $(LIB_DIR)/lib$(TGT_NAME).so
TGT_LIB			 = $(LIB_DIR)/lib$(TGT_NAME).a


LOC_DIRS		 =
LOC_DIRS		+= 3rdparty/snappy
LOC_DIRS		+= 3rdparty/xmlParser
LOC_DIRS		+= 3rdparty/xmlParser/Util
LOC_DIRS		+= 3rdparty/xmlParser/Util

LOC_DIRS		+= src/base
LOC_DIRS		+= src/factory
LOC_DIRS		+= src/logger
LOC_DIRS		+= src/main
LOC_DIRS		+= src/port
LOC_DIRS		+= src/service
LOC_DIRS		+= src/servicemessage
LOC_DIRS		+= src/worker

LOC_DIRS		+= device/BasicTimer
LOC_DIRS		+= device/DirectMessageLogger
LOC_DIRS		+= device/MultiProcessCalendar
LOC_DIRS		+= device/MultiProcessTombstoneLogger
LOC_DIRS		+= device/SeperatedFullLogger
LOC_DIRS		+= device/SingleProcessFullLogger
LOC_DIRS		+= device/SingleProcessTombstoneLogger

#==============================================================================
#	Project source List
#==============================================================================
-include $(foreach dirs, $(LOC_DIRS), $(ROOT_DIR)/config/mklist/$(notdir $(CURR_DIR))/$(dirs)/$(notdir $(dirs)).mak)

#==============================================================================
#	Extract list of object files from list of source files
#==============================================================================
SRCS			 = $(foreach dirs, $(LOC_DIRS), $($(notdir $(dirs))_srcs))
SRCS_cc			 = $(foreach dirs, $(LOC_DIRS), $($(notdir $(dirs))_srcs_cc))
SRCS_cpp		 = $(foreach dirs, $(LOC_DIRS), $($(notdir $(dirs))_srcs_cpp))

OBJS			 =
OBJS			+= $(foreach src, $(SRCS),     $(OBJ_DIR)/$(src:.c=.o))
OBJS			+= $(foreach src, $(SRCS_cc),  $(OBJ_DIR)/$(src:.cc=.o))
OBJS			+= $(foreach src, $(SRCS_cpp), $(OBJ_DIR)/$(src:.cpp=.o))

VPATH			 = $(LOC_DIRS)

#==============================================================================
#	Srcs Include path List
#==============================================================================
INC_PERMOD_PATH	 =
INC_PERMOD_PATH	+= ./inc
INC_PERMOD_PATH	+= ./device
INC_PERMOD_PATH	+= ./3rdparty

#==============================================================================
#	target rules
#==============================================================================
#all: $(SHTGT_LIB) $(TGT_LIB)
all: $(TGT_LIB)

#==============================================================================
#	Read global make rules from top most directory
#==============================================================================
include		$(MAK_DIR)/rules.mk
