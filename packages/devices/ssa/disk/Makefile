# @(#)21 1.1  src/packages/devices/ssa/disk/Makefile, mayaixpackaging, pkg41J, 9509A_all 2/20/95 21:32:08
#########################################################################
#									#
#   COMPONENT NAME: MAYAIXPACKAGING					#
#									#
#   FILE NAME:	    Makefile						#
#									#
#   ORIGINS:	    27							#
#									#
#   DESCRIPTION:    Makefile to	build the AIX 410 SSA DASD software	#
#		    installp pacakge.					#
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

#VPATH		= rte:rte/usr:ucode
VPATH		= rte:rte/usr
VPATH	       := ${VPATH}:${MAKETOP}/ssa/objclass/${TARGET_MACHINE}

#########################################################################
# This is the name of the BFF we're going to make			#
#########################################################################

BFF		= devices.ssa.disk

#########################################################################
# These is what we're making, and where to install it in the ship tree	#
#########################################################################

ILIST		= ${BFF}
IDIR		= /

#########################################################################
# The filesets								#
#########################################################################

OPTIONS		= devices.ssa.disk.rte
#		  devices.ssa.disk.ucode
ROOT_OPTIONS	= devices.ssa.disk.rte
LINKFLAG	= -L

.include <rte/packdep.mk>

.include <${RULES_MK}>
