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
#    Descrition : example makefile
#
#############################################################################
ROOT_DIR		 = $(shell pwd)/..
MAK_DIR			 = $(ROOT_DIR)/config/mak

include		$(MAK_DIR)/incs.mk

#==============================================================================
#	Declare output binary and libraries location for intermediate files
#==============================================================================
TGT_NAME		 = dbg_sample
TGT_BIN			 = $(BIN_DIR)/$(TGT_NAME)

#==============================================================================
#	Extract list of object files from list of source files
#==============================================================================
SRCS_c				 =
#SRCS_c				+= main_c.c

SRCS_cpp			 =
SRCS_cpp			+= main_cpp.cpp

OBJS				 = $(foreach src, $(SRCS_c),   $(OBJ_DIR)/$(src:.c=.o))
OBJS			 	+= $(foreach src, $(SRCS_cpp), $(OBJ_DIR)/$(src:.cpp=.o))

#==============================================================================
#	Local Build configs
#==============================================================================
LOCAL_LDFLAGS		 =
LOCAL_LDFLAGS		+= -Wl,--whole-archive
LOCAL_LDFLAGS		+= -ldbgModule_$(ARCH)
LOCAL_LDFLAGS		+= -Wl,--no-whole-archive

#==============================================================================
#	target rules
#==============================================================================
all: $(TGT_BIN)

$(TGT_BIN): $(OBJS)
	@$(TST) -d $(BIN_DIR) || mkdir -p $(BIN_DIR)
	@$(CCDV) $(CCDV_OPTS) $(CXX) $(LDFLAGS) -o $@ $<
	@$(DUMPSYM) $@


#==============================================================================
#	Read global make rules from top most directory
#==============================================================================
include		$(MAK_DIR)/rules.mk

.PHONY: $(TGT_BIN)
