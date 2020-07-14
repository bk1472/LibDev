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
TGT_NAME		 = term_mon
TGT_BIN			 = $(BIN_DIR)/$(TGT_NAME)

#==============================================================================
#	Extract list of object files from list of source files
#==============================================================================
SRCS			 = term_mon.c

#==============================================================================
#	Local Build configs
#==============================================================================
LOCAL_LDFLAGS		 =

#==============================================================================
#	target rules
#==============================================================================
all: $(TGT_BIN)

$(TGT_BIN): $(OBJS)
	@$(TST) -d $(BIN_DIR) || mkdir -p $(BIN_DIR)
	@$(CCDV) $(CCDV_OPTS) $(CC) $(SRCS) -o $@ $< -lrt


#==============================================================================
#	Read global make rules from top most directory
#==============================================================================
include		$(MAK_DIR)/rules.mk

.PHONY: $(TGT_BIN)
