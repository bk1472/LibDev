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
#    Date       : 2017/03/19
#    Descrition : makefile( Global root )
#
#############################################################################
PROC_MON_BUILD_MAK	 =	Y
export PROC_MON_BUILD_MAK

ROOT_DIR		 = $(shell pwd)
MAK_DIR			 = $(ROOT_DIR)/config/mak

include		$(MAK_DIR)/incs.mk

SUB_DIRS		 =
SUB_DIRS		+= dbgModule
SUB_DIRS		+= example
SUB_DIRS		+= term_mon

all: build_log check_endian do_mak subdirs

do_mak:
	@bash $(UTILS_DIR)/creatmak.sh

include		$(MAK_DIR)/rules.mk

PHONY: example
