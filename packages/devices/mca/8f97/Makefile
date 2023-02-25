# @(#)95 1.1  src/packages/devices/mca/8f97/Makefile, mayaixpackaging, pkg41J, 9509A_all 2/20/95 21:29:01
#########################################################################
#									#
#   COMPONENT NAME: MAYAIXPACKAGING					#
#									#
#   FILE NAME:	    Makefile						#
#									#
#   ORIGINS:	    27							#
#									#
#   DESCRIPTION:    Makefile to	build the AIX 410 SSA adapter software	#
#		    installp package.					#
#									#
#   IBM	CONFIDENTIAL --	(IBM Confidential Restricted when combined	#
#			 with the aggregate modules for	this project)	#
#									#
#   (C)	Copyright International	Business Machines Corp.	1995		#
#   All	rights reserved.						#
#									#
#########################################################################

#########################################################################
# Specify where to locate dependent files				#
#########################################################################

#VPATH		= com:rte:rte/usr:ucode:diag
VPATH		= com:rte:rte/usr:diag
VPATH	       := ${VPATH}:${MAKETOP}/ssa/objclass/${TARGET_MACHINE}

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
# The filesets								#
#########################################################################

OPTIONS		= devices.mca.8f97.com   \
		  devices.mca.8f97.rte   \
		  devices.mca.8f97.diag
#		  devices.mca.8f97.ucode

ROOT_OPTIONS	= devices.mca.8f97.com
LINKFLAG	= -L

.include <com/packdep.mk>
.include <rte/packdep.mk>
.include <diag/packdep.mk>
#.include <ucode/packdep.mk>

.include <${RULES_MK}>
