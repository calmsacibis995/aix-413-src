# @(#)53        1.8  src/packages/printers/ibm7372/rte/packdep.mk, pkgdevprint, pkg411, GOLD410 2/24/94 12:03:57
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

USR_LIBLPP_LIST		+= printers.ibm7372.rte.pre_d \
			   printers.ibm7372.rte.namelist \
			   printers.ibm7372.rte.rm_inv

ROOT_LIBLPP_LIST	+=

USR_ODMADD_LIST		+= printer.ibm7372.diag.add%printers.ibm7372.rte \
			   printer.ibm7372.add%printers.ibm7372.rte
