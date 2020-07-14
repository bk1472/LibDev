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
#    File Name  : config.mk
#    Version    :
#    Author     : kyokushin (baekwon.choi@lge.com)
#    Date       : 2018/03/05
#    Descrition : x64 build configuration and build options
#
#############################################################################
CPU				 = x64
ifneq ($(CPU), $(ARCH))
$(error "$(CPU) and $(ARCH) is different error!")
endif
OPT				 = 0

#============================================================================
#   Global Include Path
#============================================================================
INC_GLOBL_PATH	 =
#INC_GLOBL_PATH	+= $(ROOT_DIR)/include

#============================================================================
#   Global Definitions
#============================================================================
cmn_def			 =
cmn_def			+= __X64__
cmn_def			+= __LINUX__
cmn_def			+= _GNU_SOURCE
cmn_def			+= BIG_ENDIAN=0x1001
cmn_def			+= LITTLE_ENDIAN=0x1002
cmn_def			+= BIT_32=0x1003
cmn_def			+= BIT_64=0x1004
cmn_def			+= ENDIAN_TYPE=$(ENDIAN_TYPE)
cmn_def			+= BIT_WIDTH=BIT_64
cmn_def			+= OPT_LVL=$(OPT)
cmn_def			+= CPP_DEMANGLE
cmn_def			+= USE_SIGSTACK_LVL=2
DEF_GLOBL		 = $(addprefix -D, $(cmn_def))

#============================================================================
#   Common Compile Flags
#============================================================================
cmn_opt			 =
cmn_opt			+= -gdwarf-3 -O$(OPT)

ARCH_OPT		 =
ARCH_OPT		+=

cmn_flags		 =

cmn_flags		+= $(cmn_opt) $(ARCH_OPT)
cmn_flags		+= $(addprefix -I, $(INC_GLOBL_PATH))
cmn_flags		+= $(addprefix -I, $(INC_PERMOD_PATH))
cmn_flags		+= $(DEF_GLOBL)
cmn_flags		+= $(addprefix -D, $(DEF_PERMOD))

# Warning
cmn_flags		+= -Wall
cmn_flags		+= -Wno-unused-but-set-variable
cmn_flags		+= -Wno-unused-result

# Functions
cmn_flags		+= -fPIC
cmn_flags		+= -fno-omit-frame-pointer
cmn_flags		+= -fno-strict-aliasing

#============================================================================
#   C Language Compile Option
#============================================================================
CFLAGS			 =
CFLAGS			+= -std=c11
CFLAGS			+= $(cmn_flags)

#============================================================================
#   CXX Language Compile Option
#============================================================================
CXXFLAGS		 =
CXXFLAGS		+= -std=c++11
CXXFLAGS		+= $(cmn_flags)

#============================================================================
#   ASM Language Compile Option
#============================================================================
ASFLAGS			 =
ASFLAGS			+= -D__ASSEMBLY__
ASFLAGS			+= -xassembler-with-cpp
ASFLAGS			+= $(cmn_flags)

#============================================================================
#   Library Link Path
#============================================================================
GLIB_PATH		 =
GLIB_PATH		+= $(LIB_DIR)

#============================================================================
#   System Library List
#============================================================================
SYS_LIBS		 =
SYS_LIBS		+= stdc++
SYS_LIBS		+= rt
SYS_LIBS		+= dl
SYS_LIBS		+= pthread

#============================================================================
#   Link Flags Executable binaries
#============================================================================
LDFLAGS			 =
LDFLAGS			+= $(LOCAL_LDFLAGS)
LDFLAGS			+= $(addprefix -L, $(GLIB_PATH))
LDFLAGS			+= $(addprefix -L, $(LLIB_PATH))
LDFLAGS			+= $(addprefix -l, $(SYS_LIBS))
