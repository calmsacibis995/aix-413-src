# @(#)00 1.1  src/packages/devices/mca/8f97/diag/packdep.mk, mayaixpackaging, pkg41J, 9509A_all 2/20/95 21:29:37
#########################################################################
#									#
#   COMPONENT NAME: MAYAIXMAKEFILES					#
#									#
#   FILE NAME:	    packdep.mk						#
#									#
#   ORIGINS:	    27							#
#									#
#   DESCRIPTION:    Packaging definition file				#
#									#
#   IBM	CONFIDENTIAL --	(IBM Confidential Restricted when combined	#
#			 with the aggregate modules for	this project)	#
#									#
#   (C)	Copyright International	Business Machines Corp.	1995		#
#   All	rights reserved.						#
#									#
#########################################################################

USR_LIBLPP_LIST		+= devices.mca.8f97.diag.config

ROOT_LIBLPP_LIST	+= devices.mca.8f97.diag.post_i \
			   devices.mca.8f97.diag.unpost_i

USR_ODMADD_LIST		+= diag.8f97.add%devices.mca.8f97.diag

INFO_FILES		+= devices.mca.8f97.diag.insize \
			   devices.mca.8f97.diag.prereq
