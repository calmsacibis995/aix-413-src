# @(#)02        1.8  src/packages/printers/p9012/rte/packdep.mk, pkgdevprint, pkg411, GOLD410 2/24/94 12:05:02
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

USR_LIBLPP_LIST		+= printers.p9012.rte.pre_d \
			   printers.p9012.rte.namelist \
			   printers.p9012.rte.rm_inv

ROOT_LIBLPP_LIST	+=

USR_ODMADD_LIST		+= printer.p9012.diag.add%printers.p9012.rte \
			   printer.p9012.add%printers.p9012.rte
