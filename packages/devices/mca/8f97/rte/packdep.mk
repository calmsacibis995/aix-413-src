# @(#)10 1.1  src/packages/devices/mca/8f97/rte/packdep.mk, mayaixpackaging, pkg41J, 9509A_all 2/20/95 21:30:48
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

USR_LIBLPP_LIST		+= devices.mca.8f97.rte.pre_d

ROOT_LIBLPP_LIST	+= 

USR_ODMADD_LIST		+= adapter.mca.8f97.add%devices.mca.8f97.rte

