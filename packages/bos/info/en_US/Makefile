# @(#)07        1.10  src/packages/bos/info/en_US/Makefile, pkginfodb, pkg411, 9434A411a 8/22/94 08:02:45
#
# COMPONENT_NAME: pkginfodb
#
# FUNCTIONS: Makefile
#
# ORIGINS: 27
#
# IBM CONFIDENTIAL -- (IBM Confidential Restricted when
# combined with the aggregated modules for this product)
#                  SOURCE MATERIALS
# (C) COPYRIGHT International Business Machines Corp. 1993, 1994
# All Rights Reserved
#
# US Government Users Restricted Rights - Use, duplication or
# disclosure restricted by GSA ADP Schedule Contract with IBM Corp.
#

# IMPORTANT !!!!!
#
# This Makefile is being used to create the BFF image for the AIX 4.1
# SERVER media.  The list of options should only change to match the
# list of entitled pubs for server systems.
#
# There are three subdirectories also being dropped with Feature 140116.
# These subdirectories create the BFF images for the CLIENT media, the
# hypertext CDROM and the hypertext equivalent installable tape.  The
# hypertext installable tape is the only one that has all the options
# in normal BFF format (due to feature 117084), hence, all updates will
# be created from the "hypertape" directory.
#
# DO NOT REMOVE THE FOLLOWING LINES:
.if ((${PTF_UPDATE} == "yes") && defined(BUILD_INSTALL_IMAGES) && (${BUILD_INSTALL_IMAGES} != "yes"))
# if building updates only
SUBDIRS          = hypertape
.else

SUBDIRS		 = client hypertext hypertape

PTF_UPDATE	 = no

VPATH		 = adapters_gd_ref:aix_tech_ref:cmds_ref:files_ref
VPATH		:= ${VPATH}:info_help:manage_gds:nav:assemb_ref
VPATH		:= ${VPATH}:probsolv_gds:smit_help:user_gds

BFF		= bos.info.en_US

ILIST		= ${BFF}
IDIR		= /

# DO NOT CHANGE THE LIST OF OPTIONS (unless the entitled pubs change)

OPTIONS		= bos.info.en_US.adapters_gd_ref \
		  bos.info.en_US.assemb_ref \
		  bos.info.en_US.aix_tech_ref \
		  bos.info.en_US.files_ref \
		  bos.info.en_US.info_help \
		  bos.info.en_US.manage_gds \
		  bos.info.en_US.nav \
		  bos.info.en_US.probsolv_gds \
		  bos.info.en_US.smit_help \
		  bos.info.en_US.user_gds \
		  bos.info.en_US.cmds_ref

.include <adapters_gd_ref/packdep.mk>
.include <assemb_ref/packdep.mk>
.include <aix_tech_ref/packdep.mk>
.include <cmds_ref/packdep.mk>
.include <files_ref/packdep.mk>
.include <info_help/packdep.mk>
.include <manage_gds/packdep.mk>
.include <nav/packdep.mk>
.include <probsolv_gds/packdep.mk>
.include <smit_help/packdep.mk>
.include <user_gds/packdep.mk>

.endif

.include <${RULES_MK}>
