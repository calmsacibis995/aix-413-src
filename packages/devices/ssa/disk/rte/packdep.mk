# @(#)26 1.1  src/packages/devices/ssa/disk/rte/packdep.mk, mayaixpackaging, pkg41J, 9509A_all 2/20/95 21:32:44
#########################################################################
#									#
#   COMPONENT NAME: MAYAIXPACKAGING					#
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

USR_LIBLPP_LIST		+= devices.ssa.disk.rte.pre_d

ROOT_LIBLPP_LIST	+= devices.ssa.disk.rte.err \
			   devices.ssa.disk.rte.trc

USR_ODMADD_LIST		+= disk.ssa.usr.add%devices.ssa.disk.rte

ROOT_ODMADD_LIST	+= disk.ssa.root.add%devices.ssa.disk.rte

