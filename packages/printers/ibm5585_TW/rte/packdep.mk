# @(#)05        1.7  src/packages/printers/ibm5585_TW/rte/packdep.mk, pkgdevprint, pkg411, GOLD410 2/24/94 12:08:45
#
# COMPONENT_NAME: pkgdevprint
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

USR_LIBLPP_LIST		+= printers.ibm5585_TW.rte.pre_d \
			   printers.ibm5585_TW.rte.namelist \
			   printers.ibm5585_TW.rte.rm_inv

ROOT_LIBLPP_LIST	+=

USR_ODMADD_LIST		+=

INFO_FILES		+= printers.ibm5585_TW.rte.prereq
