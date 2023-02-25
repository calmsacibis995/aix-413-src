# @(#)46        1.4  src/packages/X11/info/en_US/hypertext/Makefile, pkghyper, pkg41J, 9515A_all 4/5/95 14:49:31
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
# IMPORTANT !!!!
# 
# The next two variables are used to create the HyperText CD image
# for the X11 LPP in the AIX 4.1 release
#
USR_LIBLPP_LIST	= X11.info.en_US.prog_gds.cfginfo \
		  X11.info.en_US.tech_ref.cfginfo \
		  X11.info.en_US.x_st_mgr.manage_gd.cfginfo \
		  X11.info.en_US.Dt.cfginfo

PTF_UPDATE	= no

VPATH		= ../prog_gds:../tech_ref
VPATH		:= ${VPATH}:../prog_gds/usr:../tech_ref/usr
VPATH		:= ${VPATH}:../x_st_mgr/manage_gd:../x_st_mgr/manage_gd/usr
VPATH		:= ${VPATH}:../Dt:../Dt/usr

BFF		= X11.info.en_US

ILIST		= ${BFF}.hypertext
IDIR		= /

OPTIONS		= X11.info.en_US.prog_gds \
		  X11.info.en_US.tech_ref \
		  X11.info.en_US.x_st_mgr.manage_gd \
		  X11.info.en_US.Dt

ADEPACKAGEFLAGS = -n

.include <../prog_gds/packdep.mk>
.include <../tech_ref/packdep.mk>
.include <../x_st_mgr/manage_gd/packdep.mk>
.include <../Dt/packdep.mk>

.include <${RULES_MK}>

X11.info.en_US.hypertext : X11.info.en_US
	${CP} X11.info.en_US X11.info.en_US.hypertext
