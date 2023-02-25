# @(#)15        1.8  src/packages/printers/ibm4029/rte/packdep.mk, pkgdevprint, pkg411, GOLD410 2/24/94 11:57:54
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

USR_LIBLPP_LIST		+= printers.ibm4029.rte.pre_d \
			   printers.ibm4029.rte.namelist \
			   printers.ibm4029.rte.rm_inv

ROOT_LIBLPP_LIST	+=

USR_ODMADD_LIST		+= printer.ibm4029.diag.add%printers.ibm4029.rte \
			   printer.ibm4029.add%printers.ibm4029.rte
