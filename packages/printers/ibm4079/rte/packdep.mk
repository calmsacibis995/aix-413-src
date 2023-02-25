# @(#)84        1.10  src/packages/printers/ibm4079/rte/packdep.mk, pkgdevprint, pkg411, GOLD410 2/24/94 11:51:43
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

USR_LIBLPP_LIST		+= printers.ibm4079.rte.pre_d \
			   printers.ibm4079.rte.namelist \
			   printers.ibm4079.rte.rm_inv

ROOT_LIBLPP_LIST	+=

USR_ODMADD_LIST		+= printer.ibm4079.diag.add%printers.ibm4079.rte \
			   printer.ibm4079.add%printers.ibm4079.rte
