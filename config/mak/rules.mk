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
#    File Name  : rules.mk
#    Version    :
#    Author     : kyokushin (baekwon.choi@lge.com)
#    Date       : 2018/03/05
#    Descrition : Define Path / Variables of makefile
#
#############################################################################
ifndef PROC_MON_BUILD_MAK
$(error "Can not use rules.mk file, for build project")
endif

BUILD_LOG	 = $(BUILD_DIR)/$(ARCH).build.log
MKD_ERR		 = $(BUILD_DIR)/$(ARCH).err.log


.DELETE_ON_ERROR:

#=========================================================================================
# Rules for cleanup and dependency.
#=========================================================================================
.PHONY: depend clean clobber clean_dep

depend: .depend_sub .depend_ext $(OBJS:.o=.d)

clean: .clean_sub .clean_ext
	@$(RM) $(OBJS)

clobber: .clobber_sub .clobber_ext
	@$(RM) $(OBJS) $(OBJS:.o=.d) $(SHTGT_LIB) $(SHTGT_LIB).0 $(SHTGT_LIB).0.0.0 $(TGT_LIB) *.build.log
	@$(RM) $(TGT_BIN) $(TGT_BIN).sym

clean_dep:
	@$(ECHO) "cleaning dependency files"
	@$(FIND) . -name "*.d" -exec rm {} \;

.clean_sub:
	@( $(foreach dn, $(SUB_DIRS), $(MAKE) -C $(ROOT_DIR)/$(dn) -f $(ROOT_DIR)/config/mklist/$(dn)/makefile clean   &&) $(ECHO) -n ) || exit 1

.clobber_sub:
	@( $(foreach dn, $(SUB_DIRS), $(MAKE) -C $(ROOT_DIR)/$(dn) -f $(ROOT_DIR)/config/mklist/$(dn)/makefile clobber &&) $(ECHO) -n ) || exit 1

.depend_sub:
	@( $(foreach dn, $(SUB_DIRS), $(MAKE) -C $(ROOT_DIR)/$(dn) -f $(ROOT_DIR)/config/mklist/$(dn)/makefile depend  &&) $(ECHO) -n ) || exit 1

subdirs: subdirs_ext
	@( $(foreach dn, $(SUB_DIRS), $(MAKE) -C $(ROOT_DIR)/$(dn) -f $(ROOT_DIR)/config/mklist/$(dn)/makefile &&) $(ECHO) -n ) || exit 1

distclean:
	@$(FIND) . -type d \( -name objs -o -name lib -o -name bin -o -name build \) -print0 | xargs -0 rm -rf
	@$(FIND) . -type f \( -name _config.mk -o -name dummy.* -o -name endian.mk -o -name tags -o -name ccdv -o -name *.mak \
		-o -name *.build.log -o -name dumpsym \) -print0 | xargs -0 rm -f
	-@$(CD) $(CCDV_DIR) ;  ./make.sh clobber;
	@$(MAKE) -C $(DUMPSYM_DIR) clobber

.clean_ext .clobber_ext .depend_ext subdirs_ext:


#=========================================================================================
# Override implicit rules to generate .o files
#=========================================================================================
$(OBJ_DIR)/%.d : %.c
	@$(ECHO) "++++++++ Making $(notdir $@)"
	@$(TST) -d $(OBJ_DIR) || mkdir -p $(OBJ_DIR)
	@( $(CC) -MM $(CFLAGS) $<													\
	  | sed -f $(UTILS_DIR)/mkdep.sed											\
	  | grep -v "^  \\\\"														\
	  | sed  -e "s\$(notdir $(<:.c=.o))\$@ $(OBJ_DIR)/$(notdir $(<:.c=.o))\g"	\
	) > $@ 2>$(MKD_ERR) || (cat $(MKD_ERR); rm $(MKD_ERR) $@; exit 1)			\
	$(NULL)

$(OBJ_DIR)/%.d : %.cpp
	@$(ECHO) "++++++++ Making $(notdir $@)"
	@$(TST) -d $(OBJ_DIR) || mkdir -p $(OBJ_DIR)
	@( $(CXX) -MM $(CXXFLAGS) $<												\
	  | sed -f $(UTILS_DIR)/mkdep.sed											\
	  | grep -v "^  \\\\"														\
	  | sed  -e "s\$(notdir $(<:.cpp=.o))\$@ $(OBJ_DIR)/$(notdir $(<:.cpp=.o))\g"\
	) > $@ 2>$(MKD_ERR) || (cat $(MKD_ERR); rm $(MKD_ERR) $@; exit 1)			\
	$(NULL)

$(OBJ_DIR)/%.d : %.cc
	@$(ECHO) "++++++++ Making $(notdir $@)"
	@$(TST) -d $(OBJ_DIR) || mkdir -p $(OBJ_DIR)
	@( $(CXX) -MM $(CXXFLAGS) $<												\
	  | sed -f $(UTILS_DIR)/mkdep.sed											\
	  | grep -v "^  \\\\"														\
	  | sed  -e "s\$(notdir $(<:.cc=.o))\$@ $(OBJ_DIR)/$(notdir $(<:.cc=.o))\g"	\
	) > $@ 2>$(MKD_ERR) || (cat $(MKD_ERR); rm $(MKD_ERR) $@; exit 1)			\
	$(NULL)

$(OBJ_DIR)/%.d : %.s
	@$(ECHO) "++++++++ Making $(notdir $@)"
	@$(TST) -d $(OBJ_DIR) || mkdir -p $(OBJ_DIR)
	@( $(CXX) -MM $(ASFLAGS) $<													\
	  | sed -f $(UTILS_DIR)/mkdep.sed											\
	  | grep -v "^  \\\\"														\
	  | sed  -e "s\$(notdir $(<:.s=.o))\$@ $(OBJ_DIR)/$(notdir $(<:.s=.o))\g"	\
	) > $@ 2>$(MKD_ERR) || (cat $(MKD_ERR); rm $(MKD_ERR) $@; exit 1)			\
	$(NULL)

$(OBJ_DIR)/%.d : %.S
	@$(ECHO) "++++++++ Making $(notdir $@)"
	@$(TST) -d $(OBJ_DIR) || mkdir -p $(OBJ_DIR)
	@( $(CXX) -MM $(ASFLAGS) $<													\
	  | sed -f $(UTILS_DIR)/mkdep.sed											\
	  | grep -v "^  \\\\"														\
	  | sed  -e "s\$(notdir $(<:.S=.o))\$@ $(OBJ_DIR)/$(notdir $(<:.S=.o))\g"	\
	) > $@ 2>$(MKD_ERR) || (cat $(MKD_ERR); rm $(MKD_ERR) $@; exit 1)			\
	$(NULL)

$(OBJ_DIR)/%.o: %.c
	@$(TST) -d $(OBJ_DIR) || mkdir -p $(OBJ_DIR)
	@$(CCDV) $(CCDV_OPTS) $(CC) -c $(CFLAGS) -o $@ $<

$(OBJ_DIR)/%.o: %.cpp
	@$(TST) -d $(OBJ_DIR) || mkdir -p $(OBJ_DIR)
	@$(CCDV) $(CCDV_OPTS) $(CXX) -c $(CXXFLAGS) -o $@ $<

$(OBJ_DIR)/%.o: %.cc
	@$(TST) -d $(OBJ_DIR) || mkdir -p $(OBJ_DIR)
	@$(CCDV) $(CCDV_OPTS) $(CXX) -c $(CXXFLAGS) -o $@ $<

$(OBJ_DIR)/%.o: %.s
	@$(TST) -d $(OBJ_DIR) || mkdir -p $(OBJ_DIR)
	@$(CCDV) $(CCDV_OPTS) $(CXX) -c $(ASFLAGS) -o $@ $<

$(OBJ_DIR)/%.o: %.S
	@$(TST) -d $(OBJ_DIR) || mkdir -p $(OBJ_DIR)
	@$(CCDV) $(CCDV_OPTS) $(CXX) -c $(ASFLAGS) -o $@ $<

ifneq ($(TGT_LIB),)
#==============================================================================
#   Rule for making archives. This rule should be used in sub modules
#==============================================================================
$(TGT_LIB): $(OBJS) $(PBUILT_OBJS)
	@$(TST) -d $(LIB_DIR) || mkdir -p $(LIB_DIR)
	@$(RM) $(TGT_LIB)
	@$(CCDV) $(CCDV_OPTS) $(AR) crs $(TGT_LIB) $(OBJS) $(PBUILT_OBJS)|| exit 1
endif

ifneq ($(SHTGT_LIB),)
#==============================================================================
#   Rule for making so lbraries. This rule should be used in sub modules
#==============================================================================
$(SHTGT_LIB): $(OBJS)
	@$(TST) -d $(LIB_DIR) || mkdir -p $(LIB_DIR)
	@$(RM) $(SHTGT_LIB)
	@$(CCDV) $(CCDV_OPTS) $(CC) -shared -Wl,-soname,$@.0 -o $@.0.0.0 $(OBJS) || exit 1
	@$(LN) -f $(notdir $@.0.0.0) $@
	@$(LN) -f $(notdir $@.0.0.0) $@.0
endif


#=========================================================================================
# Utilities make
#=========================================================================================
all: $(UTILS_DIR)/ccdv

#=========================================================================================
# print out build environment
#=========================================================================================
build_log:
	@$(TST) -d $(BUILD_DIR) || mkdir -p $(BUILD_DIR)
	@( if [ ! -f $(BUILD_LOG) ]; then						\
		echo    "[Used Compiler]"        > $(BUILD_LOG);	\
		echo    "$(CC), $(CXX)"         >> $(BUILD_LOG);	\
		echo    "[Compile Flags]"       >> $(BUILD_LOG);	\
		echo    "   cflags: $(CFLAGS)"  >> $(BUILD_LOG);	\
		echo    " c++flags: $(CXXFLAGS)">> $(BUILD_LOG);	\
		echo    "[Link Flags]"          >> $(BUILD_LOG);	\
		echo    " ldflags:  $(LDFLAGS)" >> $(BUILD_LOG);	\
	   fi)

#=========================================================================================
# check endianess
#=========================================================================================
ENDIAN_SRC	 = $(BUILD_DIR)/dummy.c
ENDIAN_OBJ	 = $(BUILD_DIR)/dummy.o

check_endian :
ifeq "$(ENDIAN_TYPE)" ""
	@if [ ! -f $(ENDIAN_OBJ) ]; then												\
		$(TOUCH) $(ENDIAN_SRC);														\
		$(CC) $(CFLAGS) -o $(ENDIAN_OBJ) -c $(ENDIAN_SRC);							\
		$(ECHO) "# Do not modify this file"					 > $(ENDIAN_TYPE_MK);	\
		$(ECHO) "# This file is modified automatically"		>> $(ENDIAN_TYPE_MK);	\
		file $(ENDIAN_OBJ) | grep LSB > /dev/null;									\
		if [ $$? -eq 0 ]; then														\
			$(ECHO) "ENDIAN_TYPE = LITTLE_ENDIAN"			>> $(ENDIAN_TYPE_MK);	\
		else																		\
			$(ECHO) "ENDIAN_TYPE = BIG_ENDIAN"				>> $(ENDIAN_TYPE_MK);	\
		fi																			\
	fi
ENDIAN_TYPE = $(shell grep ENDIAN_TYPE $(ENDIAN_TYPE_MK) 2> /dev/null | awk '{print $$3}')
endif

#=========================================================================================
# Conditional directive to include dependency list itself
#=========================================================================================
ifeq ($(OBJS),)
INCLUDE_DEPEND	?= 0
else
INCLUDE_DEPEND	?= 1
ifneq ($(MAKECMDGOALS),)
ifneq ($(MAKECMDGOALS),depend)
INCLUDE_DEPEND	 = 0
endif
endif
endif

ifeq ($(INCLUDE_DEPEND), 1)
-include $(OBJS:.o=.d)
ifdef TGT_LIB
$(TGT_LIB) : $(OBJS:.o=.d)
endif
ifdef SHTGT_LIB
$(SHTGT_LIB) : $(OBJS:.o=.d)
endif
endif

$(CCDV): $(CCDV_DIR)/ccdv.c $(CCDV_DIR)/sift-warn.c
ifeq ($(MAKELEVEL), 0)
	 -@$(CD) $(CCDV_DIR) ;  ./make.sh;
endif

$(DUMPSYM):
ifeq ($(MAKELEVEL), 0)
	 @$(MAKE) -C $(DUMPSYM_DIR)
endif
