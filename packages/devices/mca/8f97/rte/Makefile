# @(#)06 1.1  src/packages/devices/mca/8f97/rte/Makefile, mayaixpackaging, pkg41J, 9509A_all 2/20/95 21:30:21
#########################################################################
#									#
#   COMPONENT NAME: MAYAIXPACKAGING					#
#									#
#   FILE NAME:	    Makefile						#
#									#
#   ORIGINS:	    27							#
#									#
#   DESCRIPTION:    Makefile to	build the AIX 410 SSA adapter rte	#
#		    fileset.						#
#									#
#   IBM	CONFIDENTIAL --	(IBM Confidential Restricted when combined	#
#			 with the aggregate modules for	this project)	#
#									#
#   (C)	Copyright International	Business Machines Corp.	1995		#
#   All	rights reserved.						#
#									#
#########################################################################

VPATH		= ${MAKETOP}/ssa/objclass/${TARGET_MACHINE}

#########################################################################
# This is the name of the BFF we're going to make			#
#########################################################################

BFF		= devices.mca.8f97

#########################################################################
# These is what we're making, and where to install it in the ship tree	#
#########################################################################

ILIST		= ${BFF}
IDIR		= /

#########################################################################
# These is the USR fileset						#
#########################################################################

OPTIONS		= devices.mca.8f97.rte
ROOT_OPTIONS	= devices.mca.8f97.rte
LINKFLAG        = -L

.include <packdep.mk>

.include <${RULES_MK}>
