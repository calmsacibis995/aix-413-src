# @(#)35        1.8  src/packages/printers/ti2115/rte/packdep.mk, pkgdevprint, pkg411, GOLD410 2/24/94 12:05:50
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

USR_LIBLPP_LIST		+= printers.ti2115.rte.pre_d \
			   printers.ti2115.rte.namelist \
			   printers.ti2115.rte.rm_inv

ROOT_LIBLPP_LIST	+=

USR_ODMADD_LIST		+= printer.ti2115.diag.add%printers.ti2115.rte \
			   printer.ti2115.add%printers.ti2115.rte
