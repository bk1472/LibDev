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
#    File Name  : tools.mk
#    Version    :
#    Author     : kyokushin (baekwon.choi@lge.com)
#    Date       : 2018/03/05
#    Descrition : Definition of SW build and utilities
#
#############################################################################
ifndef PROC_MON_BUILD_MAK
$(error "Can not use tools.mk file, for build project")
endif
#============================================================================
#    Host Tools
#============================================================================
HOSTCC		 = gcc
HOSTCXX		 = g++
HOSTCPP		 = gcc -E -P

ECHO		 = echo
MKDIR		 = mkdir -p
RM			 = rm -f
RMDIR		 = rm -rf
TOUCH		 = touch
TST			 = test
CP			 = cp
MV			 = mv -f
CD			 = cd
LN			 = ln -s
TAR			 = tar
CAT			 = cat
SED			 = sed
AWK			 = awk
GZIP		 = gzip
MAKE		 = make
FIND		 = find
FIND_L		 = find -L
CHMOD		 = chmod
DOS2UNIX	 = dos2unix
CPIO_CMD	 = LC_ALL=C cpio -pudm --quiet
MD5SUM		 = md5sum
TAGS		 = ctags -R

#============================================================================
#    Cross Tools
#============================================================================
CC			 = $(CROSS_COMPILE)gcc
CXX			 = $(CROSS_COMPILE)g++
CPP			 = $(CROSS_COMPILE)gcc -E -P
AS			 = $(CROSS_COMPILE)as
AR			 = $(CROSS_COMPILE)ar
LD			 = $(CROSS_COMPILE)ld
NM			 = $(CROSS_COMPILE)nm
OBJCOPY		 = $(CROSS_COMPILE)objcopy
OBJDUMP		 = $(CROSS_COMPILE)objdump
RANLIB		 = $(CROSS_COMPILE)ranlib
SIZE		 = $(CROSS_COMPILE)size

#============================================================================
#    Add on Tools
#============================================================================
#============================================================================
#    Add on Tools
#============================================================================
CCDV_OPTS	?= -v0 -p -x 1000
CCDV		 = $(UTILS_DIR)/ccdv
DUMPSYM		 = $(UTILS_DIR)/dumpsym
