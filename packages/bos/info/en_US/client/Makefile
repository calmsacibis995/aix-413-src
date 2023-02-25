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
# CLIENT media.  The list of options should only change to match the
# list of entitled pubs for client systems.
#
# DO NOT REMOVE THE FOLLOWING LINE:
PTF_UPDATE	 = no

VPATH		 = ../files_ref:../info_help:../manage_gds:../nav:../cmds_ref
VPATH		:= ${VPATH}:../probsolv_gds:../smit_help:../user_gds

BFF		= bos.info.en_US

ILIST		= ${BFF}.client
IDIR		= /

# DO NOT CHANGE THE LIST OF OPTIONS (unless the entitled pubs change)

OPTIONS		= bos.info.en_US.files_ref \
		  bos.info.en_US.info_help \
		  bos.info.en_US.manage_gds \
		  bos.info.en_US.nav \
		  bos.info.en_US.probsolv_gds \
		  bos.info.en_US.smit_help \
		  bos.info.en_US.user_gds \
		  bos.info.en_US.cmds_ref

.include <../cmds_ref/packdep.mk>
.include <../files_ref/packdep.mk>
.include <../info_help/packdep.mk>
.include <../manage_gds/packdep.mk>
.include <../nav/packdep.mk>
.include <../probsolv_gds/packdep.mk>
.include <../smit_help/packdep.mk>
.include <../user_gds/packdep.mk>

.include <${RULES_MK}>

bos.info.en_US.client : bos.info.en_US
	${CP} bos.info.en_US bos.info.en_US.client
