# @(#)17        1.4  src/packages/bos/info/en_US/hypertext/Makefile, pkghyper, pkg411, GOLD410 6/6/94 14:51:35
#
# COMPONENT_NAME: pkg
#
# FUNCTIONS: Makefile
#
# ORIGINS: 27
#
# IBM CONFIDENTIAL -- (IBM Confidential Restricted when
# combined with the aggregated modules for this product)
#                  SOURCE MATERIALS
# (C) COPYRIGHT International Business Machines Corp. 1994
# All Rights Reserved
#
# US Government Users Restricted Rights - Use, duplication or
# disclosure restricted by GSA ADP Schedule Contract with IBM Corp.
#

# IMPORTANT !!!!!
#
# This Makefile is being used to create the BFF image for the AIX 4.1
# Hypertext TAPE media.  This should be a complete list of the options
# for the BOS LPP.
#
# DO NOT REMOVE THE FOLLOWING TWO VARIABLES:
USR_LIBLPP_LIST  = bos.info.en_US.adapters_gd_ref.cfginfo \
		   bos.info.en_US.assemb_ref.cfginfo \
		   bos.info.en_US.aix_tech_ref.cfginfo \
		   bos.info.en_US.files_ref.cfginfo \
		   bos.info.en_US.info_help.cfginfo \
		   bos.info.en_US.keybd_tech_ref.cfginfo \
		   bos.info.en_US.manage_gds.cfginfo \
		   bos.info.en_US.nav.cfginfo \
		   bos.info.en_US.probsolv_gds.cfginfo \
		   bos.info.en_US.smit_help.cfginfo \
		   bos.info.en_US.prog_gds.cfginfo \
		   bos.info.en_US.update.cfginfo \
		   bos.info.en_US.user_gds.cfginfo \
		   bos.info.en_US.cmds_ref.cfginfo

PTF_UPDATE	 = no

VPATH		 = ../adapters_gd_ref:../aix_tech_ref:../cmds_ref:../files_ref
VPATH		:= ${VPATH}:../info_help:../keybd_tech_ref:../manage_gds:../nav:../assemb_ref
VPATH		:= ${VPATH}:../probsolv_gds:../smit_help:../prog_gds:../update:../user_gds

BFF		= bos.info.en_US

ILIST		= ${BFF}.hypertext
IDIR		= /

OPTIONS		= bos.info.en_US.adapters_gd_ref \
		  bos.info.en_US.assemb_ref \
		  bos.info.en_US.aix_tech_ref \
		  bos.info.en_US.files_ref \
		  bos.info.en_US.info_help \
		  bos.info.en_US.keybd_tech_ref \
		  bos.info.en_US.manage_gds \
		  bos.info.en_US.nav \
		  bos.info.en_US.probsolv_gds \
		  bos.info.en_US.smit_help \
		  bos.info.en_US.prog_gds \
		  bos.info.en_US.update \
		  bos.info.en_US.user_gds \
		  bos.info.en_US.cmds_ref

ADEPACKAGEFLAGS = -n

.include <../adapters_gd_ref/packdep.mk>
.include <../assemb_ref/packdep.mk>
.include <../aix_tech_ref/packdep.mk>
.include <../cmds_ref/packdep.mk>
.include <../files_ref/packdep.mk>
.include <../keybd_tech_ref/packdep.mk>
.include <../info_help/packdep.mk>
.include <../manage_gds/packdep.mk>
.include <../nav/packdep.mk>
.include <../probsolv_gds/packdep.mk>
.include <../smit_help/packdep.mk>
.include <../prog_gds/packdep.mk>
.include <../update/packdep.mk>
.include <../user_gds/packdep.mk>

.include <${RULES_MK}>

bos.info.en_US.hypertext : bos.info.en_US
	${CP} bos.info.en_US bos.info.en_US.hypertext
