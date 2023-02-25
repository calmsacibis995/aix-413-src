# @(#)06        1.10  src/packages/X11/info/en_US/Makefile, pkginfodb, pkg41J, 9519A_all 4/28/95 13:40:39
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
# IMPORTANT !!!!
#
# Do not remove the next line it is being used to create the images
# to be placed on the hypertext CDRom.
.if (${PTF_UPDATE} != "yes" )
SUBDIRS          = hypertext
.endif

VPATH		= prog_gds:tech_ref:x_st_mgr:x_st_mgr/manage_gd:Dt

BFF		= X11.info.en_US

ILIST		= ${BFF}
IDIR		= /

OPTIONS		= X11.info.en_US.prog_gds \
		  X11.info.en_US.tech_ref \
		  X11.info.en_US.x_st_mgr.manage_gd \
		  X11.info.en_US.Dt

.include <prog_gds/packdep.mk>
.include <tech_ref/packdep.mk>
.include <x_st_mgr/manage_gd/packdep.mk>
.include <Dt/packdep.mk>

.include <${RULES_MK}>
