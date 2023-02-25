# @(#)77        1.12  src/packages/devices/tty/rte/packdep.mk, pkgdevtty, pkg41J, 9520A_all 4/27/95 16:14:57
#
# COMPONENT_NAME: pkgdevtty
#
# FUNCTIONS: packaging definitions
#
# ORIGINS: 27
#
# IBM CONFIDENTIAL -- (IBM Confidential Restricted when
# combined with the aggregated modules for this product)
#                  SOURCE MATERIALS
# (C) COPYRIGHT International Business Machines Corp. 1993,1994
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
OBJCLASSDB		= ${ODE_TOOLS}/usr/lib/uniqueobjclassdb

USR_LIBLPP_LIST		+= devices.tty.rte.namelist \
			devices.tty.rte.pre_rm \
			devices.tty.rte.158660.odmdel

USR_ODMADD_LIST		+= tty.add%devices.tty.rte \
			   printer.osp.add%devices.tty.rte

ROOT_ODMADD_LIST        += cfgrule.tty.add%devices.tty.rte

