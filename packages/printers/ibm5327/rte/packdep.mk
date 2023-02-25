# @(#)41        1.7  src/packages/printers/ibm5327/rte/packdep.mk, pkgdevprint, pkg411, GOLD410 2/24/94 12:01:41
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

USR_LIBLPP_LIST		+= printers.ibm5327.rte.pre_d \
			   printers.ibm5327.rte.namelist \
			   printers.ibm5327.rte.rm_inv

ROOT_LIBLPP_LIST	+=

USR_ODMADD_LIST		+= printer.ibm5327.diag.add%printers.ibm5327.rte \
			   printer.ibm5327.add%printers.ibm5327.rte

INFO_FILES		+= printers.ibm5327.rte.prereq
