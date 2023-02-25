# @(#)92	1.2  src/packages/devices/isa_sio/PNP0401/diag/packdep.mk, pkgdevdiag, pkg41J, 9520A_all 5/16/95 09:03:21
#
# COMPONENT_NAME: pkgdevdiag
#
# FUNCTIONS: packaging definitions
#
# ORIGINS: 27
#
# IBM CONFIDENTIAL -- (IBM Confidential Restricted when
# combined with the aggregated modules for this product)
#                  SOURCE MATERIALS
# (C) COPYRIGHT International Business Machines Corp. 1995
# All Rights Reserved
#
# US Government Users Restricted Rights - Use, duplication or
# disclosure restricted by GSA ADP Schedule Contract with IBM Corp.
#
#
# *_LIBLPP_LIST	contains the list of files that need to be added to 
#		the respective liblpp.a other than the copyright, .al,
#		.tcb, .size, and .inventory files.  For example, any
#		config scripts would be included here.
#		The variable may be left blank if there are no additional
#		files, but the option will exist.
#

USR_LIBLPP_LIST		+=

USR_ODMADD_LIST         += diag.pnp0401.add%devices.isa_sio.PNP0401.diag

INFO_FILES		+= devices.isa_sio.PNP0401.diag.prereq
