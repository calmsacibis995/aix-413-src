# @(#)18        1.12  src/packages/devices/mca/8ef4/rte/packdep.mk, pkgdevcommo, pkg411, GOLD410 7/5/94 15:42:52
#
# COMPONENT_NAME: pkgdevcommo
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
# *_LIBLPP_LIST contains the list of files that need to be added to 
#               the respective liblpp.a other than the copyright, .al,
#               .tcb, .size, and .inventory files.  For example, any
#               config scripts would be included here.
#               The variable may be left blank if there are no additional
#               files, but the option will exist.
#

# Set environment variable to use objclassdb with unique key identifiers
# because of conflict of PdAt entries with comio emulator package
# "bos.compat.lan"
OBJCLASSDB = ${ODE_TOOLS}/usr/lib/uniqueobjclassdb

USR_LIBLPP_LIST		+= devices.mca.8ef4.rte.pre_d \
                           devices.mca.8ef4.rte.namelist \
			   devices.mca.8ef4.rte.rm_inv

ROOT_LIBLPP_LIST	+= devices.mca.8ef4.rte.trc \
			   devices.mca.8ef4.rte.err

USR_ODMADD_LIST		+= fddi.add%devices.mca.8ef4.rte

INFO_FILES		+= devices.mca.8ef4.rte.prereq
