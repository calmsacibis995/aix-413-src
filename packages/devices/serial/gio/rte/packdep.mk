# @(#)24        1.4  src/packages/devices/serial/gio/rte/packdep.mk, pkgdevgraphics, pkg41J, 9518A_all 5/1/95 13:38:50
#
# COMPONENT_NAME: pkgx
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

USR_LIBLPP_LIST		+=
ROOT_LIBLPP_LIST	+= devices.serial.gio.rte.trc \
					   devices.serial.gio.rte.post_i

ROOT_LIBLPP_UPDT_LIST	+= devices.serial.gio.rte.post_u

USR_ODMADD_LIST		+= sgio.add%devices.serial.gio.rte
ROOT_ODMADD_LIST	+= cfgrule.sgio.add%devices.serial.gio.rte

INFO_FILES			+= devices.serial.gio.rte.prereq
